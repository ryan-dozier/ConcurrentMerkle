#SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "/usr/local/Cellar/boost")
#SET(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "/usr/local/Cellar/boost/1.72.0_2/lib")

option(Boost_DEBUG "Set to ON to enable debug output from FindBoost." ON)
option(Boost_DETAILED_FAILURE_MSG "Set to ON to get detailed information" ON)

cmake_minimum_required(VERSION 3.15)
project(ConcurrentMerkle)

set(CMAKE_CXX_STANDARD 20)
find_package(Boost 1.72.0)

#find_package(Boost 1.72.0 COMPONENTS functional)
add_executable(ConcurrentMerkle main.cpp MerkleTree.h md5.cpp md5.h sha256.cpp sha256.h setMain.cpp)
add_executable(SetVerifier setMain.cpp MerkleTreeVerifier.h md5.cpp md5.h sha256.cpp sha256.h)
include_directories(${Boost_INCLUDE_DIR})
target_link_libraries(SetVerifier ${Boost_LIBRARIES})


