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
public:
    class MerkleNode;
    class Descriptor;
    
    //TODO: There may be a better wayt to declare this sentinal node value
    inline static MerkleNode* nullNode = nullptr;
    
    /**
     * Constructor to create a merkle tree object. The required parameter is a hashing function which takes an std::string
     * and returns the hash as an std::string
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
        this->desc.store(new Descriptor());
        this->left.store(nullptr);
        this->right.store(nullptr);
    };
    
    MerkleNode(std::string* _hash, size_t key, T &v) {
        this->val = v;
        this->hash.store(_hash);
        this->key = key;
        this->type = DATA;
        this->desc.store(new Descriptor());
        this->left.store(nullptr);
        this->right.store(nullptr);
    };
    
    MerkleNode() {
        this->hash.store(new std::string(""));
        this->val = NULL;
        this->key = 0;
        this->type = HASH;
        this->desc.store(new Descriptor());
        this->left.store(nullptr);
        this->right.store(nullptr);
    };
    
    ~MerkleNode() {
        // TODO: this is causing crashes, need to look at why
        //delete this->hash.load();
        delete this->desc.load();
    };
    
private:
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
    
    Descriptor(MerkleNode* _parent, MerkleNode* _child, MerkleNode* _oldChild, Direction _dir, size_t _key) {
        this->pending = true;
        this->typeOp = HASH;
        this->parent = _parent;
        this->child = _child;
        this->oldChild = _oldChild;
        this->dir = _dir;
        this->key = _key;
    };
    
    Descriptor(MerkleNode* _parent, MerkleNode* _child, Direction _dir) {
        this->pending = true;
        this->typeOp = DATA;
        this->parent = _parent;
        this->child = _child;
        this->dir = _dir;
    };
    
    Descriptor() {
        this->pending = false;
    };
    
    ~Descriptor() {};
    
private:
};

template<typename T>
void MerkleTree<T>::update(std::string* hash, std::size_t key, T &val) {
    MerkleNode* walker = this->root.load();
    MerkleNode* next = nullptr;
    MerkleNode* dataNode = new MerkleNode(hash, val);
    MerkleNode* hashNode = nullptr;
    Descriptor* currentDesc;
    Descriptor* newDesc;
    std::stack<MerkleTree<T>::MerkleNode*> visited;
    Direction dir;
    bool finished = false;
    
    while(!finished) {
        visited.push(walker);
        
        currentDesc = walker->desc.load();
        finishOp(currentDesc);
        
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
        
        if(next == nullptr) {
            // TODO: My idea here is to instead of generate new descriptors and new nodes to have update functions to change the needed values instead of allocating each time. We know that every node will need a descriptor and a node to insert. Thereore we should only allocate once as the nodes are not shared until they exist in the tree (ABA problem immunity).
            // TODO: For Descriptor I do need a way to change from DATA and HASH, two different update functions should solve this, or one that makes it more explicit. For descriptors we may be able to only allocate when we need one, have something like if(newDesc == nullptr) newDesc == new .... this would allow for reusing some nodes saving on some data allocation.
            
            // set the value of the new node's key
            dataNode->key = key >> 1;
            newDesc = new Descriptor(walker, dataNode, dir);
            if(walker->desc.compare_exchange_weak(currentDesc, newDesc)) {
                finishOp(newDesc);
                finished = true;
                delete currentDesc;
            } else {
                delete newDesc;
            }
        }
        else if (next->type == DATA) {
            // check if the data node is what we are inserting
            if(*next->val == *val) {
                finished = true;
            } else {
                hashNode = new MerkleNode();
                switch(next->key % 2) {
                    case LEFT :
                        hashNode->left.store(next);
                        break;
                    case RIGHT :
                        hashNode->right.store(next);
                        break;
                }
                
                newDesc = new Descriptor(walker, hashNode, next, dir, next->key >> 1);
                if(walker->desc.compare_exchange_weak(currentDesc, newDesc)) {
                    finishOp(newDesc);
                    key >>= 1;
                    walker = hashNode;
                    delete currentDesc;
                } else {
                    delete hashNode;
                    delete newDesc;
                }
            }
        } else {
            key >>= 1;
            walker = next;
        }
    } // end while
    
    // This section performs the hashing operations on the visited nodes simulating a recursive call stack.
    while(!visited.empty())
    {
        
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
        // TODO: Thought 1 : maybe collect discareded old strings to remove later on,
        //delete oldHash;
    }
}

template<typename T>
void MerkleTree<T>::finishOp(Descriptor* job) {
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
