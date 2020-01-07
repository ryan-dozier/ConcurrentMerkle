//
// Created by Ryan Dozier on 1/6/20.
//

#ifndef MERKLETREE_H
#define MERKLETREE_H


// Check GCC
#if __GNUC__
#if __x86_64__ || __ppc64__
#define ENVIRONMENT64
#define MAXBITS 64
#else
#define ENVIRONMENT32
        #define MAXBITS 32
#endif
#endif

#include <atomic>
#include <cmath>
#include <cstdlib>
#include <cstddef>
#include <iostream>
#include <array>
#include <vector>
#include <functional>


#define LEFT 0
#define RIGHT 1

template<typename T>
class MerkleTree {
public:
    class MerkleNode;

    inline static MerkleNode* nullNode;
    inline static std::hash<T> hash_data;
    inline static std::hash<size_t> hash_hashes;

    MerkleTree() {
        nullNode = new MerkleNode();
        nullNode->hash.store(0);
        nullNode->val = NULL;
        nullNode->left.store(nullptr);
        nullNode->right.store(nullptr);
        this->root.store(new MerkleNode());
    };
    ~MerkleTree(){ delete root.load(); delete nullNode; };

    void insert(T &v) {
        size_t hash = hash_data(v);
        this->update(hash, v);
    };

    void remove(T v) {
        size_t hash = hash_data(v);
        this->update(hash, NULL);
    };

    bool validate(){};

    bool contains(T val) {
        return this->contains(hash_data(val));
    };

    bool contains(std::size_t hash);
    size_t getRootValue(){ return root.load(); };

private:
    void update(size_t hash, T &val);
    std::atomic<MerkleNode*> root;
};

template<typename T>
class MerkleTree<T>::MerkleNode {
public:
    T val;
    size_t key;
    std::atomic<std::size_t> hash;
    std::atomic<MerkleNode*> left;
    std::atomic<MerkleNode*> right;

    MerkleNode(size_t _hash, size_t key, T &v) {
        this->val = v;
        this->hash.store(_hash);
        this->key = key;
        this->left.store(MerkleTree::nullNode);
        this->right.store(MerkleTree::nullNode);
    };

    MerkleNode() {
        this->hash.store(0);
        this->val = NULL;
        this->key = 0;
        this->left.store(MerkleTree::nullNode);
        this->right.store(MerkleTree::nullNode);
    };

    ~MerkleNode(){};

private:
};


template<typename T>
void MerkleTree<T>::update(std::size_t hash, T &val) {
    MerkleNode* walker = this->root.load();
    MerkleNode* next;
    std::vector<MerkleTree<T>::MerkleNode*> visited;
    std::size_t key = hash;
    short dir;
    bool finished = false;

    while(!finished) {
        // mark the current node as a parent node in need of updating
        if(key % 2 == 0) {
            next = walker->left.load();
            dir = LEFT;
        }
        else {
            next = walker->right.load();
            dir = RIGHT;
        }


        if(next == MerkleTree<T>::nullNode) {
            // CASE HashNode (nonleaf)
            if(walker->val == NULL) {
                key >>= 1;

                MerkleNode* newNode = new MerkleNode(hash, key, val);

                switch (dir) {
                    case LEFT :
                        if(walker->left.compare_exchange_weak(MerkleTree::nullNode, newNode))
                            finished = true;
                        else
                            next = walker->left.load();
                        break;
                    case RIGHT :
                        if(walker->right.compare_exchange_weak(MerkleTree::nullNode, newNode))
                            finished = true;
                        else
                            next = walker->right.load();
                        break;
                }
            }
                // CASE DataNode (leaf)
            else {
                MerkleNode* newNode = new MerkleNode();

                switch (walker->key % 2) {
                    case LEFT :
                        newNode->left.store(walker);
                        break;
                    case RIGHT :
                        newNode->right.store(walker);
                        break;
                }

                MerkleNode* prev = visited.back();
                if(walker == prev->left.load()) {
                    if(prev->left.compare_exchange_weak(walker, newNode)) {
                        walker->key >>= 1;
                        next = newNode;
                    } else {
                        next = prev->left.load();
                    }
                } else if(walker == prev->right.load()) {
                    if(prev->right.compare_exchange_weak(walker, newNode)) {
                        walker->key >>= 1;
                        next = newNode;
                    } else {
                        next = prev->right.load();
                    }
                } else {
                    std::cout << "This is bad" << std::endl;
                }
            }
        } else {
            key >>= 1;
            visited.push_back(walker);
        }
        walker = next;
    }

    // This section performs the hashing operations on the visited nodes simulating a recursive call stack.
    while(!visited.empty()) {

        walker = visited.back();
        visited.pop_back();
        MerkleNode* left;
        MerkleNode* right;
        std::size_t temp;
        std::size_t newVal;

        do {
            temp = walker->hash.load();
            newVal = 0;
            left = walker->left.load();
            right = walker->right.load();

            // obtain the hashes from the child nodes
            if(left != nullNode)
                newVal += left->hash.load();
            else if(right != nullNode)
                newVal += right->hash.load();

            // compute the new hashes
            newVal = hash_hashes(newVal);
        } while(!walker->hash.compare_exchange_weak(temp, newVal));
    }
}

template<typename T>
bool MerkleTree<T>::contains(std::size_t hash) {
    bool result = false;
    size_t key = hash;
    MerkleNode* walker = this->root.load();
    for (int i = 0; i < MAXBITS && walker != MerkleTree<T>::nullNode; i++) {

        if (walker->val != NULL) {
            if (walker->hash.load() == hash)
                result = true;
            break;
        }

        if (key % 2 == 0)
            walker = walker->left.load();
        else
            walker = walker->right.load();
        key >>= 1;
    }
    return result;
}

#endif //MERKLETREE_H
