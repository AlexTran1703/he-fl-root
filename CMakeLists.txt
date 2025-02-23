cmake_minimum_required(VERSION 3.10)

# Set the project name and version
project(Homo_Encryption_Project VERSION 1.0)


set(CMAKE_CXX_STANDARD 20)

# Optionally, set include directories for all programs
include_directories(${PROJECT_SOURCE_DIR}/utils)


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

# Set source to compile
file(GLOB_RECURSE SERVER_SOURCES
    server/*
)

file(GLOB_RECURSE CLIENT_SOURCES
    client/*
)

#file(GLOB_RECURSE EXAMPLE_SOURCES
#    third_party/seal_example/*
#)

# Set the path to library if it's not globally installed
# Locate OpenFHE
find_package(OpenFHE REQUIRED)
#find_package(SEAL REQUIRED)
message(STATUS "PATH TO OPHE: ${OpenFHE_INCLUDE} ${OpenFHE_SHARED_LIBRARIES}")
include_directories( ${OPENMP_INCLUDES} )
include_directories( ${OpenFHE_INCLUDE})
include_directories( ${OpenFHE_INCLUDE}/binfhe)
include_directories( ${OpenFHE_INCLUDE}/core)
include_directories( ${OpenFHE_INCLUDE}/pke)
# Include OpenFHE headers

set( CMAKE_EXE_LINKER_FLAGS ${OpenFHE_EXE_LINKER_FLAGS})
link_libraries( ${OpenFHE_SHARED_LIBRARIES} )

# Add the source file to the project
add_executable(client ${CLIENT_SOURCES})
add_executable(server ${SERVER_SOURCES})
#add_executable(seal_example ${EXAMPLE_SOURCES})

#Set require libraries openfhe-core openfhe-pke openfhe-crypto
#set(REQUIRE_LIBS SEAL::seal)
# Link all libraries
target_link_libraries(client ${REQUIRE_LIBS})
target_link_libraries(server ${REQUIRE_LIBS})
#target_link_libraries(seal_example ${REQUIRE_LIBS})