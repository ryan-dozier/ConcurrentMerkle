# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.15

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
CMAKE_COMMAND = /Applications/CLion.app/Contents/bin/cmake/mac/bin/cmake

# The command to remove a file.
RM = /Applications/CLion.app/Contents/bin/cmake/mac/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/admin/area67/ConcurrentMerkle

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/admin/area67/ConcurrentMerkle/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/ConcurrentMerkle.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/ConcurrentMerkle.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/ConcurrentMerkle.dir/flags.make

CMakeFiles/ConcurrentMerkle.dir/main.cpp.o: CMakeFiles/ConcurrentMerkle.dir/flags.make
CMakeFiles/ConcurrentMerkle.dir/main.cpp.o: ../main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/admin/area67/ConcurrentMerkle/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/ConcurrentMerkle.dir/main.cpp.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/ConcurrentMerkle.dir/main.cpp.o -c /Users/admin/area67/ConcurrentMerkle/main.cpp

CMakeFiles/ConcurrentMerkle.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/ConcurrentMerkle.dir/main.cpp.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/admin/area67/ConcurrentMerkle/main.cpp > CMakeFiles/ConcurrentMerkle.dir/main.cpp.i

CMakeFiles/ConcurrentMerkle.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/ConcurrentMerkle.dir/main.cpp.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/admin/area67/ConcurrentMerkle/main.cpp -o CMakeFiles/ConcurrentMerkle.dir/main.cpp.s

# Object files for target ConcurrentMerkle
ConcurrentMerkle_OBJECTS = \
"CMakeFiles/ConcurrentMerkle.dir/main.cpp.o"

# External object files for target ConcurrentMerkle
ConcurrentMerkle_EXTERNAL_OBJECTS =

ConcurrentMerkle: CMakeFiles/ConcurrentMerkle.dir/main.cpp.o
ConcurrentMerkle: CMakeFiles/ConcurrentMerkle.dir/build.make
ConcurrentMerkle: CMakeFiles/ConcurrentMerkle.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/admin/area67/ConcurrentMerkle/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ConcurrentMerkle"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/ConcurrentMerkle.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/ConcurrentMerkle.dir/build: ConcurrentMerkle

.PHONY : CMakeFiles/ConcurrentMerkle.dir/build

CMakeFiles/ConcurrentMerkle.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/ConcurrentMerkle.dir/cmake_clean.cmake
.PHONY : CMakeFiles/ConcurrentMerkle.dir/clean

CMakeFiles/ConcurrentMerkle.dir/depend:
	cd /Users/admin/area67/ConcurrentMerkle/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/admin/area67/ConcurrentMerkle /Users/admin/area67/ConcurrentMerkle /Users/admin/area67/ConcurrentMerkle/cmake-build-debug /Users/admin/area67/ConcurrentMerkle/cmake-build-debug /Users/admin/area67/ConcurrentMerkle/cmake-build-debug/CMakeFiles/ConcurrentMerkle.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/ConcurrentMerkle.dir/depend

