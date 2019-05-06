## Tutorial overview
	-The simulation was developed for tutorial purposes using Structured project Structure and the C++ worker SDK. The tutorial demonstrates how to go from the C++ blank project to a functional Brownian Motion roject with a single worker moving objects randomly in Brownian Motion.

## What's included in this tutorial
	-A snapshot generation script in "/snapshotgenerator"
	-A blank managed worker in /workers/Managed
	-A blank external worker (not directly referenced in the tutorial), in /workers/external


## Building the project...and generating a snapshot

To build the project, access the top level directory from the command line. Run `spatial build`, then `spatial local start`

Build the snapshot generator by accessing the snapshotgenerator directory using your command line. The snapshot generator is a project that uses the C++ worker snapshot API to generate a snapshot. Once in the snapshotgenerator, run `cmake -G "Unix Makefiles" ..`, which will produce executable files including files for  the main snapshotgenerator script, in main.cpp in `CppBlankProject/snapshotgenerator/snapshots/main.cpp`. 

Produce a snapshot by accessing `CppBlankProject/snapshotgenerator/cmake-build/snapshots` Once in this folder run `make` This will generate a snapshot in the top level snapshots directory `/CppBlankProject/snapshots`, which will be used by the simulation.

## Pre-reading

This tutorial expects that you are familiar with the basic concepts of SpatialOS, especially Spatial's ECS architecture, Authority and Interest.

Read this walkthrough prior to engaging with the tutorial if you aren't familiar with those concepts, or at any point during the tutorial if you need a refresher.



## Tutorial Walkthrough 

	### Feature 0: Familiarize yourself with the existing project and included workers by running a local simulation

From the top level folder, run spatial build, then spatial local start. Open the inspector v1 once the prompt with the http address appears. You should see that the simulation has ten example entities. These entities have been instantiated at the Play around with the inspector to familiarize yourself with the workflow for viewing components and authoritative worker. 

	###Feature 1: Make the managed worker authoritative over the Position component of the example entities

If you click on individual entities in the inspector, you should be able to see the components attached to each example entity. Right now, the only components attached to these entities is the required SpatialOS position component that establishes an entities x y and z location in the world.

Because no worker has been assigned authority over this Position entity, the managed worker attached to the project cannot impact the worker. 

To give the managed worker authority over the Position component, go to the snapshot generation script in `CppBlankProject/snapshotgenerator/snapshots/main.cpp` 
This is the script that uses the Snapshot generation namespace to create a snapshot file, defining the initial simulation state. 

The file contains a CreateExampleEntity script at line 10 which defines an entity template for an example_entity. The final version of that function should look something like this:

```
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
```

Now double check that you have successfully given the worker authority by running the simulation locally again. If the worker has successfully been given authority you should be able to click on one of the example entities, click on the Authoritative workers tab, and see the Managed worker listed next to the entities Position component.

To see this working, check out commit "bdecac154d4cd5b893f02b4766e3e9810e917b80"

	###Feature 2: Set up a game loop and move one entity around the world randomly

Now that the worker has authority, you should be able to edit the managed worker to move the example entities around the world by modifying their Position components.

To do this, head to the startup script in the src directory of the managed worker at `CppBlankProject/workers/Managed/src/startup.cc`

This script currently contains a bare worker c++ template that establishes a connection with SpatialOS. The connection is established by the function at line 44. Take a look at this function.

Once a connection is established, you can use the connection object to interact with SpatialOS, including sending component updates for components the worker is authoritative over, or responding to component updates for components you have interest in but not authority over. 

SpatialOS will run the worker script you give it, distributing workers over the world. Typically users structure their managed workers as continuously running loops (eg "a game loop"), using some kind of control flow to ensure the loop only runs once within a certain timeframe, and enacts logic within that loop to send component updates and interact with the world.

The way to send component updates is via the connection function `sendComponentUpdate`.

Implement a worker loop in the int main function that moves the entity with entityId 1 to position (0,0,0) and then position (5,5,5) and back. The loop should run once every 500 milliseconds. There are lots of ways to implement a game loop. A simple way to do it here would be to use the chrono and thread packages. These are already imported in the startup script. Using those libraries, you can force the current thread to sleep for 500 milliseconds ` std::this_thread::sleep_for(std::chrono::milliseconds(500));
`

Your final code should look (something) like this:

```
while (is_connected) {
		//this line processes any ops that have come from the dispatcher. These ops includes any component updates for components that the worker has interest in
        dispatcher.Process(connection.GetOpList(kGetOpListTimeoutInMilliseconds));
        
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

      	connection.SendComponentUpdate<improbable::Position>(1,
                improbable::Position::Update().set_coords(
                    improbable::Coordinates
                    (
                        0,
                        0,
                        0
                    )
                )
            );

        connection.SendComponentUpdate<improbable::Position>(1,
                improbable::Position::Update().set_coords(
                    improbable::Coordinates
                    (
                        5,
                        5,
                       	5
                    )
                )
            );

    }

```

Build the project and run spatial local start. If everythings worked, you will now see a single entity, the one with entity id 1, flashing back and forth between two positions.

To see this functioning, checkout and run commit "6f58e19a055e6137c32784d6e240399d6de9b68e"


	###Feature 3: Capture all the Position components the worker is authoritative over locally, move them all together


As you've noticed, to send a component update in SpatialOS, you need the entity id of that entity. Workers are informed of the ids of the entities that are present in the workers portion of the world, and the ids of entities containing components that the worker has interest in or authority over.

Typically, game loops keep track of the entities and components in their view of the world containing some type of local . This makes control flow easier. (For example, recording all the entities for which you have authority over their Position in an array, so that you can use that array in your game loop to send Position updates every second. You can also record the old state in your local data structure for reference).

There are a few callbacks to get access to the components and entities that enter a workers view of the world:

OnAuthorityChange<typename T>: Runs when a worker is given authority over a component, in other words when an entity containing that component enters the workers area of the world. Also run when the worker loses authority. The op contained in the callback will have the initial component state included as well as the entity id and an enum indicating the status of the workers authority.


OnOnAddComponent<typename T> Runs when an entity in a workers view of the world has a component added to it. This will be run at the beginning of a simulation as components are added to entities. The op contained in the callback will have the initial component state included as well as the entity Id.

OnEntityAdd Runs when an entity enters a workers view of the world. Includes the entityid of the entity.

Create an Entity Wrapper class in the startup script that can hold the Entity Ids and coordinates of the example entities,whose Position components the managed worker is now Authoritative over.

Create a C++ vector to hold all the entity wrappers. Register a callback with any of the methods above that will initialize an EntityWrapper for each entity and add it to the vector.

Use the vector to update the position of the entities each loop.

Your code should look something like this:

The EntityWrapper class

```
class EntityWrapper {

 public:
     worker::EntityId id;
     improbable::Coordinates coords;

 public:
      EntityWrapper(worker::EntityId given_id, improbable::Coordinates given_coords){
        id = given_id;
        coords = given_coords;
      }



      worker::EntityId getId(){
        return id;
      }

      improbable::Coordinates getCoordinates(){
        return coords;
      }


};
```

The vector `  std::vector<EntityWrapper> wrappers;`


And the modified game loop: 
```
while (is_connected) {
        dispatcher.Process(connection.GetOpList(kGetOpListTimeoutInMilliseconds));
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        for (auto &entityWrapper : wrappers) // access by reference to avoid copying
        {  
            double new_x = entityWrapper.coords.x() + 1;
            double new_y = entityWrapper.coords.y() + 1;
            double new_z = entityWrapper.coords.z() + 1;

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

```

Build the project and start a local simulation. You should see the entities moving up and to the right.


	###Feature 4: Now, use your knowledge to initialize multiple instances of the Managed worker, and add logic to the vector that removes entities from the vector once the worker loses authority. May need to read up on the authority change op.
