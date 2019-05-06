#include <improbable/worker.h>
#include <improbable/standard_library.h>
#include <iostream>

#include <othello.h>

using ComponentRegistry = worker::Components<improbable::Position, improbable::Metadata, improbable::EntityAcl,
                improbable::Persistence, othello::Color, othello::Game, othello::Player, othello::TurnTaker>;

worker::Entity CreateExampleEntity() {
    worker::Entity e;
    e.Add<improbable::Position>( { { 0, 0, 0 } } );
    e.Add<improbable::Metadata>( { "example_entity" } );
    e.Add<improbable::Persistence>( {} );

    auto managedWorkerRequirementSet = improbable::WorkerRequirementSet{
            worker::List<improbable::WorkerAttributeSet>{{worker::List<std::string>{"example_attribute_of_managed_worker"}}}
    };

    e.Add<improbable::EntityAcl>( /* read */ { managedWorkerRequirementSet,
                                                     /* write */ {
                                                       { improbable::Position::ComponentId, managedWorkerRequirementSet },
                                                       { improbable::EntityAcl::ComponentId, managedWorkerRequirementSet }
                                               } } );

    return e;
}


int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Syntax: " << argv[0] << " <filename>" << std::endl;
        return -1;
    }

    worker::SnapshotOutputStream out(ComponentRegistry(), "../../../snapshots/default.snapshot");
    worker::EntityId nextId = 1;
    worker::Option<std::string> err;

    for( int a = 0; a < 5; a = a + 1 ) {
        err = out.WriteEntity(nextId++, CreateExampleEntity());
        if (err)
            std::cerr << "ERR: " << *err << std::endl;      
    }

    return 0;
}
