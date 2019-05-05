#include <improbable/worker.h>
#include <improbable/view.h>
#include <improbable/standard_library.h>
#include <sstream>
#include <iostream>
#include <chrono>
#include <thread>

#include <othello.h>

const worker::EntityId GAME_ENTITY = 1;
const worker::EntityId WHITE_PLAYER = 2;
const worker::EntityId BLACK_PLAYER = 3;

// Use this to make a worker::ComponentRegistry.
// For example use worker::Components<improbable::Position, improbable::Metadata> to track these common components
using ComponentRegistry = worker::Components<improbable::Position, improbable::Metadata,
            othello::Color, othello::Game, othello::TurnTaker>;

worker::Connection ConnectWithReceptionist(const std::string hostname,
                                           const std::uint16_t port,
                                           const std::string& worker_id) {
    worker::ConnectionParameters params;
    params.WorkerType = "player";
    params.Network.ConnectionType = worker::NetworkConnectionType::kTcp;
    params.Network.UseExternalIp = false;

    auto future = worker::Connection::ConnectAsync(ComponentRegistry{}, hostname, port, worker_id, params);
    return future.Get();
}

// Poor man's boost::lexical_cast, good enough for this example, but still pretty bad ;)
template<typename RV, typename T>
static RV lexical_cast(T in) {
    std::stringstream ss;
    ss << in;
    RV rv;
    ss >> rv;
    return rv;
}

void play(worker::Connection &conn, worker::View &dispatcher, bool black) {
    std::cout << "YOU ARE " << (black ? "black (*)" : "white (o)") << std::endl;

    worker::Option<bool> grid[8][8];

    for (auto entity : dispatcher.Entities) {
        if (entity.second.Get<improbable::Metadata>()->entity_type() != "disc")
            continue;

        int64_t x = int64_t(entity.second.Get<improbable::Position>()->coords().x());
        int64_t z = int64_t(entity.second.Get<improbable::Position>()->coords().z());
        bool black = entity.second.Get<othello::Color>()->black();

        grid[z][x] = black;
    }

    std::cout << "y +---+---+---+---+---+---+---+---+" << std::endl;
    for (int i = 7; i >= 0; --i) {
        std::cout << i << " |";
        for (int j = 0; j < 8; ++j) {
            if (grid[i][j])
                std::cout << " " << ((*grid[i][j]) ? "*" : "o") << " |";
            else
                std::cout << "   |";
        }
        std::cout << "\n  +---+---+---+---+---+---+---+---+" << std::endl;
    }
    std::cout << "    0   1   2   3   4   5   6   7 x" << std::endl;

    int64_t x=-1, y=-1;
    while (x < 0 || x > 7 || y < 0 || y > 7) {
        x = y = -1;
        std::cout << "Enter location to place tile (X Y): ";
        std::cin >> x >> y;
    }

    conn.SendCommandRequest<othello::Game::Commands::PlaceDisc>(GAME_ENTITY, othello::PlaceDiscRequest(x, y), 100);
}

int main(int argc, char *argv[]) {
    auto now = std::chrono::high_resolution_clock::now();
    std::srand(std::chrono::time_point_cast<std::chrono::nanoseconds>(now).time_since_epoch().count());

    std::string hostname, workerId;
    uint16_t port;
    if (argc > 2) {
        hostname = argv[1];
        port = lexical_cast<uint16_t>(argv[2]);
        if (argc > 3) {
            workerId = argv[3];
        }
    } else {
        hostname = "localhost";
        port = 7777;
    }

    if (workerId.empty())
        workerId = "player_" + lexical_cast<std::string>(std::rand() % 10000);

    std::cout << "Connecting to SpatialOS at " << hostname << ":" << port << " as " << workerId << std::endl;

    worker::Connection conn = ConnectWithReceptionist(hostname, port, workerId);
    conn.SendLogMessage(worker::LogLevel::kInfo, "main", "Let the game begin!");

    for (auto &s : conn.GetWorkerAttributes()) {
        std::cout << "Attribute: " << s << std::endl;
    }

    worker::View dispatcher(ComponentRegistry{});

    bool is_connected = conn.IsConnected();
    dispatcher.OnDisconnect([&](const worker::DisconnectOp& op) {
        std::cerr << "[disconnect] " << op.Reason << std::endl;
        is_connected = false;
    });

    dispatcher.OnFlagUpdate([&](const worker::FlagUpdateOp& op) {
        std::cerr << "[flag] " << op.Name << " set to " << *op.Value << std::endl;
    });

    // Print log messages received from SpatialOS
    dispatcher.OnLogMessage([&](const worker::LogMessageOp& op) {
        if (op.Level == worker::LogLevel::kFatal) {
            std::cerr << "Fatal error: " << op.Message << std::endl;
            std::terminate();
        }
        std::cout << "[remote] " << op.Message << std::endl;
    });

    if (is_connected) {
        std::cout << "[local] Connected successfully to SpatialOS, listening to ops... " << std::endl;
    }

    std::thread thr([&]() {
        while (is_connected) {
            dispatcher.Process(conn.GetOpList(100));
        }
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    worker::Dispatcher::CallbackKey create_player_cb_key;
    create_player_cb_key = dispatcher.OnCommandResponse<othello::Game::Commands::CreatePlayer>(
        [&](const worker::CommandResponseOp<othello::Game::Commands::CreatePlayer> &op) {
            dispatcher.Remove(create_player_cb_key);
            if (op.StatusCode != worker::StatusCode::kSuccess) {
                std::cerr << "Could not join the game: " << op.Message << std::endl;
                is_connected = false;
            } else {
                std::cout << "I have joined the game as " << (op.Response->black() ? "black" : "white") << std::endl;
            }
        });

    bool black = false;
    dispatcher.OnCommandRequest<othello::TurnTaker::Commands::YourTurn>([&](const worker::CommandRequestOp<othello::TurnTaker::Commands::YourTurn> &op) {
        black = dispatcher.Entities[op.EntityId].Get<othello::Color>()->black();
        std::thread thr([&]() { play(conn, dispatcher, black); });
        conn.SendCommandResponse(op.RequestId, othello::Void());
        thr.detach();
    });

    dispatcher.OnCommandResponse<othello::Game::Commands::PlaceDisc>([&](const worker::CommandResponseOp<othello::Game::Commands::PlaceDisc> &op) {
        if (op.StatusCode == worker::StatusCode::kSuccess) {
            if (op.Response->success()) {
                std::cout << "Piece placed successfully." << std::endl;
            } else {
                std::cout << "Piece cannot be placed here (retry)." << std::endl;
                std::thread thr([&]() { play(conn, dispatcher, black); });
                thr.detach();
            }
        } else {
            std::cerr << "PLACE PIECE ERROR: " << op.Message << std::endl;
        }
    });

    // Wait for 5 seconds to join the game.
    conn.SendCommandRequest<othello::Game::Commands::CreatePlayer>(GAME_ENTITY, othello::Void(), 5000);

    thr.join();

    return 0;
}
