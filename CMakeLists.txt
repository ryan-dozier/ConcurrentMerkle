cmake_minimum_required(VERSION 3.15)
project(ConcurrentMerkle)

set(CMAKE_CXX_STANDARD 20)

add_executable(ConcurrentMerkle main.cpp MerkleTree.h md5.cpp md5.h sha256.cpp sha256.h)
