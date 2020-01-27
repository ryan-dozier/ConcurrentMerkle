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
    }
    
    for (int i = 0; i < num_ops; i++) {
        int* temp = new int(base + i);
        tree->contains(temp);
        delete temp;
    }
}

void sequential_work(int thread_id, int num_ops, Sequential::MerkleTree<int> *tree)
{
    int base = thread_id * num_ops;
    for (int i = 0; i < num_ops; i++) {
        tree->insert(int(base + i));
    }
    
    for (int i = 0; i < num_ops; i++) {
        tree->contains(int(base + i));
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
    auto throughput = 2 * (NUM_OP * NUM_THREADS) / seconds.count();
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
        int* temp = &i;
        if(!tree->contains(temp)) {
            if(!printed)
                std::cout << "\tMissing:" << std::endl;
            printed = true;

            containsAll = false;
            std::cout << "\t\t" << *temp << std::endl;
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


double sequential_benchmark(int NUM_OP, int NUM_THREADS) {
    auto* tree = new Sequential::MerkleTree<int>(sha256);
    
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
    auto throughput = 2 * (NUM_OP * NUM_THREADS) / seconds.count();
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
        if(!tree->contains(i)) {
            containsAll = false;
            std::cout << "Missing: " << i << std::endl;
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

int main(int argc, const char * argv[]) {
    // insert code here...
    int NUM_OP = 10000;
    int NUM_THREADS = 8;
    
    std::cout << "Starting Benchmarks" << std::endl;
    std::cout << "\tThread Count\t: " << NUM_THREADS << std::endl;
    std::cout << "\tOps per Thread\t: " << NUM_OP << std::endl << std::endl;

    
    auto concurrent_throughput = parallel_benchmark(NUM_OP, NUM_THREADS);
    auto sequential_throughput = sequential_benchmark(NUM_OP, NUM_THREADS);
    
    std::cout << std::endl << "Concurrent vs Sequential" << std::endl;
    double percent_diff = (concurrent_throughput / sequential_throughput * 100);
    if(concurrent_throughput > sequential_throughput)
        std::cout << "\tConcurrent ran\t: " << percent_diff  << "% faster." << std::endl;
    else if(concurrent_throughput < sequential_throughput)
        std::cout << "\tConcurrent ran\t: " << (100 - percent_diff) << "% slower." << std::endl;
    else
        std::cout << "\tSomehow they had equal throughput\t: ¯\\_(ツ)_/¯" << std::endl;
    std::cout << std::endl << "Benchmark Completed" << std::endl << "\t";
    return 0;
}
