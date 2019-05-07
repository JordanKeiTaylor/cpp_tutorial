#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <improbable/worker.h>
#include <improbable/standard_library.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <map>

// Use this to make a worker::ComponentRegistry.
// For example use worker::Components<improbable::Position, improbable::Metadata> to track these common components
using ComponentRegistry = worker::Components<improbable::Position, improbable::Metadata>;

// Constants and parameters
const int ErrorExitStatus = 1;
const std::string kLoggerName = "startup.cc";
const std::uint32_t kGetOpListTimeoutInMilliseconds = 100;


int getRandomNumber(int min, int max)
{
    static const double fraction = 1.0 / (RAND_MAX + 1.0);  // static used for efficiency, so we only calculate this value once
    // evenly distribute the random number across our range
    return min + static_cast<int>((max - min + 1) * (std::rand() * fraction));
}

class EntityWrapper {

 public:
     worker::EntityId id;
     improbable::Coordinates coords;
     improbable::Coordinates destination;

 public:
      EntityWrapper(worker::EntityId given_id, improbable::Coordinates given_coords){
        id = given_id;
        coords = given_coords;
        destination = improbable::Coordinates(
           (double) getRandomNumber(-300,300)
            ,(double) getRandomNumber(-300,300)
            ,(double) getRandomNumber(-300,300)
            );
      }



      worker::EntityId getId(){
        return id;
      }

      improbable::Coordinates getCoordinates(){
        return coords;
      }

      void newDestination(){
        destination = improbable::Coordinates(
           (double) getRandomNumber(-300,300)
            ,(double) getRandomNumber(-300,300)
            ,(double) getRandomNumber(-300,300)
        );
      }

      improbable::Coordinates step(){

        double x_distance = std::abs(destination.x() - coords.x());
        double y_distance = std::abs(destination.y() - coords.y());
        double z_distance = std::abs(destination.z() - coords.z());

        double total = std::abs(destination.x() - coords.x()) + std::abs(destination.y() - coords.y()) + std::abs(destination.z() - coords.z());
        double x_move = (x_distance/total) * 5;
        double y_move = (y_distance/total) * 5;
        double z_move = (z_distance/total) * 5;

        double new_x;
        if(destination.x()>coords.x()){
            new_x = coords.x() + x_move;
        } else {
            new_x = coords.x() - x_move;
        }
        double new_y;
        if(destination.y()>coords.y()){
            new_y = coords.y() + y_move;
        } else {
            new_y = coords.y() - y_move;
        }
        double new_z;
        if(destination.z()>coords.z()){
            new_z = coords.z() + z_move;
        } else {
            new_z = coords.z() - z_move;
        }

        if(x_distance< 5 &&  y_distance < 5 && z_distance < 5){
            newDestination();
        }

        return improbable::Coordinates(new_x,new_y,new_z);
      }


};

worker::Connection ConnectWithReceptionist(const std::string hostname,
                                           const std::uint16_t port,
                                           const std::string& worker_id,
                                           const worker::ConnectionParameters& connection_parameters) {
    auto future = worker::Connection::ConnectAsync(ComponentRegistry{}, hostname, port, worker_id, connection_parameters);
    return future.Get();
}



std::string get_random_characters(size_t count) {
    const auto randchar = []() -> char {
        const char charset[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";
        const auto max_index = sizeof(charset) - 1;
        return charset[std::rand() % max_index];
    };
    std::string str(count, 0);
    std::generate_n(str.begin(), count, randchar);
    return str;
}

// Entry point
int main(int argc, char** argv) {
    auto now = std::chrono::high_resolution_clock::now();
    std::srand(std::chrono::time_point_cast<std::chrono::nanoseconds>(now).time_since_epoch().count());

    std::cout << "[local] Worker started " << std::endl;

    auto print_usage = [&]() {
        std::cout << "Usage: Managed receptionist <hostname> <port> <worker_id>" << std::endl;
        std::cout << std::endl;
        std::cout << "Connects to SpatialOS" << std::endl;
        std::cout << "    <hostname>      - hostname of the receptionist or locator to connect to.";
        std::cout << std::endl;
        std::cout << "    <port>          - port to use if connecting through the receptionist.";
        std::cout << std::endl;
        std::cout << "    <worker_id>     - (optional) name of the worker assigned by SpatialOS." << std::endl;
        std::cout << std::endl;
    };

    std::vector<std::string> arguments;
    std::vector<EntityWrapper> wrappers;
    std::map<worker::EntityId, int> entityIdToVectorIndex;

    // if no arguments are supplied, use the defaults for a local deployment
    if (argc == 1) {
        arguments = { "receptionist", "localhost", "7777" };
    } else {
        arguments = std::vector<std::string>(argv + 1, argv + argc);
    }

    if (arguments.size() != 4 && arguments.size() != 3) {
        print_usage();
        return ErrorExitStatus;
    }

    worker::ConnectionParameters parameters;
    parameters.WorkerType = "Managed";
    parameters.Network.ConnectionType = worker::NetworkConnectionType::kTcp;
    parameters.Network.UseExternalIp = false;

    std::string workerId;

    // When running as an external worker using 'spatial local worker launch'
    // The WorkerId isn't passed, so we generate a random one
    if (arguments.size() == 4) {
        workerId = arguments[3];
    } else {
        workerId = parameters.WorkerType + "_" + get_random_characters(4);
    }

    std::cout << "[local] Connecting to SpatialOS as " << workerId << "..." << std::endl;

    // Connect with receptionist
    worker::Connection connection = ConnectWithReceptionist(arguments[1], atoi(arguments[2].c_str()), workerId, parameters);

    connection.SendLogMessage(worker::LogLevel::kInfo, kLoggerName, "Connected successfully");

    // Register callbacks and run the worker main loop.
    worker::Dispatcher dispatcher{ ComponentRegistry{} };
    bool is_connected = connection.IsConnected();

    dispatcher.OnDisconnect([&](const worker::DisconnectOp& op) {
        std::cerr << "[disconnect] " << op.Reason << std::endl;
        is_connected = false;
    });

    // Print log messages received from SpatialOS
    dispatcher.OnLogMessage([&](const worker::LogMessageOp& op) {
        if (op.Level == worker::LogLevel::kFatal) {
            std::cerr << "Fatal error: " << op.Message << std::endl;
            std::terminate();
        }
        std::cout << "[remote] " << op.Message << std::endl;
    });

    dispatcher.OnAddComponent<improbable::Position>([&](const worker::AddComponentOp<improbable::Position>& op) {
        std::cout << "Entity " << op.EntityId << " added." << std::endl;
        entityIdToVectorIndex[op.EntityId] = wrappers.size();
        wrappers.push_back(EntityWrapper(op.EntityId,op.Data.coords()));
    });

    dispatcher.OnAuthorityChange<improbable::Position>([&](const worker::AuthorityChangeOp& op) {
        std::cout << "Entity " << op.EntityId << " removed." << std::endl;
        if(op.Authority == worker::Authority::kAuthorityLossImminent){
            int indexInVector = entityIdToVectorIndex.at(op.EntityId);
            std::cout << "Entity " << indexInVector << " removed." << std::endl;
            wrappers.erase(wrappers.begin() + indexInVector);
        }

    });

    if (is_connected) {
        std::cout << "[local] Connected successfully to SpatialOS, listening to ops... " << std::endl;
    }

    while (is_connected) {
        dispatcher.Process(connection.GetOpList(kGetOpListTimeoutInMilliseconds));
    
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        for (auto &entityWrapper : wrappers) // access by reference to avoid copying
        {  
            improbable::Coordinates new_location = entityWrapper.step();
            double new_x = new_location.x();
            double new_y = new_location.y();
            double new_z = new_location.z();

             connection.SendComponentUpdate<improbable::Position>(entityWrapper.id,
                improbable::Position::Update().set_coords(
                    improbable::Coordinates
                    (
                        new_x,
                        new_y,
                        new_z
                    )
                )
            );

            entityWrapper.coords.set_x(new_x);
            entityWrapper.coords.set_y(new_y);
            entityWrapper.coords.set_z(new_z);
        }

    }

    return ErrorExitStatus;
}
