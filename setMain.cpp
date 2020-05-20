//
// Created by n00b on 5/14/20.
//
#include <iostream>
#include <thread>
#include <vector>
#include <time.h>
#include "MerkleTreeVerifier.h"
#include "md5.h"
#include "sha256.h"
#include "SequentialMerkle.h"
/*
int main(int argc, const char * argv[]) {
    auto* tree = new ConcurrentVerifier::MerkleTreeVerifier<int*>(2, sha256);
    int num_items = 10;
    for (int i = 0; i < num_items; i++) {
        int* nextItem = new int(i);
        tree->insert(nextItem);
    }
    
    for (int i = 0; i < num_items; i++) {
        if(!tree->contains(&i))
            std::cout << "missing: " << i << std::endl;
    }
    tree->validate();
    std::cout << "hello world " << tree->getRootValue() << std::endl;
    return 0;
}
*/


void parallel_work(int thread_id, int num_ops, ConcurrentVerifier::MerkleTreeVerifier<int*> *tree)
{
    int numInserts = num_ops * .2;
    int numContains = num_ops - numInserts;
    int base = thread_id * numInserts;
    for (int i = 0; i < numInserts; i++) {
        int* nextItem = new int(base + i);
        tree->insert(nextItem);
    }
    for (int i = 0; i < numContains; i++) {
        int* temp = new int(base + i);
        tree->contains(temp);
        delete temp;
    }
}

double parallel_benchmark(int num_index_bits, int NUM_OP, int NUM_THREADS) {
    auto* tree = new ConcurrentVerifier::MerkleTreeVerifier<int*>(num_index_bits, sha256);
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
    auto throughput = (NUM_OP * NUM_THREADS) / seconds.count();
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
    for(int i = 0; i < .2 *  NUM_OP * NUM_THREADS; i++) {
        if(!tree->contains(&i)) {
            if(!printed)
                //std::cout << "\tMissing:" << std::endl;
            printed = true;

            containsAll = false;
            //std::cout << "\t\t" << i << std::endl;
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
    int NUM_OP = 1000;
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
    std::cout << "\tOps per Thread  : " << NUM_OP << std::endl << std::endl;

    for(int i = 1; i < 5; i++) {
        int num_nodes = 1 << i;
        std::cout << "Child Nodes: " << num_nodes << std::endl;
        auto concurrent_throughput = parallel_benchmark(i, NUM_OP, NUM_THREADS);
        std::cout <<  std::endl;

    }
    
    std::cout << std::endl << "Benchmark Completed" << std::endl << "\t";
    return 0;
}
