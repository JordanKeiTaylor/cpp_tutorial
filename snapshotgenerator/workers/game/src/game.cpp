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
std::pair<worker::Option<worker::RequestId<worker::IncomingCommandRequest<othello::Game::Commands::CreatePlayer>>>,
          worker::Option<worker::RequestId<worker::IncomingCommandRequest<othello::Game::Commands::CreatePlayer>>>> PendingPlayer;
worker::Option<bool> black_turn;

// Use this to make a worker::ComponentRegistry.
// For example use worker::Components<improbable::Position, improbable::Metadata> to track these common components
using ComponentRegistry = worker::Components<improbable::Position, improbable::Metadata, improbable::EntityAcl,
        improbable::Persistence, othello::Game, othello::Color, othello::Player, othello::TurnTaker>;

worker::Connection ConnectWithReceptionist(const std::string hostname,
                                           const std::uint16_t port,
                                           const std::string& worker_id) {
    worker::ConnectionParameters params;
    params.WorkerType = "game";
    params.Network.ConnectionType = worker::NetworkConnectionType::kTcp;
    params.Network.UseExternalIp = false;

    auto future = worker::Connection::ConnectAsync(ComponentRegistry{}, hostname, port, worker_id, params);
    return future.Get();
}

worker::Entity CreateDiscEntry(int64_t x, int64_t z, bool black) {
    worker::Entity e;
    e.Add<improbable::Position>( { { double(x) + 0.5, 0, double(z) + 0.5 } } );
    e.Add<improbable::Metadata>( { "disc" } );
    e.Add<improbable::Persistence>( {} );

    auto anyWorkerRequirementSet = improbable::WorkerRequirementSet{
            worker::List<improbable::WorkerAttributeSet>{{worker::List<std::string>{"player"}, worker::List<std::string>{"game"}}}
    };
    auto gameWorkerRequirementSet = improbable::WorkerRequirementSet{
            worker::List<improbable::WorkerAttributeSet>{{worker::List<std::string>{"game"}}}
    };

    e.Add<improbable::EntityAcl>( /* read */ { anyWorkerRequirementSet,
                                                     /* write */ {
                                                       { improbable::Position::ComponentId, gameWorkerRequirementSet },
                                                       { othello::Color::ComponentId, gameWorkerRequirementSet }
                                               } } );

    e.Add<othello::Color>( { black } );

    return e;
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
        workerId = "game_" + lexical_cast<std::string>(std::rand() % 10000);

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

    dispatcher.OnCommandRequest<othello::Game::Commands::CreatePlayer>([&](const worker::CommandRequestOp<othello::Game::Commands::CreatePlayer> &op) {
        bool black = false;
        if (!PendingPlayer.first && dispatcher.Entities[BLACK_PLAYER].Get<othello::Player>()->worker().empty()) {
            black = true;
            PendingPlayer.first = op.RequestId;
        } else if (PendingPlayer.second || !dispatcher.Entities[WHITE_PLAYER].Get<othello::Player>()->worker().empty()) {
            conn.SendCommandFailure(op.RequestId, "Two players are already playing!");
            return;
        } else {
            PendingPlayer.second = op.RequestId;
        }

        conn.SendComponentUpdate<othello::Player>(black ? BLACK_PLAYER : WHITE_PLAYER,
                                                  othello::Player::Update().set_worker(op.CallerWorkerId));

        auto anyWorkerRequirementSet = improbable::WorkerRequirementSet{
                worker::List<improbable::WorkerAttributeSet>{{worker::List<std::string>{"workerId:" + op.CallerWorkerId},
                worker::List<std::string>{"game"}}}
        };
        auto playerWorkerRequirementSet = improbable::WorkerRequirementSet{
                worker::List<improbable::WorkerAttributeSet>{{worker::List<std::string>{"workerId:" + op.CallerWorkerId}}}};
        auto gameWorkerRequirementSet = improbable::WorkerRequirementSet{
                worker::List<improbable::WorkerAttributeSet>{{worker::List<std::string>{"game"}}}};

        conn.SendComponentUpdate<improbable::EntityAcl>(black ? BLACK_PLAYER : WHITE_PLAYER,
                improbable::EntityAcl::Update().set_read_acl(anyWorkerRequirementSet).set_component_write_acl(
                /* write */ {
                    { improbable::Position::ComponentId, gameWorkerRequirementSet },
                    { improbable::EntityAcl::ComponentId, gameWorkerRequirementSet },
                    { othello::Player::ComponentId, gameWorkerRequirementSet },
                    { othello::Color::ComponentId, gameWorkerRequirementSet },
                    { othello::TurnTaker::ComponentId, playerWorkerRequirementSet }
                } ));
    });

    dispatcher.OnComponentUpdate<othello::Player>([&](const worker::ComponentUpdateOp<othello::Player> &op) {
        if (op.EntityId == BLACK_PLAYER) {
            if (PendingPlayer.first) {
                conn.SendCommandResponse(*PendingPlayer.first, othello::CreatePlayerResult(true));
                PendingPlayer.first.clear();
                conn.SendLogMessage(worker::LogLevel::kInfo, "main", "Worker " +
                        dispatcher.Entities[BLACK_PLAYER].Get<othello::Player>()->worker() + " is playing as black");
            }
        } else if (op.EntityId == WHITE_PLAYER) {
            if (PendingPlayer.second) {
                conn.SendCommandResponse(*PendingPlayer.second, othello::CreatePlayerResult(false));
                PendingPlayer.second.clear();
                conn.SendLogMessage(worker::LogLevel::kInfo, "main", "Worker " +
                         dispatcher.Entities[WHITE_PLAYER].Get<othello::Player>()->worker() + " is playing as white");
            }
        }

        // We have two players, and it's nobodies turn!
        if (!dispatcher.Entities[BLACK_PLAYER].Get<othello::Player>()->worker().empty() &&
            !dispatcher.Entities[WHITE_PLAYER].Get<othello::Player>()->worker().empty() && black_turn.empty()) {
            conn.SendCommandRequest<othello::TurnTaker::Commands::YourTurn>(BLACK_PLAYER, othello::Void(), worker::Option<uint32_t>());
            black_turn = true;
        }
    });

    worker::Map<worker::RequestId<worker::IncomingCommandRequest<othello::Game::Commands::PlaceDisc> >,
                std::pair<bool, std::pair<int64_t, int64_t> > > response_turn;
    worker::Map<worker::RequestId<worker::OutgoingCommandRequest<othello::Color::Commands::FindColor> >,
                worker::RequestId<worker::IncomingCommandRequest<othello::Game::Commands::PlaceDisc> > > request_turn;

    dispatcher.OnCommandRequest<othello::Game::Commands::PlaceDisc>([&](const worker::CommandRequestOp<othello::Game::Commands::PlaceDisc> &op) {
        bool black = false;
        if (dispatcher.Entities[BLACK_PLAYER].Get<othello::Player>()->worker() == op.CallerWorkerId) {
            black = true;
        } else if (dispatcher.Entities[WHITE_PLAYER].Get<othello::Player>()->worker() != op.CallerWorkerId) {
            conn.SendCommandFailure(op.RequestId, "You are not playing!");
            return;
        }

        if (black_turn.empty() || black != *black_turn) {
            conn.SendCommandFailure(op.RequestId, "It is not your turn.");
            return;
        }

        worker::Option<worker::EntityId> north, east, south, west;
        for (auto entity : dispatcher.Entities) {
            if (entity.second.Get<improbable::Metadata>()->entity_type() != "disc")
                continue;

            // If the disc is the same colour, it's useless from a placement perspective.
            if (entity.second.Get<othello::Color>()->black() == black)
                continue;

            if (int64_t(entity.second.Get<improbable::Position>()->coords().x()) == op.Request.x()) {
                if (int64_t(entity.second.Get<improbable::Position>()->coords().z()) == op.Request.z()) {
                    conn.SendCommandResponse(op.RequestId, othello::SuccessResult(false));
                    return;
                }

                if (op.Request.z() > 0 &&
                    int64_t(entity.second.Get<improbable::Position>()->coords().z()) == op.Request.z() - 1) {
                    south = entity.first;
                } else if (int64_t(entity.second.Get<improbable::Position>()->coords().z()) == op.Request.z() + 1) {
                    north = entity.first;
                }
            } else if (int64_t(entity.second.Get<improbable::Position>()->coords().z()) == op.Request.z()) {
                if (op.Request.x() > 0 &&
                    int64_t(entity.second.Get<improbable::Position>()->coords().x()) == op.Request.x() - 1) {
                    west = entity.first;
                } else if (int64_t(entity.second.Get<improbable::Position>()->coords().x()) == op.Request.x() + 1) {
                    east = entity.first;
                }
            }
        }

        if (!north && !east && !south && !west) {
            conn.SendCommandResponse(op.RequestId, othello::SuccessResult(false));
            return;
        }

        response_turn[op.RequestId] = std::make_pair(false, std::make_pair(op.Request.x(), op.Request.z()));
        if (north) {
            auto req_id = conn.SendCommandRequest<othello::Color::Commands::FindColor>(*north,
                    othello::FindColorRequest(othello::Direction::NORTH, black), worker::Option<uint32_t>());
            request_turn[req_id] = op.RequestId;
        }
        if (east) {
            auto req_id = conn.SendCommandRequest<othello::Color::Commands::FindColor>(*east,
                   othello::FindColorRequest(othello::Direction::EAST, black), worker::Option<uint32_t>());
            request_turn[req_id] = op.RequestId;
        }
        if (south) {
            auto req_id = conn.SendCommandRequest<othello::Color::Commands::FindColor>(*south,
                   othello::FindColorRequest(othello::Direction::SOUTH, black), worker::Option<uint32_t>());
            request_turn[req_id] = op.RequestId;
        }
        if (west) {
            auto req_id = conn.SendCommandRequest<othello::Color::Commands::FindColor>(*west,
                   othello::FindColorRequest(othello::Direction::WEST, black), worker::Option<uint32_t>());
            request_turn[req_id] = op.RequestId;
        }
    });

    worker::Map<worker::RequestId<worker::OutgoingCommandRequest<othello::Color::Commands::FindColor> >,
                std::pair<worker::EntityId, worker::RequestId<worker::IncomingCommandRequest<othello::Color::Commands::FindColor> > > > request_chain;

    // Look for the color in the direction specified.
    dispatcher.OnCommandRequest<othello::Color::Commands::FindColor>([&](const worker::CommandRequestOp<othello::Color::Commands::FindColor> &op) {
        int64_t my_x = int64_t(dispatcher.Entities[op.EntityId].Get<improbable::Position>()->coords().x());
        int64_t my_z = int64_t(dispatcher.Entities[op.EntityId].Get<improbable::Position>()->coords().z());

        auto action_on_found = [&](auto entity) {
            if (entity.second.Get<othello::Color>()->black() == op.Request.black()) {
                conn.SendComponentUpdate<othello::Color>(op.EntityId, othello::Color::Update().set_black(
                        op.Request.black()));
                conn.SendCommandResponse(op.RequestId, othello::SuccessResult(true));
            } else {
                auto req = conn.SendCommandRequest<othello::Color::Commands::FindColor>(entity.first,
                         othello::FindColorRequest(op.Request.dir(), op.Request.black()), worker::Option<uint32_t>());
                request_chain[req] = std::make_pair(op.EntityId, op.RequestId);
            }
        };

        for (auto entity : dispatcher.Entities) {
            if (entity.second.Get<improbable::Metadata>()->entity_type() != "disc")
                continue;

            switch (op.Request.dir()) {
            case othello::Direction::NORTH:
                if (int64_t(entity.second.Get<improbable::Position>()->coords().x()) == my_x &&
                    int64_t(entity.second.Get<improbable::Position>()->coords().z()) == my_z + 1) {
                    action_on_found(entity);
                    return;
                }
                break;
            case othello::Direction::EAST:
                if (int64_t(entity.second.Get<improbable::Position>()->coords().x()) == my_x + 1 &&
                    int64_t(entity.second.Get<improbable::Position>()->coords().z()) == my_z) {
                    action_on_found(entity);
                    return;
                }
                break;
            case othello::Direction::SOUTH:
                if (int64_t(entity.second.Get<improbable::Position>()->coords().x()) == my_x &&
                    int64_t(entity.second.Get<improbable::Position>()->coords().z()) == my_z - 1) {
                    action_on_found(entity);
                    return;
                }
                break;
            case othello::Direction::WEST:
                if (int64_t(entity.second.Get<improbable::Position>()->coords().x()) == my_x - 1 &&
                    int64_t(entity.second.Get<improbable::Position>()->coords().z()) == my_z) {
                    action_on_found(entity);
                    return;
                }
                break;
            }
        }

        conn.SendCommandResponse(op.RequestId, othello::SuccessResult(false));
    });

    dispatcher.OnCommandResponse<othello::Color::Commands::FindColor>([&](const worker::CommandResponseOp<othello::Color::Commands::FindColor> &op) {
        auto itr = request_chain.find(op.RequestId);
        if (itr != request_chain.end()) {
            if (op.Response->success()) {
                conn.SendComponentUpdate<othello::Color>(itr->second.first, othello::Color::Update().set_black(
                        dispatcher.Entities[op.EntityId].Get<othello::Color>()->black()));
            }
            conn.SendCommandResponse(itr->second.second, othello::SuccessResult(op.Response->success()));
        } else {
            auto itr = request_turn.find(op.RequestId);
            if (itr != request_turn.end()) {
                auto rsp = itr->second;
                request_turn.erase(itr);

                auto rsp_itr = response_turn.find(rsp);
                if (op.Response->success() && rsp_itr != response_turn.end()) {
                    if (op.Response->success()) {
                        conn.SendCreateEntityRequest(
                                CreateDiscEntry(rsp_itr->second.second.first, rsp_itr->second.second.second,
                                                dispatcher.Entities[op.EntityId].Get<othello::Color>()->black()),
                                worker::Option<worker::EntityId>(), worker::Option<uint32_t>());
                        rsp_itr->second.first = op.Response->success();
                    }
                }

                // We're not done yet.
                for (auto &turn : request_turn) {
                    if (turn.second == rsp)
                        return;
                }

                bool success = false;
                if (rsp_itr != response_turn.end()) {
                    success = rsp_itr->second.first;
                    conn.SendCommandResponse(rsp, othello::SuccessResult(rsp_itr->second.first));
                    response_turn.erase(rsp_itr);
                } else {
                    conn.SendCommandResponse(rsp, othello::SuccessResult(false));
                }

                if (success) {
                    if (black_turn.empty() || !*black_turn) {
                        conn.SendCommandRequest<othello::TurnTaker::Commands::YourTurn>(BLACK_PLAYER, othello::Void(),
                                                                                        worker::Option<uint32_t>());
                        black_turn = true;
                    } else {
                        conn.SendCommandRequest<othello::TurnTaker::Commands::YourTurn>(WHITE_PLAYER, othello::Void(),
                                                                                        worker::Option<uint32_t>());
                        black_turn = false;
                    }
                }
            }
        }
    });


    std::thread thr([&]() {
        while (is_connected) {
            dispatcher.Process(conn.GetOpList(100));
        }
    });

    thr.join();

    return 0;
}
