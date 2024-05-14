# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.17

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Disable VCS-based implicit rules.
% : %,v


# Disable VCS-based implicit rules.
% : RCS/%


# Disable VCS-based implicit rules.
% : RCS/%,v


# Disable VCS-based implicit rules.
% : SCCS/s.%


# Disable VCS-based implicit rules.
% : s.%


.SUFFIXES: .hpux_make_needs_suffix_list


# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

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
CMAKE_COMMAND = /apps/applications/cmake/3.17.4/bin/cmake

# The command to remove a file.
RM = /apps/applications/cmake/3.17.4/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /scratch/s5104a21/mpi_danzer/new/mpifileutils

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /scratch/s5104a21/mpi_danzer/new/mpifileutils

# Include any dependencies generated for this target.
include src/dbcast/CMakeFiles/dbcast.dir/depend.make

# Include the progress variables for this target.
include src/dbcast/CMakeFiles/dbcast.dir/progress.make

# Include the compile flags for this target's objects.
include src/dbcast/CMakeFiles/dbcast.dir/flags.make

src/dbcast/CMakeFiles/dbcast.dir/dbcast.c.o: src/dbcast/CMakeFiles/dbcast.dir/flags.make
src/dbcast/CMakeFiles/dbcast.dir/dbcast.c.o: src/dbcast/dbcast.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/scratch/s5104a21/mpi_danzer/new/mpifileutils/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object src/dbcast/CMakeFiles/dbcast.dir/dbcast.c.o"
	cd /scratch/s5104a21/mpi_danzer/new/mpifileutils/src/dbcast && /apps/compiler/gcc/7.2.0/bin/gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/dbcast.dir/dbcast.c.o   -c /scratch/s5104a21/mpi_danzer/new/mpifileutils/src/dbcast/dbcast.c

src/dbcast/CMakeFiles/dbcast.dir/dbcast.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/dbcast.dir/dbcast.c.i"
	cd /scratch/s5104a21/mpi_danzer/new/mpifileutils/src/dbcast && /apps/compiler/gcc/7.2.0/bin/gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /scratch/s5104a21/mpi_danzer/new/mpifileutils/src/dbcast/dbcast.c > CMakeFiles/dbcast.dir/dbcast.c.i

src/dbcast/CMakeFiles/dbcast.dir/dbcast.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/dbcast.dir/dbcast.c.s"
	cd /scratch/s5104a21/mpi_danzer/new/mpifileutils/src/dbcast && /apps/compiler/gcc/7.2.0/bin/gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /scratch/s5104a21/mpi_danzer/new/mpifileutils/src/dbcast/dbcast.c -o CMakeFiles/dbcast.dir/dbcast.c.s

# Object files for target dbcast
dbcast_OBJECTS = \
"CMakeFiles/dbcast.dir/dbcast.c.o"

# External object files for target dbcast
dbcast_EXTERNAL_OBJECTS =

src/dbcast/dbcast: src/dbcast/CMakeFiles/dbcast.dir/dbcast.c.o
src/dbcast/dbcast: src/dbcast/CMakeFiles/dbcast.dir/build.make
src/dbcast/dbcast: src/common/libmfu.so.4.0.0
src/dbcast/dbcast: /usr/lib64/liblustreapi.so
src/dbcast/dbcast: /apps/compiler/gcc/7.2.0/openmpi/3.1.0/lib/libmpi.so
src/dbcast/dbcast: /scratch/s5104a21/mpi_danzer/new/install/lib/libdtcmp.so
src/dbcast/dbcast: /scratch/s5104a21/mpi_danzer/new/install/lib/libarchive.so
src/dbcast/dbcast: /scratch/s5104a21/mpi_danzer/new/install/lib/libcircle.so
src/dbcast/dbcast: /usr/lib64/libbz2.so
src/dbcast/dbcast: src/dbcast/CMakeFiles/dbcast.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/scratch/s5104a21/mpi_danzer/new/mpifileutils/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable dbcast"
	cd /scratch/s5104a21/mpi_danzer/new/mpifileutils/src/dbcast && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/dbcast.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/dbcast/CMakeFiles/dbcast.dir/build: src/dbcast/dbcast

.PHONY : src/dbcast/CMakeFiles/dbcast.dir/build

src/dbcast/CMakeFiles/dbcast.dir/clean:
	cd /scratch/s5104a21/mpi_danzer/new/mpifileutils/src/dbcast && $(CMAKE_COMMAND) -P CMakeFiles/dbcast.dir/cmake_clean.cmake
.PHONY : src/dbcast/CMakeFiles/dbcast.dir/clean

src/dbcast/CMakeFiles/dbcast.dir/depend:
	cd /scratch/s5104a21/mpi_danzer/new/mpifileutils && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /scratch/s5104a21/mpi_danzer/new/mpifileutils /scratch/s5104a21/mpi_danzer/new/mpifileutils/src/dbcast /scratch/s5104a21/mpi_danzer/new/mpifileutils /scratch/s5104a21/mpi_danzer/new/mpifileutils/src/dbcast /scratch/s5104a21/mpi_danzer/new/mpifileutils/src/dbcast/CMakeFiles/dbcast.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/dbcast/CMakeFiles/dbcast.dir/depend

