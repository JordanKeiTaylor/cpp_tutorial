#include <improbable/worker.h>
#include <improbable/standard_library.h>
#include <iostream>

#include <othello.h>

using ComponentRegistry = worker::Components<improbable::Position, improbable::Metadata, improbable::EntityAcl,
                improbable::Persistence, othello::Color, othello::Game, othello::Player, othello::TurnTaker>;

worker::Entity CreateGameEntry() {
    worker::Entity e;
    e.Add<improbable::Position>( { { 4, 0, 4 } } );
    e.Add<improbable::Metadata>( { "game" } );
    e.Add<improbable::Persistence>( {} );

    auto anyWorkerRequirementSet = improbable::WorkerRequirementSet{
            worker::List<improbable::WorkerAttributeSet>{{worker::List<std::string>{"example_attribute_of_external_worker"}, worker::List<std::string>{"example_attribute_of_managed_worker"}}}
    };
    auto gameWorkerRequirementSet = improbable::WorkerRequirementSet{
            worker::List<improbable::WorkerAttributeSet>{{worker::List<std::string>{"example_attribute_of_managed_worker"}}}
    };

    e.Add<othello::Game>( {} );
    e.Add<improbable::EntityAcl>( /* read */ { anyWorkerRequirementSet,
                                                     /* write */ {
                                                       { othello::Game::ComponentId, gameWorkerRequirementSet }
                                               } } );

    return e;
}

worker::Entity CreatePlayerEntry(bool black) {
    worker::Entity e;
    e.Add<improbable::Position>( { { double(black ? 3 : 5), 0, 4 } } );
    e.Add<improbable::Metadata>( { "player" } );
    e.Add<improbable::Persistence>( {} );

    auto gameWorkerRequirementSet = improbable::WorkerRequirementSet{
            worker::List<improbable::WorkerAttributeSet>{{worker::List<std::string>{"example_attribute_of_managed_worker"}}}
    };

    e.Add<improbable::EntityAcl>( /* read */ { gameWorkerRequirementSet,
                                                     /* write */ {
                                                       { improbable::Position::ComponentId, gameWorkerRequirementSet },
                                                       { improbable::EntityAcl::ComponentId, gameWorkerRequirementSet },
                                                       { othello::Player::ComponentId, gameWorkerRequirementSet },
                                                       { othello::Color::ComponentId, gameWorkerRequirementSet }
                                               } } );

    e.Add<othello::Player>( { "" });
    e.Add<othello::Color>( { black } );
    e.Add<othello::TurnTaker>( { } );

    return e;
}

worker::Entity CreateDiscEntry(int64_t x, int64_t z, bool black) {
    worker::Entity e;
    e.Add<improbable::Position>( { { double(x) + 0.5, 0, double(z) + 0.5 } } );
    e.Add<improbable::Metadata>( { "disc" } );
    e.Add<improbable::Persistence>( {} );

    auto anyWorkerRequirementSet = improbable::WorkerRequirementSet{
            worker::List<improbable::WorkerAttributeSet>{{worker::List<std::string>{"example_attribute_of_managed_worker"}, worker::List<std::string>{"example_attribute_of_external_worker"}}}
    };
    auto gameWorkerRequirementSet = improbable::WorkerRequirementSet{
            worker::List<improbable::WorkerAttributeSet>{{worker::List<std::string>{"example_attribute_of_managed_worker"}}}
    };

    e.Add<improbable::EntityAcl>( /* read */ { anyWorkerRequirementSet,
                                                     /* write */ {
                                                       { improbable::Position::ComponentId, gameWorkerRequirementSet },
                                                       { othello::Color::ComponentId, gameWorkerRequirementSet }
                                               } } );

    e.Add<othello::Color>( { black } );

    return e;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Syntax: " << argv[0] << " <filename>" << std::endl;
        return -1;
    }

    worker::SnapshotOutputStream out(ComponentRegistry(), argv[1]);
    worker::EntityId nextId = 1;

    worker::Option<std::string> err;
    err = out.WriteEntity(nextId++, CreateGameEntry());
    if (err)
        std::cerr << "ERR: " << *err << std::endl;
    err = out.WriteEntity(nextId++, CreatePlayerEntry(false));
    if (err)
        std::cerr << "ERR: " << *err << std::endl;
    err = out.WriteEntity(nextId++, CreatePlayerEntry(true));
    if (err)
        std::cerr << "ERR: " << *err << std::endl;
    err = out.WriteEntity(nextId++, CreateDiscEntry(3, 3, false));
    if (err)
        std::cerr << "ERR: " << *err << std::endl;
    err = out.WriteEntity(nextId++, CreateDiscEntry(3, 4, true));
    if (err)
        std::cerr << "ERR: " << *err << std::endl;
    err = out.WriteEntity(nextId++, CreateDiscEntry(4, 3, true));
    if (err)
        std::cerr << "ERR: " << *err << std::endl;
    err = out.WriteEntity(nextId++, CreateDiscEntry(4, 4, false));
    if (err)
        std::cerr << "ERR: " << *err << std::endl;

    return 0;
}
