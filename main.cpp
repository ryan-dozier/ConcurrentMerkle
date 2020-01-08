//
//  main.cpp
//  MerkleTree
//
//  Created by n00b on 1/7/20.
//  Copyright © 2020 n00b. All rights reserved.
//

#include <iostream>
#include <thread>
#include <vector>
#include "MerkleTree.h"


void work(int thread_id, int num_ops, MerkleTree<int*> *tree)
{
    int base = thread_id * num_ops;
    for (int i = 0; i < num_ops; i++)
    {
        int* nextItem = new int(base + i);
        tree->insert(nextItem);
    }
}

int main(int argc, const char * argv[]) {
    // insert code here...
    auto* tree = new MerkleTree<int*>();
    std::vector<std::thread> threads;

    int NUM_THREADS = 4;
    for (int i = 0; i < NUM_THREADS; i++)
    {
        threads.push_back(std::thread(work, i, 100, tree));
    }
    
    for (std::thread &t : threads)
        t.join();
    
    
    std::cout << tree->validate() << std::endl;
    delete tree;
    return 0;
}


