# Install script for directory: /scratch/s5104a21/mpi_danzer/new/mpifileutils/man

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
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/scratch/s5104a21/mpi_danzer/new/install/share/man/man1/dbcast.1;/scratch/s5104a21/mpi_danzer/new/install/share/man/man1/dchmod.1;/scratch/s5104a21/mpi_danzer/new/install/share/man/man1/dcmp.1;/scratch/s5104a21/mpi_danzer/new/install/share/man/man1/dcp.1;/scratch/s5104a21/mpi_danzer/new/install/share/man/man1/ddup.1;/scratch/s5104a21/mpi_danzer/new/install/share/man/man1/dbz2.1;/scratch/s5104a21/mpi_danzer/new/install/share/man/man1/dfind.1;/scratch/s5104a21/mpi_danzer/new/install/share/man/man1/dreln.1;/scratch/s5104a21/mpi_danzer/new/install/share/man/man1/drm.1;/scratch/s5104a21/mpi_danzer/new/install/share/man/man1/dstripe.1;/scratch/s5104a21/mpi_danzer/new/install/share/man/man1/dsync.1;/scratch/s5104a21/mpi_danzer/new/install/share/man/man1/dtar.1;/scratch/s5104a21/mpi_danzer/new/install/share/man/man1/dwalk.1")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/scratch/s5104a21/mpi_danzer/new/install/share/man/man1" TYPE FILE FILES
    "/scratch/s5104a21/mpi_danzer/new/mpifileutils/man/dbcast.1"
    "/scratch/s5104a21/mpi_danzer/new/mpifileutils/man/dchmod.1"
    "/scratch/s5104a21/mpi_danzer/new/mpifileutils/man/dcmp.1"
    "/scratch/s5104a21/mpi_danzer/new/mpifileutils/man/dcp.1"
    "/scratch/s5104a21/mpi_danzer/new/mpifileutils/man/ddup.1"
    "/scratch/s5104a21/mpi_danzer/new/mpifileutils/man/dbz2.1"
    "/scratch/s5104a21/mpi_danzer/new/mpifileutils/man/dfind.1"
    "/scratch/s5104a21/mpi_danzer/new/mpifileutils/man/dreln.1"
    "/scratch/s5104a21/mpi_danzer/new/mpifileutils/man/drm.1"
    "/scratch/s5104a21/mpi_danzer/new/mpifileutils/man/dstripe.1"
    "/scratch/s5104a21/mpi_danzer/new/mpifileutils/man/dsync.1"
    "/scratch/s5104a21/mpi_danzer/new/mpifileutils/man/dtar.1"
    "/scratch/s5104a21/mpi_danzer/new/mpifileutils/man/dwalk.1"
    )
endif()

