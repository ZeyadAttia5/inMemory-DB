cmake_minimum_required(VERSION 3.12)
project(inMemory-db-server)

# Set the C++ standard to C++17 (or your desired version)
set(CMAKE_CXX_STANDARD 17)

# Specify the source files for the server
set(SERVER_SOURCES
    ./source/server.cpp
    ./source/hashtable.cpp
    ./source/serialization.cpp
    ./source/avl.cpp
    ./source/heap.cpp

)
set(CMAKE_BUILD_TYPE Debug)
# Specify the header files for the server (if any)
set(SERVER_HEADERS
    ./include/hashtable.h
    ./include/serialization.h
    ./include/avl.hpp
    ./include/heap.hpp
)

# Create the executable target for the server
add_executable(inMemory-DB ${SERVER_SOURCES} ${SERVER_HEADERS})

# Specify include directory for the "server"
target_include_directories(inMemory-DB
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/server
)

# You can add any additional libraries or dependencies here.
# For example, if you are using a library like Boost:
# find_package(Boost REQUIRED)
# target_link_libraries(inMemory-db-server PRIVATE Boost::Boost)

# Set compiler flags (optional)
# For example, to enable warnings and optimizations:
# target_compile_options(inMemory-db-server PRIVATE -Wall -Wextra -O2)
