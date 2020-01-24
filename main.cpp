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
#include "bigInt.h"

#define SHOW_BIG_INT(x) std::cout << #x << " : Hex Value = " << x.toStr0xHex() << ", Hex Digits = " << x.getActualHexDigits() << "; Dec Value = " << x.toStrDec() << ", Dec Digits = " << x.getActualDecDigits() << std::endl << std::endl
#define SHOW_DOUBLE(x) std::cout << #x << " : " << std::fixed << x << std::endl << std::endl

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
    
    BigInt::Rossi n2 ("224f3e07282886cce82404b6f8", BigInt::HEX_DIGIT);
    SHOW_BIG_INT(n2);
    
    std::vector<std::thread> threads;
    int NUM_OP = 10000;
    int NUM_THREADS = 4;
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
    if(tree->validate()) {
        std::cout << "Tree Verified" << std::endl;
    } else {
        std::cout << "Invalid Tree" << std::endl;
    }
    
    bool containsAll = true;
    bool printed = false;
    for(int i = 0; i < NUM_OP * NUM_THREADS; i++) {
        int* temp = &i;
        if(!tree->contains(temp)) {
            if(!printed)
                std::cout << "Tree Lost Data" << std::endl;
            printed = true;

            containsAll = false;
            std::cout << "Missing: " << *temp << std::endl;
        }
    }
    if(containsAll) {
        std::cout << "Tree Contains All Inserts" << std::endl;
    } else {
        tree->print_values();
    }

    delete tree;
    return 0;
}
