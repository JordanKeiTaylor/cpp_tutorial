# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.14

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/local/Cellar/cmake/3.14.3/bin/cmake

# The command to remove a file.
RM = /usr/local/Cellar/cmake/3.14.3/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/jordantaylor/Desktop/newcpptutorial/CppBlankProject/snapshotgenerator

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/jordantaylor/Desktop/newcpptutorial/CppBlankProject/snapshotgenerator/cmake-build

# Utility rule file for gameZip.

# Include the progress variables for this target.
include workers/game/CMakeFiles/gameZip.dir/progress.make

workers/game/CMakeFiles/gameZip: workers/game/game
	/usr/local/Cellar/cmake/3.14.3/bin/cmake -E make_directory /Users/jordantaylor/Desktop/newcpptutorial/CppBlankProject/snapshotgenerator/build/assembly/worker
	/usr/local/bin/spatial file zip -b /Users/jordantaylor/Desktop/newcpptutorial/CppBlankProject/snapshotgenerator/cmake-build/workers/game -o /Users/jordantaylor/Desktop/newcpptutorial/CppBlankProject/snapshotgenerator/build/assembly/worker/game --worker_platform=current game

gameZip: workers/game/CMakeFiles/gameZip
gameZip: workers/game/CMakeFiles/gameZip.dir/build.make

.PHONY : gameZip

# Rule to build all files generated by this target.
workers/game/CMakeFiles/gameZip.dir/build: gameZip

.PHONY : workers/game/CMakeFiles/gameZip.dir/build

workers/game/CMakeFiles/gameZip.dir/clean:
	cd /Users/jordantaylor/Desktop/newcpptutorial/CppBlankProject/snapshotgenerator/cmake-build/workers/game && $(CMAKE_COMMAND) -P CMakeFiles/gameZip.dir/cmake_clean.cmake
.PHONY : workers/game/CMakeFiles/gameZip.dir/clean

workers/game/CMakeFiles/gameZip.dir/depend:
	cd /Users/jordantaylor/Desktop/newcpptutorial/CppBlankProject/snapshotgenerator/cmake-build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/jordantaylor/Desktop/newcpptutorial/CppBlankProject/snapshotgenerator /Users/jordantaylor/Desktop/newcpptutorial/CppBlankProject/snapshotgenerator/workers/game /Users/jordantaylor/Desktop/newcpptutorial/CppBlankProject/snapshotgenerator/cmake-build /Users/jordantaylor/Desktop/newcpptutorial/CppBlankProject/snapshotgenerator/cmake-build/workers/game /Users/jordantaylor/Desktop/newcpptutorial/CppBlankProject/snapshotgenerator/cmake-build/workers/game/CMakeFiles/gameZip.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : workers/game/CMakeFiles/gameZip.dir/depend

