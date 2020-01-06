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
    ~MerkleTree(){};
    void insert(T &v);
    void remove(T v);
    bool validate();
    bool contains(T val);
    bool contains(std::size_t hash);
    char* getRootValue(){ return root.load(); };

private:
    void update(size_t hash, T &val);
    std::atomic<MerkleNode *> root;
};

template<typename T>
class MerkleTree<T>::MerkleNode {
public:
    T val;
    std::atomic<std::size_t> hash;

    MerkleNode(size_t remHash, T &v);
    MerkleNode();
    ~MerkleNode(){};
    std::atomic<MerkleNode *> left;
    std::atomic<MerkleNode *> right;
private:
};

#endif //MERKLETREE_H