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
    MerkleNode * walker = this->root.load();
    MerkleNode * next;
    std::vector<MerkleNode *> visited;
    short dir;
    bool finished = false;

    while(!finished) {
        // mark the current node as a parent node in need of updating
        visited.push_back(walker);
        if(hash % 2 == 0) {
            next = walker->left.load();
            dir = LEFT;
        }
        else {
            next = walker->right.load();
            dir = RIGHT;
        }
        hash >>= 1;

        if(next == nullptr) {
            // CASE HashNode (nonleaf)
            if(walker->val == NULL) {
                MerkleNode * newNode = new MerkleNode(hash, val);

                switch (dir) {
                    case LEFT :
                        if(walker->left.compare_exchange_weak(nullptr, newNode))
                            finished = true;
                        else
                            next = walker->left.load();
                        break;
                    case RIGHT :
                        if(walker->right.compare_exchange_weak(nullptr, newNode))
                            finished = true;
                        else
                            next = walker->right.load();
                        break;
                }
            }
            // CASE DataNode (leaf)
            else {
                MerkleNode * newNode = new MerkleNode();

                switch (walker->hash % 2) {
                    case LEFT :
                        newNode->left.store(walker);
                        break;
                    case RIGHT :
                        newNode->right.store(walker);
                        break;
                }

                switch (dir) {
                    case LEFT :
                        if(walker->left.compare_exchange_weak(next, newNode))
                            next->hash >>= 1;
                        next = walker->left.load();
                        break;
                    case RIGHT :
                        if(walker->right.compare_exchange_weak(next, newNode))
                            next->hash >>= 1;
                        next = walker->right.load();
                        break;
                }
            }
        }
        walker = next;
    }

    // This section performs the hashing operations on the visited nodes simulating a recursive call stack.
    while(!visited.empty()) {

        walker = visited.pop_back();
        MerkleNode * left, right;
        std::size_t temp;
        std::size_t newVal;

        do {
            temp = walker->hash.load();
            newVal = 0;
            left = walker->left.load();
            right = walker->right.load();

            // obtain the hashes from the child nodes
            if(left != nullptr)
                newVal += left->hash.load();
            else if(right != nullptr)
                newVal += right->hash.load();

            // compute the new hashes
            newVal = hash_hashes(newVal);
        } while(walker->hash.compare_exchange_weak(temp, newVal));
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
    MerkleNode * walker = this->root.load();
    for(int i = 0; i < MAXBITS; i++) {

        if(walker->val != NULL && walker->hash.load() == hash) {
            result = true;
            break;
        }

        if(hash % 2 == 0)
            walker = walker->left.load();
        else
            walker = walker->right.load();
        hash >>= 1;
    }
    return result;
}