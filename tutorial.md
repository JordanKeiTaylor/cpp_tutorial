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


###Feature 4: Now, use your C++ knowledge to implement a more complex EntityWrapper class. That class will move the entities towards a present destination

To add a destination to the example entities, we could add a destination Component to our local ExampleEntityclass. (To start, we'll set a random destination). 

Then, to simulate entity movement, we would send repeated updates to Spatial to rewrite the entity's Position at intermdiate positions between the origin and the destination. This approach would imply that when the entity crosses boundaries, it would be assigned a new destination. If we wanted to change that, we could save the destination in Spatial, as a component field, rather than simply maintaining it locally in our worker code.

For now, try modifying your ExampleEntity class so that it saves a destination for each entity. Add a step function that moves the entity towards the destination every game loop. 

Your final EntityWrapper code might look something like this:

```
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

        //remember to set a new destination once the entity is close enough to its old destination
        if(x_distance< 5 &&  y_distance < 5 && z_distance < 5){
            newDestination();
        }

        return improbable::Coordinates(new_x,new_y,new_z);
      }


};
```

We used this function to produce a random number

```
int getRandomNumber(int min, int max)
{
    static const double fraction = 1.0 / (RAND_MAX + 1.0);  // static used for efficiency, so we only calculate this value once
    // evenly distribute the random number across our range
    return min + static_cast<int>((max - min + 1) * (std::rand() * fraction));
}
```

And you'll need to modify your game loop so that it uses the output of each step function to set the new Position component of the entity in Spatial.

```
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
```

If you build and load the project you should now see the entities moving towards random destinations.


###Feature 5: Spin up multiple instances of the Managed worker and try a different load balancing strategy

So far, we've worked with a single instance of the managed worker. This is useful for local testing, and a small number of entities, but for more entities, well need more workers.

Spatial achieves high entity counts by load balancing between multiple copies of workers, with configurable strategies for how load balancing is achieved.

Load balancing configurations for all layers of workers is set in the top level launch configuration file. Navigate to that file, and change the load balancing strategy for the Managed worker to spin up 4 workers. Also, change it so that it employs the sharding by entityId loadbalancing strategy, which splits up entities between workers by id, rather than by geographic location.

You'll probably need to reference the [launch configuration file documentation](https://docs.improbable.io/reference/13.7/shared/project-layout/launch-config#launch-configuration-file)

(**Note that you may also need to change the configuration file format to reflect the latest version**).

Your final config file should look like this (again, sorry for needing to update the format):

```
{
  "template": "small",
  "world": {
    "chunkEdgeLengthMeters": 50,
    "snapshots": {
      "snapshotWritePeriodSeconds": 0
    },
    "dimensions": {
      "xMeters": 1500,
      "zMeters": 1500
    }
  },
  "load_balancing": {
    "layer_configurations": [
        {
            "layer": "example_attribute_of_managed_worker",
            "entity_id_sharding": {
                "numWorkers": 5
            }        
        }
    ]
  },
  "workers": [
    {
      "worker_type": "External",
      "permissions": [{
        "entity_creation": {
          "allow": true
        },
        "entity_deletion": {
          "allow": true
        },
        "entity_query": {
          "allow": true,
          "components": []
        }
      }]
    },
    {
      "worker_type": "Managed",
      "permissions": [{
        "entity_creation": {
          "allow": true
        },
        "entity_deletion": {
          "allow": true
        },
        "entity_query": {
          "allow": true,
          "components": []
        }
      }]
    }
  ]
}

```

Now that you've changed the launch config, run the project locally. You should see 4 instances of the Managed worker present in the simulation, moving the entities between destinations.

Reading list for this section:
  - [The structure of the launch configuration file](https://docs.improbable.io/reference/13.7/shared/project-layout/launch-config#launch-configuration-file)
  - [Load balancing options](https://docs.improbable.io/reference/13.7/shared/worker-configuration/load-balancing#load-balancing)

###Feature 6: Modify the Managed worker startup script so that it updates its local record of entity state upon Authority loss

You may have noticed there is a slight problem with our current implementation of the Managed worker. Recall, we are using a vector to keep a local record of all the entities the worker has authority over, and sending component updates by referencing that vector. We are adding entities to the vector when we receive a message from Spatial letting us know that a Position component has entered our view.

What we  are not doing is taking any action to remove the entities we lose authority over from our vector. This means we are likely sending updates to entities after we lose authority over them, which can lead to errors.

We need to remove entities from our local vector when we lose authority over those entities. To enable this type of syncing between local state and canonical server state, Spatial sends a message to a worker to indicate when there is about to be a change in its authority over a component. The function used to register callbacks to respond to those messages is called onAuthorityChange, and it is passed an op object that contains an enum as a field. [The enum indicates what type of authority change is about to happen](https://docs.improbable.io/reference/13.7/cppsdk/api-reference#worker-authoritychangeop-struct)

Use the OnAuthorityChange function to remove entityWrappers from the vector when the worker is going to lose authority. You may need to track the index of the entitywrapper in the vector to accomplish this.

Your final code should look something like this:

```
dispatcher.OnAuthorityChange<improbable::Position>([&](const worker::AuthorityChangeOp& op) {
        std::cout << "Entity " << op.EntityId << " removed." << std::endl;

        if(op.Authority == worker::Authority::kAuthorityLossImminent){
            int indexInVector = entityIdToVectorIndex.at(op.EntityId);
            std::cout << "Entity " << indexInVector << " removed." << std::endl;
            wrappers.erase(wrappers.begin() + indexInVector);
        }

    });
```

You could have used a map to track the index of EntityWrappers in the vector like so:

```
std::vector<std::string> arguments;
    std::vector<EntityWrapper> wrappers;
    std::map<worker::EntityId, int> entityIdToVectorIndex;
```

```
    dispatcher.OnAddComponent<improbable::Position>([&](const worker::AddComponentOp<improbable::Position>& op) {
        std::cout << "Entity " << op.EntityId << " added." << std::endl;
        entityIdToVectorIndex[op.EntityId] = wrappers.size();
        wrappers.push_back(EntityWrapper(op.EntityId,op.Data.coords()));
    });

```

Reading list for this section
- [The authority enum that is passed to the callback](https://docs.improbable.io/reference/13.7/cppsdk/api-reference#worker-authoritychangeop-struct)


###Feature 7: Add a new worker to the project to simulate a new component

So far, we've worked with only a single worker, which is simulating Position specifically and entity movement more generally. What if we want to add another worker, to simulate a different component and layer of activity in the world.

To add another worker you simply have to add another folder to the workers directory. The folder should be named the same thing as your worker (Note wou'll have to reference the worker name in several places in your code). 

To create a new worker, we'll copy the Managed worker directory, as that directory contains a lot of the boilerplate code necessary for a C++ worker. We'll give the directory a new name, "Example" and add the resulting Example code.

Ultimately, all workers are compiled to executable files that live in the build directory in the SpatialOS project folder. To avoid name collisions with the old Managed worker, youll need to update the project code inside the Example worker in 4 places (this will also teach you how a worker is configured):

  -In the name of spatialos.Managed.worker.json file. This is a [required configuration file for the worker](https://docs.improbable.io/reference/13.7/shared/project-layout/introduction#configuration-file), and it must have the same name as the worker. Change the name to spatialos.Example.worker.json

  -Within the spatialos.Example.worker.json, where there are multiple references to the name of the binary file the worker will be compiled to. Currently these are all Managed@[OS].zip. Change these references to Example@[OS].zip. A find and replace would work well here.

  - In the `/src/startup.cc` file, where the worker type parameter should be changed to Managed.

  - In the CMakeLists.txt file, where every reference to ${PROJECT_NAME} should be changed to "Example" (the cmake file uses that variable to build Make files which referecne the zipped binary that will be created)

   -Finally, (outside of the worker folder), In the launch configuration file we used earlier, to add a section configuring the worker (here, you can use the same configuration parameters as the Managed worker)

Once you've changed those files, revert the startup script in the Example worker to the original in the CPPBlankProject so that the logic about moving the entities is removed.

If you build and run the project you should see a number of Example workers spun up as well.





