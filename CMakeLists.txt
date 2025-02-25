cmake_minimum_required(VERSION 3.10)

# Set the project name and version
project(Homo_Encryption_Project VERSION 1.0)


set(CMAKE_CXX_STANDARD 20)

# Optionally, set include directories for all programs
include_directories(${PROJECT_SOURCE_DIR}/utils)
include(ExternalProject)  # Ensure this line is present

set(CMAKE_CXX_COMPILER "/usr/bin/g++")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -fopenmp \
                        -Wno-missing-field-initializers -Wno-unused-parameter")   # For C++ compiler
# Optionally, add custom configurations based on build type
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "+[BUILDING MODE] Debug build is selected.")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -DDEBUG_MODE")  # Debug flags
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS "+[BUILDING MODE] Release build is selected.")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O3")  # Debug flags
endif()

###########################################
# Set source to compile
file(GLOB_RECURSE SERVER_SOURCES
    server/*
)

file(GLOB_RECURSE CLIENT_SOURCES
    client/*
)
###########################################

###########################################
## Add External Third Party Libraries
# Add OpenFHE as an external project
set(THIRD_PARTY_DIR ${PROJECT_SOURCE_DIR}/third_party)
set(OPENFHE_DIR ${THIRD_PARTY_DIR}/openfhe-development)
set(OPENFHE_INSTALL ${OPENFHE_DIR}/install)
# Add the external project
ExternalProject_Add(
    OpenFHE
    PREFIX ${OPENFHE_INSTALL}
    SOURCE_DIR ${OPENFHE_DIR}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${OPENFHE_INSTALL} -DBUILD_SHARED_LIBS=ON
    BUILD_ALWAYS OFF
    INSTALL_DIR ${OPENFHE_INSTALL}
)

################################################################
# Locate Static Built Libraries
# Set the path to library if it's not globally installed

#find_package(SEAL REQUIRED)

##################################################################

###################################################################
# Includes third parties libraries
# And Link dymanic libraries
find_package(OpenFHE REQUIRED)

message(STATUS "PATH TO OpenFHE: ${OpenFHE_INCLUDE} ${OpenFHE_SHARED_LIBRARIES}")
include_directories(${OPENMP_INCLUDES})
include_directories(${OpenFHE_INCLUDE})
include_directories(${OpenFHE_INCLUDE}/binfhe)
include_directories(${OpenFHE_INCLUDE}/core)
include_directories(${OpenFHE_INCLUDE}/pke)
set(CMAKE_EXE_LINKER_FLAGS ${OpenFHE_EXE_LINKER_FLAGS})
link_libraries( ${OpenFHE_SHARED_LIBRARIES} )

####################################################################

####################################################################
set(REQUIRED_LIBS ${CMAKE_DL_LIBS} 
                -static-libstdc++ -static-libgcc -Wl,-Bstatic -lc -Wl,-Bdynamic ${CMAKE_DL_LIBS})
# Set up source and run the compiler
# Add the source file to the project
add_executable(client ${CLIENT_SOURCES})
add_executable(server ${SERVER_SOURCES})
# Link all libraries ${CMAKE_DL_LIBS}
target_link_libraries(client ${REQUIRE_LIBS})
target_link_libraries(server ${REQUIRE_LIBS})