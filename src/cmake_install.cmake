# Install script for directory: /scratch/s5104a21/mpi_danzer/new/mpifileutils/src

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

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/scratch/s5104a21/mpi_danzer/new/mpifileutils/src/common/cmake_install.cmake")
  include("/scratch/s5104a21/mpi_danzer/new/mpifileutils/src/dbcast/cmake_install.cmake")
  include("/scratch/s5104a21/mpi_danzer/new/mpifileutils/src/dbz2/cmake_install.cmake")
  include("/scratch/s5104a21/mpi_danzer/new/mpifileutils/src/dchmod/cmake_install.cmake")
  include("/scratch/s5104a21/mpi_danzer/new/mpifileutils/src/dcmp/cmake_install.cmake")
  include("/scratch/s5104a21/mpi_danzer/new/mpifileutils/src/dcp/cmake_install.cmake")
  include("/scratch/s5104a21/mpi_danzer/new/mpifileutils/src/dcp1/cmake_install.cmake")
  include("/scratch/s5104a21/mpi_danzer/new/mpifileutils/src/ddup/cmake_install.cmake")
  include("/scratch/s5104a21/mpi_danzer/new/mpifileutils/src/dfilemaker1/cmake_install.cmake")
  include("/scratch/s5104a21/mpi_danzer/new/mpifileutils/src/dfind/cmake_install.cmake")
  include("/scratch/s5104a21/mpi_danzer/new/mpifileutils/src/dreln/cmake_install.cmake")
  include("/scratch/s5104a21/mpi_danzer/new/mpifileutils/src/drm/cmake_install.cmake")
  include("/scratch/s5104a21/mpi_danzer/new/mpifileutils/src/dstripe/cmake_install.cmake")
  include("/scratch/s5104a21/mpi_danzer/new/mpifileutils/src/dsync/cmake_install.cmake")
  include("/scratch/s5104a21/mpi_danzer/new/mpifileutils/src/dtar/cmake_install.cmake")
  include("/scratch/s5104a21/mpi_danzer/new/mpifileutils/src/dwalk/cmake_install.cmake")

endif()

