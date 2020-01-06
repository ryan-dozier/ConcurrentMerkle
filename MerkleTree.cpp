//
// Created by Ryan Dozier on 1/6/20.
//

#include "MerkleTree.h"

template<typename T>
void MerkleTree<T>::insert(T &v) {
    size_t hash = hash_data(v);
    this->update(hash, v);
}

template<typename T>
void MerkleTree<T>::remove(T v) {
    size_t hash = hash_data(v);
    this->update(hash, NULL);
}

template<typename T>
void MerkleTree<T>::update(std::size_t hash, T &val) {
    MerkleNode walker = this->root.load();
    std::vector<MerkleNode> visited;

    while(true) {
        if(hash == 0) break;

        visited.push_back(walker);
        if(hash % 2 == 0)
            walker = walker.left.load();
        else
            walker = walker.right.load();
        hash >>= 1;

        if(walker == nullptr) {

            // TODO: This is where I need to potentially use a descriptor object to add a new Node into the structure
            //      Create descriptor
            //      add node to structure
        }
    }

    while(!visited.empty()) {
        walker = visited.pop_back();
        MerkleNode left, right;
        std::size_t temp;
        std::size_t next;
        do {
            next = 0;
            left = walker.left.load();
            right = walker.right.load();
            temp = walker.hash.load();
            if(left != nullptr)
                next += left.hash.load();
            else if(right != nullptr)
                next += right.hash.load();
            next = hash_hashes(next);
        } while(walker.hash.compare_exchange_weak());
    }
}

template<typename T>
bool MerkleTree<T>::validate() {

}

template<typename T>
bool MerkleTree<T>::contains(T val) {
    return this->contains(hash_data(val));
}

template<typename T>
bool MerkleTree<T>::contains(std::size_t hash) {
    bool result = false;
    MerkleNode walker = this->root.load();
    for(int i = 0; i < MAXBITS; i++) {

        if(walker.val != NULL && walker.hash == hash) {
            result = true;
            break;
        }

        if(hash % 2 == 0)
            walker = walker.left.load();
        else
            walker = walker.right.load();
        hash >>= 1;
    }
    return result;
}
