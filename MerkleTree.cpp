//
// Created by Ryan Dozier on 1/6/20.
//

#include "MerkleTree.h"

template<typename T>
void MerkleTree<T>::insert(T &v) {

}

template<typename T>
bool MerkleTree<T>::validate() {

}

template<typename T>
bool MerkleTree<T>::contains(T val) {

}

template<typename T>
bool MerkleTree<T>::contains(std::size_t hash) {

}

template<typename T>
bool MerkleTree<T>::delete_hash(std::size_t hash) {

}




/*
 * Because iterating these may not be needed.
template<typename T>
bool MerkleTree<T>::MerkleNode::contains(std::size_t hash) {
    // while key > 0
}

template<typename T>
bool MerkleTree<T>::MerkleNode::insert(std::size_t key, T &v) {
    // while key > 0
}

template<typename T>
bool MerkleTree<T>::MerkleNode::delete_hash(std::string hash) {
    // while key > 0
}
 */