# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.10

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Produce verbose output by default.
VERBOSE = 1

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
CMAKE_COMMAND = /usr/local/bin/cmake

# The command to remove a file.
RM = /usr/local/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /root/MyServer/tiger

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /root/MyServer/tiger/build

# Include any dependencies generated for this target.
include CMakeFiles/tiger.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/tiger.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/tiger.dir/flags.make

CMakeFiles/tiger.dir/src/config.cc.o: CMakeFiles/tiger.dir/flags.make
CMakeFiles/tiger.dir/src/config.cc.o: ../src/config.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/root/MyServer/tiger/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/tiger.dir/src/config.cc.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/tiger.dir/src/config.cc.o -c /root/MyServer/tiger/src/config.cc

CMakeFiles/tiger.dir/src/config.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/tiger.dir/src/config.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /root/MyServer/tiger/src/config.cc > CMakeFiles/tiger.dir/src/config.cc.i

CMakeFiles/tiger.dir/src/config.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/tiger.dir/src/config.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /root/MyServer/tiger/src/config.cc -o CMakeFiles/tiger.dir/src/config.cc.s

CMakeFiles/tiger.dir/src/config.cc.o.requires:

.PHONY : CMakeFiles/tiger.dir/src/config.cc.o.requires

CMakeFiles/tiger.dir/src/config.cc.o.provides: CMakeFiles/tiger.dir/src/config.cc.o.requires
	$(MAKE) -f CMakeFiles/tiger.dir/build.make CMakeFiles/tiger.dir/src/config.cc.o.provides.build
.PHONY : CMakeFiles/tiger.dir/src/config.cc.o.provides

CMakeFiles/tiger.dir/src/config.cc.o.provides.build: CMakeFiles/tiger.dir/src/config.cc.o


CMakeFiles/tiger.dir/src/log.cc.o: CMakeFiles/tiger.dir/flags.make
CMakeFiles/tiger.dir/src/log.cc.o: ../src/log.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/root/MyServer/tiger/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/tiger.dir/src/log.cc.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/tiger.dir/src/log.cc.o -c /root/MyServer/tiger/src/log.cc

CMakeFiles/tiger.dir/src/log.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/tiger.dir/src/log.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /root/MyServer/tiger/src/log.cc > CMakeFiles/tiger.dir/src/log.cc.i

CMakeFiles/tiger.dir/src/log.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/tiger.dir/src/log.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /root/MyServer/tiger/src/log.cc -o CMakeFiles/tiger.dir/src/log.cc.s

CMakeFiles/tiger.dir/src/log.cc.o.requires:

.PHONY : CMakeFiles/tiger.dir/src/log.cc.o.requires

CMakeFiles/tiger.dir/src/log.cc.o.provides: CMakeFiles/tiger.dir/src/log.cc.o.requires
	$(MAKE) -f CMakeFiles/tiger.dir/build.make CMakeFiles/tiger.dir/src/log.cc.o.provides.build
.PHONY : CMakeFiles/tiger.dir/src/log.cc.o.provides

CMakeFiles/tiger.dir/src/log.cc.o.provides.build: CMakeFiles/tiger.dir/src/log.cc.o


CMakeFiles/tiger.dir/src/util.cc.o: CMakeFiles/tiger.dir/flags.make
CMakeFiles/tiger.dir/src/util.cc.o: ../src/util.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/root/MyServer/tiger/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/tiger.dir/src/util.cc.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/tiger.dir/src/util.cc.o -c /root/MyServer/tiger/src/util.cc

CMakeFiles/tiger.dir/src/util.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/tiger.dir/src/util.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /root/MyServer/tiger/src/util.cc > CMakeFiles/tiger.dir/src/util.cc.i

CMakeFiles/tiger.dir/src/util.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/tiger.dir/src/util.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /root/MyServer/tiger/src/util.cc -o CMakeFiles/tiger.dir/src/util.cc.s

CMakeFiles/tiger.dir/src/util.cc.o.requires:

.PHONY : CMakeFiles/tiger.dir/src/util.cc.o.requires

CMakeFiles/tiger.dir/src/util.cc.o.provides: CMakeFiles/tiger.dir/src/util.cc.o.requires
	$(MAKE) -f CMakeFiles/tiger.dir/build.make CMakeFiles/tiger.dir/src/util.cc.o.provides.build
.PHONY : CMakeFiles/tiger.dir/src/util.cc.o.provides

CMakeFiles/tiger.dir/src/util.cc.o.provides.build: CMakeFiles/tiger.dir/src/util.cc.o


CMakeFiles/tiger.dir/src/thread.cc.o: CMakeFiles/tiger.dir/flags.make
CMakeFiles/tiger.dir/src/thread.cc.o: ../src/thread.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/root/MyServer/tiger/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/tiger.dir/src/thread.cc.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/tiger.dir/src/thread.cc.o -c /root/MyServer/tiger/src/thread.cc

CMakeFiles/tiger.dir/src/thread.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/tiger.dir/src/thread.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /root/MyServer/tiger/src/thread.cc > CMakeFiles/tiger.dir/src/thread.cc.i

CMakeFiles/tiger.dir/src/thread.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/tiger.dir/src/thread.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /root/MyServer/tiger/src/thread.cc -o CMakeFiles/tiger.dir/src/thread.cc.s

CMakeFiles/tiger.dir/src/thread.cc.o.requires:

.PHONY : CMakeFiles/tiger.dir/src/thread.cc.o.requires

CMakeFiles/tiger.dir/src/thread.cc.o.provides: CMakeFiles/tiger.dir/src/thread.cc.o.requires
	$(MAKE) -f CMakeFiles/tiger.dir/build.make CMakeFiles/tiger.dir/src/thread.cc.o.provides.build
.PHONY : CMakeFiles/tiger.dir/src/thread.cc.o.provides

CMakeFiles/tiger.dir/src/thread.cc.o.provides.build: CMakeFiles/tiger.dir/src/thread.cc.o


CMakeFiles/tiger.dir/src/MyGo/context/jump_x86_64_sysv_elf_gas.S.o: CMakeFiles/tiger.dir/flags.make
CMakeFiles/tiger.dir/src/MyGo/context/jump_x86_64_sysv_elf_gas.S.o: ../src/MyGo/context/jump_x86_64_sysv_elf_gas.S
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/root/MyServer/tiger/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building ASM object CMakeFiles/tiger.dir/src/MyGo/context/jump_x86_64_sysv_elf_gas.S.o"
	/usr/bin/cc $(ASM_DEFINES) $(ASM_INCLUDES) $(ASM_FLAGS) -o CMakeFiles/tiger.dir/src/MyGo/context/jump_x86_64_sysv_elf_gas.S.o -c /root/MyServer/tiger/src/MyGo/context/jump_x86_64_sysv_elf_gas.S

CMakeFiles/tiger.dir/src/MyGo/context/jump_x86_64_sysv_elf_gas.S.o.requires:

.PHONY : CMakeFiles/tiger.dir/src/MyGo/context/jump_x86_64_sysv_elf_gas.S.o.requires

CMakeFiles/tiger.dir/src/MyGo/context/jump_x86_64_sysv_elf_gas.S.o.provides: CMakeFiles/tiger.dir/src/MyGo/context/jump_x86_64_sysv_elf_gas.S.o.requires
	$(MAKE) -f CMakeFiles/tiger.dir/build.make CMakeFiles/tiger.dir/src/MyGo/context/jump_x86_64_sysv_elf_gas.S.o.provides.build
.PHONY : CMakeFiles/tiger.dir/src/MyGo/context/jump_x86_64_sysv_elf_gas.S.o.provides

CMakeFiles/tiger.dir/src/MyGo/context/jump_x86_64_sysv_elf_gas.S.o.provides.build: CMakeFiles/tiger.dir/src/MyGo/context/jump_x86_64_sysv_elf_gas.S.o


CMakeFiles/tiger.dir/src/MyGo/context/make_x86_64_sysv_elf_gas.S.o: CMakeFiles/tiger.dir/flags.make
CMakeFiles/tiger.dir/src/MyGo/context/make_x86_64_sysv_elf_gas.S.o: ../src/MyGo/context/make_x86_64_sysv_elf_gas.S
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/root/MyServer/tiger/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building ASM object CMakeFiles/tiger.dir/src/MyGo/context/make_x86_64_sysv_elf_gas.S.o"
	/usr/bin/cc $(ASM_DEFINES) $(ASM_INCLUDES) $(ASM_FLAGS) -o CMakeFiles/tiger.dir/src/MyGo/context/make_x86_64_sysv_elf_gas.S.o -c /root/MyServer/tiger/src/MyGo/context/make_x86_64_sysv_elf_gas.S

CMakeFiles/tiger.dir/src/MyGo/context/make_x86_64_sysv_elf_gas.S.o.requires:

.PHONY : CMakeFiles/tiger.dir/src/MyGo/context/make_x86_64_sysv_elf_gas.S.o.requires

CMakeFiles/tiger.dir/src/MyGo/context/make_x86_64_sysv_elf_gas.S.o.provides: CMakeFiles/tiger.dir/src/MyGo/context/make_x86_64_sysv_elf_gas.S.o.requires
	$(MAKE) -f CMakeFiles/tiger.dir/build.make CMakeFiles/tiger.dir/src/MyGo/context/make_x86_64_sysv_elf_gas.S.o.provides.build
.PHONY : CMakeFiles/tiger.dir/src/MyGo/context/make_x86_64_sysv_elf_gas.S.o.provides

CMakeFiles/tiger.dir/src/MyGo/context/make_x86_64_sysv_elf_gas.S.o.provides.build: CMakeFiles/tiger.dir/src/MyGo/context/make_x86_64_sysv_elf_gas.S.o


CMakeFiles/tiger.dir/src/MyGo/context/context.cpp.o: CMakeFiles/tiger.dir/flags.make
CMakeFiles/tiger.dir/src/MyGo/context/context.cpp.o: ../src/MyGo/context/context.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/root/MyServer/tiger/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Building CXX object CMakeFiles/tiger.dir/src/MyGo/context/context.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/tiger.dir/src/MyGo/context/context.cpp.o -c /root/MyServer/tiger/src/MyGo/context/context.cpp

CMakeFiles/tiger.dir/src/MyGo/context/context.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/tiger.dir/src/MyGo/context/context.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /root/MyServer/tiger/src/MyGo/context/context.cpp > CMakeFiles/tiger.dir/src/MyGo/context/context.cpp.i

CMakeFiles/tiger.dir/src/MyGo/context/context.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/tiger.dir/src/MyGo/context/context.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /root/MyServer/tiger/src/MyGo/context/context.cpp -o CMakeFiles/tiger.dir/src/MyGo/context/context.cpp.s

CMakeFiles/tiger.dir/src/MyGo/context/context.cpp.o.requires:

.PHONY : CMakeFiles/tiger.dir/src/MyGo/context/context.cpp.o.requires

CMakeFiles/tiger.dir/src/MyGo/context/context.cpp.o.provides: CMakeFiles/tiger.dir/src/MyGo/context/context.cpp.o.requires
	$(MAKE) -f CMakeFiles/tiger.dir/build.make CMakeFiles/tiger.dir/src/MyGo/context/context.cpp.o.provides.build
.PHONY : CMakeFiles/tiger.dir/src/MyGo/context/context.cpp.o.provides

CMakeFiles/tiger.dir/src/MyGo/context/context.cpp.o.provides.build: CMakeFiles/tiger.dir/src/MyGo/context/context.cpp.o


# Object files for target tiger
tiger_OBJECTS = \
"CMakeFiles/tiger.dir/src/config.cc.o" \
"CMakeFiles/tiger.dir/src/log.cc.o" \
"CMakeFiles/tiger.dir/src/util.cc.o" \
"CMakeFiles/tiger.dir/src/thread.cc.o" \
"CMakeFiles/tiger.dir/src/MyGo/context/jump_x86_64_sysv_elf_gas.S.o" \
"CMakeFiles/tiger.dir/src/MyGo/context/make_x86_64_sysv_elf_gas.S.o" \
"CMakeFiles/tiger.dir/src/MyGo/context/context.cpp.o"

# External object files for target tiger
tiger_EXTERNAL_OBJECTS =

../lib/libtiger.so: CMakeFiles/tiger.dir/src/config.cc.o
../lib/libtiger.so: CMakeFiles/tiger.dir/src/log.cc.o
../lib/libtiger.so: CMakeFiles/tiger.dir/src/util.cc.o
../lib/libtiger.so: CMakeFiles/tiger.dir/src/thread.cc.o
../lib/libtiger.so: CMakeFiles/tiger.dir/src/MyGo/context/jump_x86_64_sysv_elf_gas.S.o
../lib/libtiger.so: CMakeFiles/tiger.dir/src/MyGo/context/make_x86_64_sysv_elf_gas.S.o
../lib/libtiger.so: CMakeFiles/tiger.dir/src/MyGo/context/context.cpp.o
../lib/libtiger.so: CMakeFiles/tiger.dir/build.make
../lib/libtiger.so: CMakeFiles/tiger.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/root/MyServer/tiger/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_8) "Linking CXX shared library ../lib/libtiger.so"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/tiger.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/tiger.dir/build: ../lib/libtiger.so

.PHONY : CMakeFiles/tiger.dir/build

CMakeFiles/tiger.dir/requires: CMakeFiles/tiger.dir/src/config.cc.o.requires
CMakeFiles/tiger.dir/requires: CMakeFiles/tiger.dir/src/log.cc.o.requires
CMakeFiles/tiger.dir/requires: CMakeFiles/tiger.dir/src/util.cc.o.requires
CMakeFiles/tiger.dir/requires: CMakeFiles/tiger.dir/src/thread.cc.o.requires
CMakeFiles/tiger.dir/requires: CMakeFiles/tiger.dir/src/MyGo/context/jump_x86_64_sysv_elf_gas.S.o.requires
CMakeFiles/tiger.dir/requires: CMakeFiles/tiger.dir/src/MyGo/context/make_x86_64_sysv_elf_gas.S.o.requires
CMakeFiles/tiger.dir/requires: CMakeFiles/tiger.dir/src/MyGo/context/context.cpp.o.requires

.PHONY : CMakeFiles/tiger.dir/requires

CMakeFiles/tiger.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/tiger.dir/cmake_clean.cmake
.PHONY : CMakeFiles/tiger.dir/clean

CMakeFiles/tiger.dir/depend:
	cd /root/MyServer/tiger/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /root/MyServer/tiger /root/MyServer/tiger /root/MyServer/tiger/build /root/MyServer/tiger/build /root/MyServer/tiger/build/CMakeFiles/tiger.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/tiger.dir/depend
