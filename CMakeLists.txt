cmake_minimum_required(VERSION 3.10)

# Set the project name and version
project(Homo_Encryption_Project VERSION 1.0)


set(CMAKE_CXX_STANDARD 20)

# Optionally, set include directories for all programs
include_directories(${PROJECT_SOURCE_DIR}/utils/include)
include(ExternalProject)  # Ensure this line is present

set(CMAKE_CXX_COMPILER "/usr/bin/g++")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wignored-attributes -Wwrite-strings -fopenmp \
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
    utils/src/*
)

file(GLOB_RECURSE CLIENT_SOURCES
    client/*
    utils/src/*
)

file(GLOB_RECURSE UTILS_SOURCES
)
###########################################

###########################################
## Add External Third Party Libraries
# Add OpenFHE as an external project
set(THIRD_PARTY_DIR ${PROJECT_SOURCE_DIR}/third_party)
set(OPENFHE_DIR ${THIRD_PARTY_DIR}/openfhe-development)
set(OPENFHE_INSTALL ${CMAKE_BINARY_DIR}/openfhe)
# Add the external project
ExternalProject_Add(
    OpenFHE
    PREFIX ${OPENFHE_INSTALL}
    SOURCE_DIR ${OPENFHE_DIR}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${OPENFHE_INSTALL} -DUSE_SYSTEM_RAPIDJSON=ON -DRAPIDJSON_DIR=/usr/share/cmake/RapidJSON
    BUILD_ALWAYS OFF
    INSTALL_DIR ${OPENFHE_INSTALL}
)

################################################################
# Locate Static Built Libraries
# Set the path to library if it's not globally installed

##################################################################

###################################################################
# Includes third parties libraries
# And Link dymanic libraries
find_package(RapidJSON REQUIRED)
find_package(OpenFHE REQUIRED)
find_package(OpenSSL REQUIRED)
#OpenMP
include_directories(${OPENMP_INCLUDES})
#OpenFHE
set(OPENFHE_INCLUDE ${OPENFHE_INSTALL}/include)
include_directories(${OPENFHE_INCLUDE}/openfhe/pke)
include_directories(${OPENFHE_INCLUDE}/openfhe)
include_directories(${OPENFHE_INCLUDE}/openfhe/binfhe)
include_directories(${OPENFHE_INCLUDE}/openfhe/core)
link_libraries( ${OpenFHE_SHARED_LIBRARIES} )
set(CMAKE_EXE_LINKER_FLAGS ${OpenFHE_EXE_LINKER_FLAGS})
#RapidJson
include_directories(${RapidJSON_INCLUDE_DIRS})
#OpenSSL
#set(OPENSSL_USE_STATIC_LIBS TRUE)
#set(OPENSSL_CRYPTO_LIBRARY "/usr/local/lib/libcrypto.so")
#set(OPENSSL_SSL_LIBRARY "/usr/local/lib/libssl.so")
include_directories(${OpenSSL_INCLUDE_DIRS})
link_libraries( ${OpenSSL_SHARED_LIBRARIES} )
set(OPENSSL_LIBS -lssl -lcrypto)
####################################################################

####################################################################
set(REQUIRED_LIBS
                 -static-libstdc++ 
                 -static-libgcc 
                 -Wl,-Bstatic 
                 -lc 
                 -Wl,-Bdynamic
                 ${OPENSSL_LIBS}
                )
message(STATUS "OKOKOKO" ${REQUIRED_LIBS})
# Set up source and run the compiler
# Add the source file to the project
add_executable(client ${CLIENT_SOURCES})
add_executable(server ${SERVER_SOURCES})
# Link all libraries ${CMAKE_DL_LIBS}
target_link_libraries(client ${REQUIRE_LIBS} -lssl -lcrypto -static-libstdc++ -static-libgcc -Wl,-Bstatic -lc -Wl,-Bdynamic)
target_link_libraries(server ${REQUIRE_LIBS} -lssl -lcrypto -static-libstdc++ -static-libgcc -Wl,-Bstatic -lc -Wl,-Bdynamic)