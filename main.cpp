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
#include "SequentialMerkle.h"

void parallel_work(int thread_id, int num_ops, Concurrent::MerkleTree<int*> *tree)
{
    int base = thread_id * num_ops;
    for (int i = 0; i < num_ops; i++) {
        int* nextItem = new int(base + i);
        tree->insert(nextItem);
        tree->contains(nextItem);
        tree->contains(nextItem);
    }
    for(int j = 0; j < 5; j++) {
        for (int i = 0; i < num_ops; i++) {
            int* temp = new int(base + i);
            tree->contains(temp);
            delete temp;
        }
    }
}

double parallel_benchmark(int NUM_OP, int NUM_THREADS) {
    auto* tree = new Concurrent::MerkleTree<int*>(sha256);
    std::vector<std::thread> threads;
    std::cout << "Concurrent Benchmark" << std::endl;
    std::cout << "\tInitializing Threads" << std::endl;
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < NUM_THREADS; i++) {
        threads.push_back(std::thread(parallel_work, i, NUM_OP, tree));
    }
    for (std::thread &t : threads)
        t.join();
    
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = end-start;
    std::cout << "\tThreads Terminated" << std::endl;
    std::cout << "Execution Stats" << std::endl;

    std::chrono::duration<double> seconds = elapsed;
    auto throughput = 8 * (NUM_OP * NUM_THREADS) / seconds.count();
    std::cout << "\troot->val = 0x" << tree->getRootValue() << std::endl;
    std::cout << "\tThroughput    :\t" << throughput << " ops/sec" << std::endl;
    std::cout << "\tHash Validity :\t";
    if(tree->validate()) {
        std::cout << "Verified" << std::endl;
        
    } else {
        std::cout << "Invalid" << std::endl;
    }
    
    bool containsAll = true;
    bool printed = false;
    for(int i = 0; i < NUM_OP * NUM_THREADS; i++) {
        if(!tree->contains(&i)) {
            if(!printed)
                std::cout << "\tMissing:" << std::endl;
            printed = true;

            containsAll = false;
            std::cout << "\t\t" << i << std::endl;
        }
    }
    std::cout << "\tData Validity :\t";
    if(containsAll) {
        std::cout << "Correct" << std::endl;
    } else {
        std::cout << "Incorrect" << std::endl;
        //tree->print_values();
    }

    delete tree;
    return throughput;
}

void sequential_work(int thread_id, int num_ops, Sequential::MerkleTree<int*> *tree)
{
    int base = thread_id * num_ops;
    for (int i = 0; i < num_ops; i++) {
        int* nextItem = new int(base + i);
        tree->insert(nextItem);
        tree->contains(nextItem);
        tree->contains(nextItem);
    }
    
    for(int j = 0; j < 5; j++) {
        for (int i = 0; i < num_ops; i++) {
            int* temp = new int(base + i);
            tree->contains(temp);
            delete temp;
        }
    }
}

double sequential_benchmark(int NUM_OP, int NUM_THREADS) {
    auto* tree = new Sequential::MerkleTree<int*>(sha256);
    
    std::vector<std::thread> threads;
    std::cout << std::endl << "Coarse Grained Benchmark" << std::endl;
    std::cout << "\tInitializing Threads" << std::endl;

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < NUM_THREADS; i++) {
        threads.push_back(std::thread(sequential_work, i, NUM_OP, tree));
    }
    for (std::thread &t : threads)
        t.join();
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = end-start;
    std::cout << "\tThreads Terminated" << std::endl;
    std::cout << "Execution Stats" << std::endl;

    std::chrono::duration<double> seconds = elapsed;
    auto throughput = 8 * (NUM_OP * NUM_THREADS) / seconds.count();
    std::cout << "\troot->val = 0x" << tree->getRootValue() << std::endl;
    std::cout << "\tThroughput    :\t" << throughput << " ops/sec" << std::endl;
    std::cout << "\tHash Validity :\t";
    if(tree->validate()) {
        std::cout << "Verified" << std::endl;
        
    } else {
        std::cout << "Invalid" << std::endl;
    }
    
    bool containsAll = true;
    for(int i = 0; i < NUM_OP * NUM_THREADS; i++) {
        if(!tree->contains(&i)) {
            containsAll = false;
            std::cout << "Missing: " << i << std::endl;
        }
    }
    std::cout << "\tData Validity :\t";
    if(containsAll) {
        std::cout << "Correct" << std::endl;
    } else {
        std::cout << "Incorrect" << std::endl;
        // Debugging Purposes
        // tree->print_values();
    }

    delete tree;
    return throughput;
}

int main(int argc, const char * argv[]) {
    int NUM_OP = 10000;
    int NUM_THREADS = 4;
    
    if(argc == 3) {
        NUM_OP = atoi(argv[1]);
        NUM_THREADS = atoi(argv[2]);
    } else {
        std::cout << "Using Default Enviornment." << std::endl;
        std::cout << "To define user parameters use .\\<program> <num ops> <thread count>" << std::endl << std::endl;
    }
    
    std::cout << "Starting Benchmarks" << std::endl;
    std::cout << "\tThread Count\t: " << NUM_THREADS << std::endl;
    std::cout << "\tOps per Thread\t: " << NUM_OP << std::endl << std::endl;

    
    auto concurrent_throughput = parallel_benchmark(NUM_OP, NUM_THREADS);
    auto sequential_throughput = sequential_benchmark(NUM_OP, NUM_THREADS);
    
    std::cout << std::endl << "Concurrent vs Sequential" << std::endl;
    
    double percent_diff = ((concurrent_throughput - sequential_throughput) / sequential_throughput * 100);
    std::cout << "\tConcurrent ran\t: " << percent_diff  << "% faster." << std::endl;
    if(percent_diff == 0)
        std::cout << "\tSomehow they had equal throughput\t: ¯\\_(ツ)_/¯" << std::endl;
    std::cout << std::endl << "Benchmark Completed" << std::endl << "\t";
    return 0;
}
