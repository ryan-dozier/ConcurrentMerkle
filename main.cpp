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
#include "md5.h"
#include "sha256.h"

void work(int thread_id, int num_ops, MerkleTree<int*> *tree)
{
    int base = thread_id * num_ops;
    for (int i = 0; i < num_ops; i++) {
        int* nextItem = new int(base + i);
        tree->insert(nextItem);
    }
    
    for (int i = 0; i < num_ops; i++) {
        int* temp = new int(base + i);
        delete temp;
    }
}

int main(int argc, const char * argv[]) {
    // insert code here...
    auto* tree = new MerkleTree<int*>(sha256);
    
    std::vector<std::thread> threads;
    int NUM_OP = 10000;
    int NUM_THREADS = 8;
    std::cout << "Beginning Test" << std::endl;
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < NUM_THREADS; i++) {
        threads.push_back(std::thread(work, i, NUM_OP, tree));
    }
    for (std::thread &t : threads)
        t.join();
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = end-start;
    std::cout << "Threads Joined" << std::endl;
    std::cout << std::endl << "Execution Stats" << std::endl;

    std::chrono::duration<double> seconds = elapsed;
    auto throughput = 2 * (NUM_OP * NUM_THREADS) / seconds.count();
    std::cout << "Root->val = 0x" << tree->getRootValue() << std::endl;
    std::cout << "Throughput    :\t" << throughput << " ops/sec" << std::endl;
    std::cout << "Hash Validity :\t";
    if(tree->validate()) {
        std::cout << "Verified" << std::endl;
        
    } else {
        std::cout << "Invalid" << std::endl;
    }
    
    std::cout << "Data Validity :\t";
    bool containsAll = true;
    bool printed = false;
    for(int i = 0; i < NUM_OP * NUM_THREADS; i++) {
        int* temp = &i;
        if(!tree->contains(temp)) {
            if(!printed)
                std::cout << "Incorrect" << std::endl;
            printed = true;

            containsAll = false;
            std::cout << "Missing: " << *temp << std::endl;
        }
    }
    if(containsAll) {
        std::cout << "Correct" << std::endl;
    } else {
        tree->print_values();
    }

    delete tree;
    return 0;
}
