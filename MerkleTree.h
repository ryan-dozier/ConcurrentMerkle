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
#include <string>
#include <iostream>

#define LEFT 0
#define RIGHT 1

template<typename T>
class MerkleTree {
public:
    static std::hash<T> hash_hashes;
    static std::hash<T> hash_data;
    class MerkleNode;

    MerkleTree();
    ~MerkleTree(){ delete root.load(); };
    void insert(T &v);
    void remove(T v);
    bool validate();
    bool contains(T val);
    bool contains(std::size_t hash);
    size_t getRootValue(){ return root.load(); };

private:
    void update(size_t hash, T &val);
    std::atomic<MerkleNode *> root;
};

template<typename T>
class MerkleTree<T>::MerkleNode {
public:
    T val;
    std::atomic<std::size_t> hash;

    MerkleNode(size_t _hash, T &v);
    MerkleNode();
    ~MerkleNode(){};
    std::atomic<MerkleNode *> left;
    std::atomic<MerkleNode *> right;
private:
};


template<typename T>
MerkleTree<T>::MerkleTree() {
    this->root.store(new MerkleNode());
}

template<typename T>
MerkleTree<T>::MerkleNode::MerkleNode(size_t _hash, T &v) {
    this->val = v;
    this->hash.store(_hash);
    this->left.store(nullptr);
    this->right.store(nullptr);
}

template<typename T>
MerkleTree<T>::MerkleNode::MerkleNode() {
    this->hash.store(0);
    this->val = NULL;
    this->left.store(nullptr);
    this->right.store(nullptr);
}

#endif //MERKLETREE_H