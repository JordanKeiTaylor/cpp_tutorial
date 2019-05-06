Tutorial overview
	-A spatial OS simulation running
	-The simulation was developed for tutorial purposes using Structured project Structure and C++. In the course of this document, it shows how to go from the Cpp blank structure to a functional project with a single worker moving objects randomly in Brownian Motion.

What's included in this tutorial
	-A snapshot generation script in "/snapshotgenerator"
	-A blank managed worker in /Managed
	-A blank managed


Building the project

From the top level folder, run spatial build, then spatial local start

Build the snapshot generator by




1. Familiarize yourself with the existing project and included workers by running a local simulation

Run spatial build, then spatial local start. Open the inspector at port. Note that the inspector has 3 entities. Play around with the inspector to familiarize yourself with the components and whats on them.

2. Feature 1: Make the managed worker authoritative over the Position component of the example entities

3. Feature 2: See the worker authority and radius in the inspector

4. Feature 3: Set up a game loop and move one entity around the world randomly

5. Feature 4: Capture all the Position components the worker is authoritative over locally

6. Feature 5: Implement Brownian Motion

7. Feature 6: 


