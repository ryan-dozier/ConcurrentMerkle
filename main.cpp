//
//  main.cpp
//  MerkleTree
//
//  Created by n00b on 1/7/20.
//  Copyright © 2020 n00b. All rights reserved.
//

#include <iostream>
#include "MerkleTree.h"

int main(int argc, const char * argv[]) {
    // insert code here...
    MerkleTree<int*>* tree = new MerkleTree<int*>();
    int value = 5;
    int* test = &value;
    tree->insert(test);
    int value2 = 6;
    int* test2 = &value2;
    tree->insert(test2);

    int value3 = 7;
    int* test3 = &value3;

    int value4 = 8;
    int* test4 = &value4;
    tree->insert(test4);

    int value5 = 9;
    int* test5 = &value5;
    tree->insert(test5);


    bool is5here = tree->contains(test);
    bool is6here = tree->contains(test2);
    bool is7here = tree->contains(test3);
    bool is8here = tree->contains(test4);
    bool is9here = tree->contains(test5);

    tree->remove(test5);
    bool is9here2 = tree->contains(test5);


    std::cout << is5here << std::endl;
    std::cout << is6here << std::endl;
    std::cout << is7here << std::endl;
    std::cout << is8here << std::endl;
    std::cout << is9here << std::endl;
    std::cout << is9here2 << std::endl;

    return 0;
}
