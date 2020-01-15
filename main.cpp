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
#include <time.h>
#include "MerkleTree.h"


void work(int thread_id, int num_ops, MerkleTree<int*> *tree)
{
    int base = thread_id * num_ops;
    for (int i = 0; i < num_ops; i++)
    {
        int* nextItem = new int(base + i);
        tree->insert(nextItem, thread_id);
    }
}

int main(int argc, const char * argv[]) {
    // insert code here...
    auto* tree = new MerkleTree<int*>();
    std::vector<std::thread> threads;
    int NUM_OP = 10000;
    int NUM_THREADS = 1;
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < NUM_THREADS; i++) {
        threads.push_back(std::thread(work, i, NUM_OP, tree));
    }
    
    for (std::thread &t : threads)
        t.join();
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = end-start;
    std::chrono::duration<double> seconds = elapsed;
    auto throughput = (NUM_OP * NUM_THREADS) / seconds.count();
    std::cout << throughput << std::endl;
    std::cout << tree->validate() << std::endl;
    //delete tree;
    return 0;
}
