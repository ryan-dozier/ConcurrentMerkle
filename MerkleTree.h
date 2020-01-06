//
// Created by Ryan Dozier on 1/6/20.
//

#ifndef MERKLETREE_H
#define MERKLETREE_H

#include <atomic>
#include <math.h>
#include <string>
#include <iostream>

#define LEFT 0
#define RIGHT 1

template<typename T>
class MerkleTree {
public:
    class MerkleNode;
    MerkleTree(char* (*const hashFunc)(T val));
    ~MerkleTree();
    void insert(T &v);
    bool validate();
    bool contains(T val);
    bool contains(std::size_t hash);
    bool delete_hash(std::size_t hash);
    char* getRootValue(){ return root.load(); };

    char* (*const hash_function)(T val);
private:
    std::atomic<MerkleNode*> root;
};

template<typename T>
class MerkleTree<T>::MerkleNode {
public:
    //bool contains(std::size_t hash);
    //bool insert(std::size_t key, T &v);
    //virtual bool delete_hash(std::string hash);
    std::atomic<MerkleNode *> left { nullptr };
    std::atomic<MerkleNode *> right { nullptr };
private:

};



#endif //MERKLETREE_H
