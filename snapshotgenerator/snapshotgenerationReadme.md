If downloading old Othello from scratch ensure that line 22 of CMakeLists.txt in the schema folder looks like this:
add_dependencies(Schema schema-gen dependencies)

And ensure that the depencies names in the cmake snapshot folder look like this:

target_link_libraries(${PROJECT_NAME} Schema WorkerSdk CoreSdk protobuf RakNetLibStatic grpc++ grpc gpr ssl z)



COmmand line to run:


Jordans-MacBook-Pro:Othello jordantaylor$ mkdir cmake-build
Jordans-MacBook-Pro:Othello jordantaylor$ cd cmake-build/
Jordans-MacBook-Pro:cmake-build jordantaylor$ cmake -G "Unix Makefiles" ..
-- The C compiler identification is AppleClang 10.0.1.10010046
-- The CXX compiler identification is AppleClang 10.0.1.10010046
-- Check for working C compiler: /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc
-- Check for working C compiler: /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc -- works
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
-- Detecting C compile features
-- Detecting C compile features - done
-- Check for working CXX compiler: /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++
-- Check for working CXX compiler: /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ -- works
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- Configuring done
-- Generating done
-- Build files have been written to: /Users/jordantaylor/Desktop/workingOthello/Othello/cmake-build
Jordans-MacBook-Pro:cmake-build jordantaylor$ ls
CMakeCache.txt			schema
CMakeFiles			snapshots
Makefile			spatialos_worker_packages.json
cmake_install.cmake		workers
Jordans-MacBook-Pro:cmake-build jordantaylor$ cd snapshots/
Jordans-MacBook-Pro:snapshots jordantaylor$ make
Scanning dependencies of target schema-gen
A newer version of spatial is available. Run 'spatial update' to get the latest version.
For more information, see https://docs.improbable.io/reference/latest/shared/release-policy#spatial-cli-releases
Extracting packages 1/1    [====================] 100%
Generating code for cpp. 1/1    [====================] 100%
'spatial schema generate' succeeded (0.4s)
[  0%] Built target schema-gen
Scanning dependencies of target dependencies
A newer version of spatial is available. Run 'spatial update' to get the latest version.
For more information, see https://docs.improbable.io/reference/latest/shared/release-policy#spatial-cli-releases
Extracting packages 1/1    [====================] 100%
'spatial package unpack' succeeded (0.4s)
[  0%] Built target dependencies
Scanning dependencies of target Schema
[ 16%] Building CXX object schema/CMakeFiles/Schema.dir/__/generated_src/othello.cc.o
[ 33%] Building CXX object schema/CMakeFiles/Schema.dir/__/generated_src/improbable/standard_library.cc.o
[ 50%] Building CXX object schema/CMakeFiles/Schema.dir/__/generated_src/improbable/vector3.cc.o
[ 66%] Linking CXX static library libSchema.a
[ 66%] Built target Schema
Scanning dependencies of target create_snapshot
[ 83%] Building CXX object snapshots/CMakeFiles/create_snapshot.dir/main.cpp.o
[100%] Linking CXX executable create_snapshot
[100%] Built target create_snapshot
Scanning dependencies of target create_default_snapshot
[100%] Built target create_default_snapshot
Jordans-MacBook-Pro:snapshots jordantaylor$ less Makefile
Jordans-MacBook-Pro:snapshots jordantaylor$ ls
CMakeFiles		cmake_install.cmake
Makefile		create_snapshot
Jordans-MacBook-Pro:snapshots jordantaylor$ open .
Jordans-MacBook-Pro:snapshots jordantaylor$ ls
CMakeFiles		cmake_install.cmake
Makefile		create_snapshot
Jordans-MacBook-Pro:snapshots jordantaylor$ make create_default_snapshot
A newer version of spatial is available. Run 'spatial update' to get the latest version.
For more information, see https://docs.improbable.io/reference/latest/shared/release-policy#spatial-cli-releases
No changes detected, skipping code generation.
'spatial schema generate' succeeded (0.1s)
[  0%] Built target schema-gen
A newer version of spatial is available. Run 'spatial update' to get the latest version.
For more information, see https://docs.improbable.io/reference/latest/shared/release-policy#spatial-cli-releases
'spatial package unpack' succeeded (0.2s)
[  0%] Built target dependencies
[ 66%] Built target Schema
[100%] Built target create_snapshot
[100%] Built target create_default_snapshot
Jordans-MacBook-Pro:snapshots jordantaylor$ ls
CMakeFiles		cmake_install.cmake
Makefile		create_snapshot
Jordans-MacBook-Pro:snapshots jordantaylor$ cd ..
Jordans-MacBook-Pro:cmake-build jordantaylor$ ls
CMakeCache.txt			generated_src
CMakeFiles			schema
Makefile			snapshots
cmake_install.cmake		spatialos_worker_packages.json
dependencies			workers
Jordans-MacBook-Pro:cmake-build jordantaylor$ cd ..
Jordans-MacBook-Pro:Othello jordantaylor$ ls
CMakeLists.txt				schema
build					snapshots
cmake-build				spatialos.json
default_launch.json			spatialos_worker_packages.json.in
logs					workers
Jordans-MacBook-Pro:Othello jordantaylor$ cd snapshots/
Jordans-MacBook-Pro:snapshots jordantaylor$ ls
CMakeLists.txt		default.snapshot	main.cpp
Jordans-MacBook-Pro:snapshots jordantaylor$ open .
Jordans-MacBook-Pro:snapshots jordantaylor$ cd ..
Jordans-MacBook-Pro:Othello jordantaylor$ ls
CMakeLists.txt				schema
build					snapshots
cmake-build				spatialos.json
default_launch.json			spatialos_worker_packages.json.in
logs					workers
Jordans-MacBook-Pro:Othello jordantaylor$ cd cmake-build/
Jordans-MacBook-Pro:cmake-build jordantaylor$ ls
CMakeCache.txt			generated_src
CMakeFiles			schema
Makefile			snapshots
cmake_install.cmake		spatialos_worker_packages.json
dependencies			workers
Jordans-MacBook-Pro:cmake-build jordantaylor$ make
A newer version of spatial is available. Run 'spatial update' to get the latest version.
For more information, see https://docs.improbable.io/reference/latest/shared/release-policy#spatial-cli-releases
'spatial package unpack' succeeded (0.2s)
[  0%] Built target dependencies
A newer version of spatial is available. Run 'spatial update' to get the latest version.
For more information, see https://docs.improbable.io/reference/latest/shared/release-policy#spatial-cli-releases
No changes detected, skipping code generation.
'spatial schema generate' succeeded (0.1s)
[  0%] Built target schema-gen
[ 40%] Built target Schema
Scanning dependencies of target game
[ 50%] Building CXX object workers/game/CMakeFiles/game.dir/src/game.cpp.o
/Users/jordantaylor/Desktop/workingOthello/Othello/workers/game/src/game.cpp:270:31: error: 
      use 'template' keyword to treat 'Get' as a dependent template name
            if (entity.second.Get<othello::Color>()->black() == op.Reque...
                              ^
                              template 
1 error generated.
make[2]: *** [workers/game/CMakeFiles/game.dir/src/game.cpp.o] Error 1
make[1]: *** [workers/game/CMakeFiles/game.dir/all] Error 2
make: *** [all] Error 2
