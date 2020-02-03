//
// Created by Ryan Dozier on 1/6/20.
//

#ifndef MERKLETREE_H
#define MERKLETREE_H

#include <iostream>
#include <atomic>
#include <stack>
#include <string>

namespace Concurrent {

// The tree has two child nodes, left and right.
enum Direction { LEFT, RIGHT };
// Merkle trees contain two types of nodes, HASH, and DATA
enum NodeType { HASH, DATA };

/**
 * Class MerkleTree
 *
 */
template<typename T>
class MerkleTree {
    
protected:
    class MerkleNode;
    class Descriptor;
    
public:
    
    //TODO: There may be a better wayt to declare this sentinal node value
    inline static MerkleNode* nullNode = nullptr;
    
    /**
     * Constructor to create a merkle tree object. The required parameter is a hashing function
     * which takes an std::string and returns the hash as an std::string
     */
    MerkleTree(std::string (*hash_func)(std::string)) {
        this->hashFunc = hash_func;
        this->root.store(new MerkleNode());
    };

    ~MerkleTree() {
        // call recursive helper function
        this->post_delete(root.load());
        delete nullNode;
    };
    
    // Inserts a value into the tree
    void insert(T &v) {
        std::string* hash = new std::string(hashFunc(std::to_string(*v)));
        size_t key = gen_key(*hash);
        this->update(hash, key, v);
    };
    
    // Removes a value into the tree
    void remove(T v) {
        std::string* hash = new std::string(hashFunc(std::to_string(v)));
        size_t key = gen_key(*hash);
        // TODO: this is probably wrong, will need to debug later
        T temp = NULL;
        this->update("", key, temp);
    };
    
    // TODO: This function will require a lot of thought as it will likely have to be blocking. I want to have
    //          insert/remove/contains flushed out first. My current idea is to have validate block access to the tree
    //          during the operation, but to allow other threads to add pending operations to a queue. The contains
    //          operation will be able to look at the pending operations and the tree to determine if the item is
    //          present. There is also a possiblility of method crossing if a remove and insert are called during the
    //          validate operation.
    bool validate();
    
    // checks if a value is in the tree.
    bool contains(T val) {
        std::string hash = hashFunc(std::to_string(*val));
        size_t key = gen_key(hash);
        return this->contains(hash, key);
    };
    
    // returns the value of the root hash
    std::string getRootValue() { return *(root.load()->hash.load()); };
    
    // prints all DATA Nodes in postorder traversal. (Mainly for debugging)
    void print_values() { print_values(this->root.load()); }
    

private:
    // root node
    std::atomic<MerkleNode*> root;
    
    // hash function
    std::string (*hashFunc)(std::string);
    
    // hashing function to generate node keys
    std::hash<std::string> gen_key;
    
    // The update function performs both inserts and removes depending on the parameters.
    void update(std::string* hash, size_t key, T &val);
    
    // finishOp allows other executing threads to help finish the operation
    void finishOp(Descriptor* job);
    
    // underlying contains operation, it generates the hash / key we are looking for
    bool contains(std::string hash, std::size_t key);
    
    /**
    * This function does a postorder traversal of the tree and deallocates the used memory.
    * It is NOT thread safe.
    */
    void post_delete(MerkleNode* node) {
        if (node != nullNode) {
            post_delete(node->left.load());
            post_delete(node->right.load());
            delete node->hash.load();
            if(node->type == DATA)
                delete node->val;
            delete node;
        }
    };
    
    /**
     * Performs a postorder traversal and outputs all DATA nodes to the console.
     * (primarily for debugging purposes)
     */
    void print_values(MerkleNode* node) {
        if (node != nullNode) {
            print_values(node->left.load());
            print_values(node->right.load());
            if(node->type == DATA) {
                std::cout << *(node->val) << std::endl;
            }
        }
    };
};

/**
* Class MerkleNode
*
*/
template<typename T>
class MerkleTree<T>::MerkleNode {
public:
    
    T val;
    std::size_t key;
    NodeType type;
    std::atomic<std::string*> hash;
    std::atomic<Descriptor*> desc;
    std::atomic<MerkleNode*> left;
    std::atomic<MerkleNode*> right;
    
    MerkleNode(std::string* _hash, T &v) {
        this->val = v;
        this->hash.store(_hash);
        this->type = DATA;
        this->desc.store(nullptr);
        this->left.store(nullptr);
        this->right.store(nullptr);
    };
    
    MerkleNode(std::string* _hash, size_t key, T &v) {
        this->val = v;
        this->hash.store(_hash);
        this->key = key;
        this->type = DATA;
        this->desc.store(nullptr);
        this->left.store(nullptr);
        this->right.store(nullptr);
    };
    
    MerkleNode() {
        this->hash.store(new std::string(""));
        this->val = NULL;
        this->key = 0;
        this->type = HASH;
        this->desc.store(nullptr);
        this->left.store(nullptr);
        this->right.store(nullptr);
    };
    
    ~MerkleNode() {
        // TODO: this is causing crashes, need to look at why
        //delete this->hash.load();
        delete this->desc.load();
    };
    
    void resetChildNodes() {
        this->left.store(nullptr);
        this->right.store(nullptr);
    }
};

/**
* Class Descriptor
*
*/
template<typename T>
class MerkleTree<T>::Descriptor
{
public:
    bool pending;
    NodeType typeOp;
    MerkleNode* parent;
    MerkleNode* oldChild;
    MerkleNode* child;
    Direction dir;
    size_t key;
    
    
    Descriptor() {
        this->pending = true;
        this->typeOp = HASH;
    };
    
    Descriptor(MerkleNode* _parent, MerkleNode* _child, MerkleNode* _oldChild, Direction _dir, size_t _key) {
        this->pending = true;
        this->typeOp = HASH;
        this->parent = _parent;
        this->child = _child;
        this->oldChild = _oldChild;
        this->dir = _dir;
        this->key = _key;
    };
    
    Descriptor(MerkleNode* _child) {
        this->pending = true;
        this->typeOp = DATA;
        this->child = _child;
    };
    
    ~Descriptor() {};
    
    void setDataDescriptor(MerkleNode* _parent, Direction _dir) {
        this->parent = _parent;
        this->dir = _dir;
    };
    
    void setHashDescriptor(MerkleNode* _parent, MerkleNode* _child, MerkleNode* _oldChild, Direction _dir, size_t _key) {
        this->parent = _parent;
        this->child = _child;
        this->oldChild = _oldChild;
        this->dir = _dir;
        this->key = _key;
    };
private:
};

template<typename T>
void MerkleTree<T>::update(std::string* hash, std::size_t key, T &val) {
    // walks through the tree
    MerkleNode* walker = this->root.load();
    MerkleNode* next = nullptr;
    Descriptor* currentDesc;
    std::stack<MerkleTree<T>::MerkleNode*> visited;

    // Allocate the Data node to insert
    MerkleNode* dataNode = new MerkleNode(hash, val);
    Descriptor* dataDesc = new Descriptor(dataNode);
    
    // These are used if an intermediary node is needed
    MerkleNode* hashNode = nullptr;
    Descriptor* hashDesc = nullptr;
    
    Direction dir;
    bool finished = false;
    
    while(!finished) {
        // After insertion we will need to update the hashes along the path. Nodes on the path are stored in the stack
        visited.push(walker);
        
        // Grab the current descriptor
        currentDesc = walker->desc.load();
        // Finish its pending operation
        finishOp(currentDesc);
        
        
        // Determine which direction we need to traverse and load next based on the direction
        switch(key % 2) {
            case LEFT :
                next = walker->left.load();
                dir = LEFT;
                break;
            case RIGHT :
                next = walker->right.load();
                dir = RIGHT;
                break;
        }
        
        // In this function we stop if the next type is DATA or nullptr.
        // Therefore walker must be a HASH node, we can try to insert a DATA node at next
        if(next == nullptr) {
            // set the value of the new node's key
            dataNode->key = key >> 1;
            dataDesc->setDataDescriptor(walker, dir);
            if(walker->desc.compare_exchange_weak(currentDesc, dataDesc)) {
                // TODO: is there a chance dataDesc could be removed by here?
                finishOp(dataDesc);
                finished = true;
                delete currentDesc;
            }
        } else if (next->type == DATA) {
            /**
             * There are two cases when reaching a DATA node,
             * 1) We are updating an existing value
             * 2) An intermediary node is needed between the current insert and the node stored in next.
             */
            
            // check if the data node is what we are inserting
            // TODO: this is not entirely comprehensive as the hashes could be equal in the case of a remove operation. BUT string compare is slow, so will have to check equality without an expensive operation. Perhaps key == key, then if that is true compare the full hashes. if key == key we have a hash collision though which will cause problems.
            if(*next->val == *val) {
                // TODO: this is where we will end up performing the removal operation as well. ATM just insert works.
                return;
            } else {
                /**
                 * Next is currently at a differing DATA node along the path the inserted node
                 * needs to be apended. An intermediary hash node needs to be created and placed
                 * between walker and next.
                 */
                
                // Check if we have an already allocated node
                if(hashNode == nullptr) {
                    // Create a new node and descriptor
                    hashNode = new MerkleNode();
                    hashDesc = new Descriptor(walker, hashNode, next, dir, next->key >> 1);
                }
                else {
                    // Clean the allocated node and reinitialize the descriptor
                    hashNode->resetChildNodes();
                    hashDesc->setHashDescriptor(walker, hashNode, next, dir, next->key >> 1);
                }
                
                // Check which direction next should be apended to the intermediary node
                switch(next->key % 2) {
                    case LEFT :
                        hashNode->left.store(next);
                        break;
                    case RIGHT :
                        hashNode->right.store(next);
                        break;
                }
                
                // Try to swap the new descriptor
                if(walker->desc.compare_exchange_weak(currentDesc, hashDesc)) {
                    // finish the operation
                    finishOp(hashDesc);
                    
                    // Get ready for the next loop iteration
                    key >>= 1;
                    walker = hashNode;
                    
                    // now that the old descriptor has been swapped, de-allocate it
                    delete currentDesc;
                    
                    // Mark local nodes as used
                    hashNode = nullptr;
                    hashDesc = nullptr;
                }
            }
        } else {
            // Still have HASH nodes to traverse, shift the key, set walker to next.
            key >>= 1;
            walker = next;
        }
    } // end while
    
    // If we have unused intermediary nodes, free them from memory
    if(hashNode != nullptr) {
        delete hashNode;
        delete hashDesc;
    }

    // This section performs the hashing operations on the visited nodes simulating a recursive call stack.
    while(!visited.empty()) {
        // Grab the top node from the stack
        walker = visited.top();
        MerkleNode* left;
        MerkleNode* right;
        visited.pop();
        std::string* oldHash;
        std::string* newVal = new std::string();
        do {
            // TODO: there may be a more efficient way to do this string arithmetic
            *newVal = "";
            // grab a temporary copy of the hash, used to detect state changes
            oldHash = walker->hash.load();
            left = walker->left.load();
            right = walker->right.load();
            
            // obtain the hashes from the child nodes
            if(left != nullptr)
                *newVal += *(left->hash.load());
    
            if(right != nullptr)
                *newVal += *(right->hash.load());
            
            // compute the new hashes
            *newVal = hashFunc(*newVal);
            // Attempt to compare and swap the newly computed hash, if it fails another thread has
            // updated the hash. Need to reload the values and recompute the hashes for the next iteration.
        } while(!walker->hash.compare_exchange_weak(oldHash, newVal));
        
        // TODO: this is a memory leak, but need to think about how to solve it. The issue is oldhash could be referenced by other threads as they work.
        // TODO: Thought 1 : maybe collect discareded old strings to remove later on qqueue?
        //delete oldHash;
    }
}

template<typename T>
void MerkleTree<T>::finishOp(Descriptor* job) {
    if(job != nullptr) {
        std::atomic<MerkleNode*>* update_node;
        if(job->pending) {
            switch(job->dir) {
                case LEFT :
                    update_node = &(job->parent->left);
                    break;
                case RIGHT :
                    update_node = &(job->parent->right);
                    break;
            }
            
            switch(job->typeOp) {
                case HASH :
                    if (update_node->compare_exchange_weak(job->oldChild, job->child))
                        job->oldChild->key = job->key;
                    break;
                case DATA :
                    update_node->compare_exchange_weak(nullNode, job->child);
                    break;
            }
            job->pending = false;
        }
    }
}

template<typename T>
bool MerkleTree<T>::contains(std::string hash, std::size_t key) {
    bool result = false;
    MerkleNode* walker = this->root.load();
    while (walker != nullptr) {
        
        // Arrived at a leaf node.
        if (walker->type == DATA) {
            // check if the leaf contains the correct value
            if (walker->hash.load()->compare(hash) == 0)
                result = true;
            // break from the loop, if the hash was not equal the value is not in the tree as we are at the bottom
            break;
        }
        
        // If there are further parent nodes determine which direction to continue the search and set the walker to the next node to search.
        switch (key % 2) {
            case LEFT :
                walker = walker->left.load();
                break;
            case RIGHT :
                walker = walker->right.load();
                break;
        }
        // decrement the key and continue.
        key >>= 1;
    }
    return result;
}

// TODO: This will not currently work with concurrent execution. However it does work sequentially for testing at the moment.
template<typename T>
bool MerkleTree<T>::validate() {
    bool result = true;
    
    // create an empty stack and push root node
    std::stack<MerkleNode*> stk;
    stk.push(this->root.load());
    
    // create another stack to store post-order traversal
    std::stack<MerkleNode*> order;
    
    // run till stack is not empty
    while (!stk.empty()) {
        // we pop a node from the stack and push the data to output stack
        MerkleNode* curr = stk.top();
        stk.pop();
        
        if(curr->val == NULL)
            order.push(curr);
        
        // push left and right child of popped node to the stack
        if (curr->left.load() != nullNode)
            stk.push(curr->left.load());
        
        if (curr->right.load() != nullNode)
            stk.push(curr->right.load());
    }
    
    MerkleNode *walker, *left, *right;
    std::string computed_hash;
    
    //TODO: This could probably be optimized some.
    while (!order.empty()) {
        computed_hash = "";
        walker = order.top();
        left = walker->left.load();
        right = walker->right.load();
        
        if(left != nullNode)
            computed_hash += *(left->hash.load());
        if(right != nullNode)
            computed_hash += *(right->hash.load());
        
        computed_hash = hashFunc(computed_hash);
        if(computed_hash.compare(*walker->hash.load()) != 0)
            result = false;
        
        order.pop();
    }
    return result;
}

} // end Concurrent Namespace
#endif //MERKLETREE_H
