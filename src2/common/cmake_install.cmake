# Install script for directory: /scratch/s5104a21/mpi_danzer/new/mpifileutils/src/common

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/scratch/s5104a21/mpi_danzer/new/install")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "0")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE FILE FILES
    "/scratch/s5104a21/mpi_danzer/new/mpifileutils/src/common/mfu.h"
    "/scratch/s5104a21/mpi_danzer/new/mpifileutils/src/common/mfu_errors.h"
    "/scratch/s5104a21/mpi_danzer/new/mpifileutils/src/common/mfu_bz2.h"
    "/scratch/s5104a21/mpi_danzer/new/mpifileutils/src/common/mfu_flist.h"
    "/scratch/s5104a21/mpi_danzer/new/mpifileutils/src/common/mfu_flist_internal.h"
    "/scratch/s5104a21/mpi_danzer/new/mpifileutils/src/common/mfu_io.h"
    "/scratch/s5104a21/mpi_danzer/new/mpifileutils/src/common/mfu_param_path.h"
    "/scratch/s5104a21/mpi_danzer/new/mpifileutils/src/common/mfu_path.h"
    "/scratch/s5104a21/mpi_danzer/new/mpifileutils/src/common/mfu_pred.h"
    "/scratch/s5104a21/mpi_danzer/new/mpifileutils/src/common/mfu_proc.h"
    "/scratch/s5104a21/mpi_danzer/new/mpifileutils/src/common/mfu_progress.h"
    "/scratch/s5104a21/mpi_danzer/new/mpifileutils/src/common/mfu_util.h"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/libmfu.so.4.0.0" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/libmfu.so.4.0.0")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/libmfu.so.4.0.0"
         RPATH "/scratch/s5104a21/mpi_danzer/new/install/lib64:/apps/compiler/gcc/7.2.0/openmpi/3.1.0/lib:/scratch/s5104a21/mpi_danzer/new/install/lib")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib64" TYPE SHARED_LIBRARY FILES "/scratch/s5104a21/mpi_danzer/new/mpifileutils/src/common/libmfu.so.4.0.0")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/libmfu.so.4.0.0" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/libmfu.so.4.0.0")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/libmfu.so.4.0.0"
         OLD_RPATH "/apps/compiler/gcc/7.2.0/openmpi/3.1.0/lib:/scratch/s5104a21/mpi_danzer/new/install/lib:::::::::::::::::::::::::::::::::::::::::::::::"
         NEW_RPATH "/scratch/s5104a21/mpi_danzer/new/install/lib64:/apps/compiler/gcc/7.2.0/openmpi/3.1.0/lib:/scratch/s5104a21/mpi_danzer/new/install/lib")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/libmfu.so.4.0.0")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/libmfu.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/libmfu.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/libmfu.so"
         RPATH "/scratch/s5104a21/mpi_danzer/new/install/lib64:/apps/compiler/gcc/7.2.0/openmpi/3.1.0/lib:/scratch/s5104a21/mpi_danzer/new/install/lib")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib64" TYPE SHARED_LIBRARY FILES "/scratch/s5104a21/mpi_danzer/new/mpifileutils/src/common/libmfu.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/libmfu.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/libmfu.so")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/libmfu.so"
         OLD_RPATH "/apps/compiler/gcc/7.2.0/openmpi/3.1.0/lib:/scratch/s5104a21/mpi_danzer/new/install/lib:::::::::::::::::::::::::::::::::::::::::::::::"
         NEW_RPATH "/scratch/s5104a21/mpi_danzer/new/install/lib64:/apps/compiler/gcc/7.2.0/openmpi/3.1.0/lib:/scratch/s5104a21/mpi_danzer/new/install/lib")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/libmfu.so")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib64" TYPE STATIC_LIBRARY FILES "/scratch/s5104a21/mpi_danzer/new/mpifileutils/src/common/libmfu.a")
endif()

