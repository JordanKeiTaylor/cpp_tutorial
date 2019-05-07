## Tutorial overview
	-This project was developed for tutorial purposes using [Structured Project Layout](https://docs.improbable.io/reference/13.7/shared/glossary#structured-project-layout-spl) and the C++ worker SDK. 

  The tutorial demonstrates how to add functional random movement to a C++ blank project, teaching users about Authority, Interest, and writing worker code along the way.

## What's included in this directory
	-A snapshot generation script for generating initial simulation state in `CppBlankProject/snapshotgenerator`
	-A blank managed worker in /workers/Managed. This will be the main worker we develop during the tutorial in this doc
	-A blank external worker (not directly referenced in the tutorial, but extensible if users choose to do so), in /workers/external


## Pre-reading

This tutorial expects that you are familiar with the core concepts of SpatialOS.

If you come across a concept you don't know, or need a primer or explanation of code, reference the SpatialOS docs at https://docs.improbable.io/reference/13.7.


## Building and running the project

To build the project, access the top level directory from the command line. From there run `spatial build`.

To run the project, use `spatial local launch` (This project already has a snapshot, and as a result is runnable -- more on that below)

Build the snapshot generator script by accessing the snapshotgenerator directory at `CPPBlankProject/snapshotgenerator/` using your command line. The snapshot generator is a project that uses the C++ worker sdk to build a snapshot file, which is deposited in the top level snapshots folder.

Once in the snapshotgenerator, cd into the `cmake-build` directory and run `cmake -G "Unix Makefiles" ..`. That will produce executable files including files for  the main snapshotgenerator script, which is in main.cpp in `CppBlankProject/snapshotgenerator/snapshots/main.cpp`. 

Produce a snapshot by accessing `CppBlankProject/snapshotgenerator/cmake-build/snapshots` From there run `make` This will generate a snapshot in the top level snapshots directory `/CppBlankProject/snapshots`, which will be used by the simulation.

You will need to rerun the two paragraphs above every time you change the snapshot generation script.

We recommend you read about snapshots and initial state in Spatial here:

-https://docs.improbable.io/reference/13.7/shared/operate/snapshot



## Feature Tutorial

	### Feature 0: Familiarize yourself with the project and its included managed worker by using the Inspector

First, build and run the project. Follow the command line output to view the inspector v1 in your browser. The inspector is a simple, browser based GUI for viewing the state of a running simulation.

Using the inspector, you should be able to see that the simulation has ten entities called example_entity. These entities have been instantiated using the snapshot generator script.

Play around with the inspector to familiarize yourself with the workflow for viewing components and authoritative workers. Specifically, notice that if you click on an entities in the gui, you can see the details for that entity on the pane in the right. In the authoritative worker section you can see which workers are authoritative over the components attached to that entity.

Over the course of this tutorial, well implement movement of these example entities.

Reading for this section:
  -[What is a managed worker](https://docs.improbable.io/reference/13.7/shared/design/design-workers#managed-workers)
  -[If you need a review of what entities and components are](https://docs.improbable.io/reference/13.7/shared/glossary#component)
  -[Components are defined using schema files in the schema folder](https://docs.improbable.io/reference/13.7/shared/concepts/schema#schema)
  -[Workers are processes defined in the workers folder, responsible for simulating the world. Workers are distributed across the world by SpatialOS, with configuration parameters for workers handled in the configuration file called "spatialos.<worker-type>.worker.json"](https://docs.improbable.io/reference/13.7/shared/project-layout/introduction)


###Feature 1: Make the managed worker authoritative over the Position component of the example entities

If you clicked on individual example_entities in the inspector, you should have seen that the Position components attached to each example entity have no worker that is authoritative over them. 

The Position component is a required component in SpatialOS that designates an entities x,y, and z location in the world. Right now this is the only component on the example entities (besides the entity Acl, another component that establishes which workers have authority and interest in the components on the entity)

Because the Managed worker has been assigned authority over this Position entity, the Managed worker attached to the project cannot impact the worker. 

Making the Managed worker authoritative over the example entities Position will allow the worker to move the entities around the world.

To give the managed worker authority over the Position component, go to the snapshot generation script in `CppBlankProject/snapshotgenerator/snapshots/main.cpp` 
(This is the script that uses the Snapshot generation namespace to create a snapshot file, defining the initial simulation state. )

The file contains a CreateExampleEntity function at line 10 which defines an entity template for the example entities. Later on this function is used to add the entities you saw to the snapshot.

In the line that starts `e.Add<improbable::EntityAcl`, the Example Entities EntityAcl component is being defined as simulated by the managed worker (eg the managed worker has Authority over it.) Copy this format to give the managed worker Authority over the Position entity.


 The final version of that function should look something like this:

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

Once you are done double check that you have successfully given the worker authority by running the simulation locally again. 

If the worker has successfully been given authority, you should be able to click on one of the example entities, click on the Authoritative workers tab, and see the Managed worker listed next to the entities Position component.

To see this step working, check out commit "bdecac154d4cd5b893f02b4766e3e9810e917b80" using git.

Reading for this section:
  -[Authority is when a worker has write access to a component; another important concept, Interest, is when a worker has read access only](https://docs.improbable.io/reference/13.7/shared/concepts/interest-authority#interest-and-authority)
  -[The EntityAcl is a required component that defines which workers have authority and interest over the components on an entity](https://docs.improbable.io/reference/13.7/shared/schema/standard-schema-library#entityacl-required)

###Feature 2: Set up a game loop and move one entity around the world randomly

Now that the worker has Authority over the example entities' Position components, you should be able to edit the Managed Worker so that it moves the example entities around the world by modifying their Position components.

To do this, head to the startup script in the src directory of the managed worker at `CppBlankProject/workers/Managed/src/startup.cc`

This script contains all the C++ logic that defines what the worker does once it is connected to SpatialOS. 

Like all SpatialOS workers, Managed is just a normal C++ process, which uses code generated by SpatialOS to create a connection to Spatial. Once the connection exists, it uses all the helper classes in the SpatialOS C++ sdk to do things like create entities, move entities, and generally simulate the world.

This script currently contains a bare worker c++ template that establishes a connection with SpatialOS. The connection is established by the function at line 44, called `ConnectWithReceptionist`. 

Once a connection is established, you can use the connection object to interact with SpatialOS, including sending component updates for components the worker is authoritative over, or responding to component updates for components you have interest in but not authority over. Read more about Operations to understand what helper functions are available.

SpatialOS will run the worker script you give it, distributing workers over the world. Through the generated classes and connection object, the script will have access to the components it has interest in that exist in its part of the world.

Typically users structure their managed workers as continuously running loops (eg "a game loop"), in which the worker repeatedly takes actions that constitute simulating their part of the world. (For example, moving an entity towards a destination every 100 milliseconds.)

The way to send component updates is via the connection function `sendComponentUpdate`. That component has [the signature described here](https://docs.improbable.io/reference/13.7/javasdk/using/sending-data#sending-component-updates) It takes the entity's id, and an instantiation of the Update class for the component you want to update.

Implement a worker loop in the main function of the startup script. For now, it should just move the entity with entityId 1 to position (0,0,0), and then to position (5,5,5), and then back. Try to structure the loop so that it runs once every 500 milliseconds.

 A simple way to implement a loop in C++ is touse the chrono and thread packages. These are already imported in the startup script.

Using those libraries, you can force the current thread to sleep for 500 milliseconds with the statement `std::this_thread::sleep_for(std::chrono::milliseconds(500));`

Once you've written the game loop your final code should look something like this (replacing the while is connected statement currently at line 119) :

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

Build the project and launch it locally. If everythings worked, you will now see a single entity,with entity id 1, flashing back and forth between two positions.

To see this functioning, you can checkout and run the commit labeled "6f58e19a055e6137c32784d6e240399d6de9b68e"

Reading List for this section
- [Operations or how workers are implemented and communicate with Spatial](https://docs.improbable.io/reference/13.7/shared/design/operations#operations-how-workers-communicate-with-spatialos)
- [Sending component updates when you have authority](https://docs.improbable.io/reference/13.7/cppsdk/using/sending-data#sending-data-to-spatialos)
- [Responding to data received from Spatial, when you have interest, using the Spatial OS dispatcher](https://docs.improbable.io/reference/13.7/cppsdk/using/receiving-data#dispatcher-callbacks)


###Feature 3: Capture all the Position components the worker is authoritative over locally, move them all together


As you've noticed, to send a component update in SpatialOS, you need the entity id of that entity. Spatial informs workers of the ids of the entities that are present in the workers portion of the world when they enter their portion of the world. It also informs workers of the ids of entities containing components that the worker has interest in or authority over.

These messages from Spatial containing important information about the state of entities in their part of the world come in through the dispatcher. You can register callbacks on the dispatcher, so that certain functions are run upon the worker receiving certain messages from Spatial.

Typically, workers are structured to keep track of the local state of entities and components in their view of the world in their type of local data structure. This allows easy reference to relevant components within a game loop, and easier control flow. (For example, recording all the entities for which the worker has authority over their Position in an array, so that you can use that array in your game loop to send Position updates every second).

There are a few convenient callbacks that can be used to set initial local state for components and entities when they enter a workers view of the world. Remember that if a worker has authority over a certain kind of component, it will be given authority over the instance of that component attached to a specific entity when the entity carrying the component enters the workers area.

Some of these callbacks that are useful to capture initial state are:

`OnAuthorityChange<typename T>: Runs when a worker is given authority over a component. Also run when the worker loses authority. The op contained in the callback will describes the initial component state as well as the entity id. It also cotnains an enum indicating the status of the workers authority.`


`OnAddComponent<typename T> Runs when an entity in a workers view of the world has a component added to it. This will be run at the beginning of a simulation as components are added to entities at startup. The op contained in the callback will have the initial component state included as well as the entity Id.`

`OnEntityAdd: Runs when an entity enters a workers view of the world. Includes the entityid of the entity.`

Create an Entity Wrapper class in the Managed worker startup script. The class should be able to hold the Entity Id and the coordinates of the example entities as fields.

Create a C++ vector to hold all the entity wrappers. Register a callback with any of the methods above so that when a message is receieved from Spatial when that entity is added to the workers view of the worlds, an EntityWrapper will be added to the vector.

Then rewrite the game loop so that it uses vector to update the position of the entities each loop based on their old positions. It should increment their x, y, and z components by 1 each loop.

In the end, your code should look something like this:

The EntityWrapper class:

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

The vector definition ` std::vector<EntityWrapper> wrappers;`

The callback registration:

```
    dispatcher.OnAddComponent<improbable::Position>([&](const worker::AddComponentOp<improbable::Position>& op) {
        std::cout << "Entity " << op.EntityId << " added." << std::endl;
        wrappers.push_back(EntityWrapper(op.EntityId,op.Data.coords()));
        entities.push_back(op.EntityId);
    });
```

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

To see this working, checkout commit "afea5e7865f4fa39656be26f19533dbebccbab02"

Reading list for this step:
  - All messages the dispatcher provides to workers(https://docs.improbable.io/reference/13.7/cppsdk/using/receiving-data)
  - Processing operations in C++(https://docs.improbable.io/reference/13.7/cppsdk/using/receiving-data)



###Feature 4: Now, use your knowledge to initialize multiple instances of the Managed worker; and add logic to the vector that removes entities from the vector once the worker loses authority.  
