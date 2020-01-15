//
// Created by Ryan Dozier on 1/6/20.
//

#ifndef MERKLETREE_H
#define MERKLETREE_H


// Check GCC
#if __GNUC__
#if __x86_64__ || __ppc64__
#define ENVIRONMENT64
#define MAXBITS 64
#else
#define ENVIRONMENT32
#define MAXBITS 32
#endif
#endif

#include <iostream>
#include <atomic>
#include <stack>

enum Direction { LEFT, RIGHT };
enum NodeType { HASH, DATA };

template<typename T>
class MerkleTree {
public:
    class MerkleNode;
    class Descriptor;

    //TODO: There may be a better wayt to declare this sentinal node value
    inline static MerkleNode* nullNode = nullptr;
    
    //TODO: I would prefer to utilize different a different hashing function, but this will work for the moment.
    inline static std::hash<T> hash_data;
    inline static std::hash<std::string> hash_hashes;
    
    MerkleTree() {
        this->root.store(new MerkleNode());
    };
    // TODO: This needs a full traversal to delete all nodes, will work on this later
    ~MerkleTree() {
        this->post_delete(root.load());
        delete nullNode;
    };
    
    // Inserts a value into the tree
    void insert(T &v, int tid) {
        size_t hash = hash_data(v);
        this->update(hash, v, tid);
    };
    
    // Removes a value into the tree
    void remove(T v, int tid) {
        size_t hash = hash_data(v);
        T temp = NULL;
        this->update(hash, temp, tid);
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
        return this->contains(hash_data(val));
    };
    
    // checks if a given hash is in the tree
    bool contains(std::size_t hash);
    
    // returns the root hash value
    size_t getRootValue() { return root.load()->hash.load(); };
    
private:
    std::atomic<MerkleNode*> root;
    // The update function performs both inserts and removes depending on the parameters.
    void update(size_t hash, T &val, int tid);
    void finishOp(Descriptor* job);
    
    // helper deconstructor function.
    void post_delete(MerkleNode* node) {
        if (node != nullNode) {
            post_delete(node->left.load());
            post_delete(node->right.load());
            delete node;
        }
    }
};

template<typename T>
class MerkleTree<T>::MerkleNode {
public:

    T val;
    std::size_t key;
    NodeType type;
    //TODO: This likely should be changed to be a string, but c++ doesnt have atomic string support. will have to think about this.
    std::atomic<std::size_t> hash;
    std::atomic<Descriptor*> desc;
    std::atomic<MerkleNode*> left;
    std::atomic<MerkleNode*> right;
    
    MerkleNode(size_t _hash, size_t key, T &v) {
        this->val = v;
        this->hash.store(_hash);
        this->key = key;
        this->type = DATA;
        this->desc.store(new Descriptor());
        this->left.store(nullptr);
        this->right.store(nullptr);
    };
    
    MerkleNode() {
        this->hash.store(0);
        this->val = NULL;
        this->key = 0;
        this->type = HASH;
        this->desc.store(new Descriptor());
        this->left.store(nullptr);
        this->right.store(nullptr);
    };
    
    ~MerkleNode() {};
    
private:
};

template<typename T>
class MerkleTree<T>::Descriptor {
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
    }
    
    Descriptor(MerkleNode* _parent, MerkleNode* _child, Direction _dir) {
        this->pending = true;
        this->typeOp = DATA;
        this->parent = _parent;
        this->child = _child;
        this->dir = _dir;
    }
    
    Descriptor() {
        this->pending = false;
    }
    
    ~Descriptor() {};
    
private:
};

template<typename T>
void MerkleTree<T>::update(std::size_t hash, T &val, int tid) {
    MerkleNode* walker = this->root.load();
    short dir;
    MerkleNode* next = nullptr;
    MerkleNode* newNode;
    Descriptor* newDesc;
    std::stack<MerkleTree<T>::MerkleNode*> visited;
    Descriptor* currentDesc;
    
    std::size_t key = hash;
    bool finished = false;
    
    while(!finished) {
        visited.push(walker);
        
        currentDesc = walker->desc.load();
        finishOp(currentDesc);
            
        dir = key % 2;
        switch(dir) {
            case LEFT :
                next = walker->left.load();
                dir = LEFT;
                break;
            case RIGHT :
                next = walker->right.load();
                dir = RIGHT;
                break;
        }
        key >>= 1;

        if(next == nullptr) {
            newNode = new MerkleNode(hash, key, val);
            switch(dir) {
                case LEFT :
                    do {
                        newDesc = new Descriptor(walker, newNode, LEFT);
                        if(walker->desc.compare_exchange_weak(currentDesc, newDesc)) {
                            finished = true;
                        }
                        finishOp(newDesc);
                        finished = true;
                    } while (!finished || walker->left.load() == nullptr);
                    break;
                case RIGHT :
                    do {
                        newDesc = new Descriptor(walker, newNode, RIGHT);
                        if(walker->desc.compare_exchange_weak(currentDesc, newDesc)) {
                            finished = true;
                        }
                        finishOp(newDesc);
                        finished = true;
                    } while (!finished || walker->right.load() == nullptr);
                    break;
            }
        } else if (next->type == DATA) {
            // check if the data node is what we are inserting
            if(next->hash.load() == hash && next->val == val) {
                finished = true;
            } else {
                newNode = new MerkleNode();
                switch(next->key % 2) {
                    case LEFT :
                        newNode->left.store(next);
                        break;
                    case RIGHT :
                        newNode->right.store(next);
                        break;
                }
                
                switch(dir) {
                    case LEFT :
                        newDesc = new Descriptor(walker, newNode, next, LEFT, next->key >> 1);
                        if(walker->desc.compare_exchange_weak(currentDesc, newDesc)) {
                            finishOp(newDesc);
                            walker = newNode;
                        } else {
                            walker = walker->left.load();
                        }
                        break;
                    case RIGHT :
                        newDesc = new Descriptor(walker, newNode, next, RIGHT, next->key >> 1);
                        if(walker->desc.compare_exchange_weak(currentDesc, newDesc)) {
                            finishOp(newDesc);
                            walker = newNode;
                        } else {
                            walker = walker->right.load();
                        }
                        break;
                }
            }
        } else {
            walker = next;
            break;
        }
    } // end while
    
    // This section performs the hashing operations on the visited nodes simulating a recursive call stack.
    while(!visited.empty()) {
        
        // Grab the top node from the stack
        walker = visited.top();
        visited.pop();
        std::size_t temp;
        std::size_t newVal;
        std::string concatHash;
        do {
            //TODO: there may be a more efficient way to do this string arithmetic
            concatHash = "";
            // grab a temporary copy of the hash, used to detect state changes
            temp = walker->hash;
            
            // obtain the hashes from the child nodes
            if(walker->left.load() != nullptr)
                concatHash += std::to_string(walker->left.load()->hash.load());
            if(walker->right.load() != nullptr)
                concatHash += std::to_string(walker->right.load()->hash.load());
            
            // compute the new hashes
            newVal = hash_hashes(concatHash);
            // Attempt to compare and swap the newly computed hash, if it fails another thread has updated the hash. Need to reload the
            // values and recompute the hashes for the next iteration.
        } while(!walker->hash.compare_exchange_weak(temp, newVal));
    }
}

template<typename T>
void MerkleTree<T>::finishOp(Descriptor* job) {
    if(job->pending) {
        switch(job->typeOp) {
            case HASH :
                switch(job->dir) {
                    case LEFT :
                        if(job->parent->left.compare_exchange_weak(job->oldChild, job->child)) {
                            job->oldChild->key = job->key;
                        }
                        break;
                    case RIGHT :
                        if(job->parent->left.compare_exchange_weak(job->oldChild, job->child)) {
                            job->oldChild->key = job->key;
                        }
                        break;
                }
                
                break;
            case DATA :
                switch(job->dir) {
                    case LEFT :
                        job->parent->left.compare_exchange_weak(nullNode, job->child);
                        break;
                    case RIGHT :
                        job->parent->right.compare_exchange_weak(nullNode, job->child);
                        break;
                }
                break;
        }
        job->pending = false;
    }
}



template<typename T>
bool MerkleTree<T>::contains(std::size_t hash) {
    bool result = false;
    size_t key = hash;
    MerkleNode* walker = this->root.load();
    for (int i = 0; i < MAXBITS && walker != nullNode; i++) {
        
        // Arrived at a leaf node.
        if (walker->val.load() != NULL) {
            // check if the leaf contains the correct value
            if (walker->hash.load() == hash)
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
    size_t computed_hash;
    std::string concatHash;
    
    //TODO: This could probably be optimized some.
    while (!order.empty()) {
        concatHash = "";
        walker = order.top();
        left = walker->left.load();
        right = walker->right.load();
        
        if(left != nullNode)
            concatHash += std::to_string(left->hash.load());
        if(right != nullNode)
            concatHash += std::to_string(right->hash.load());
        
        computed_hash = hash_hashes(concatHash);
        
        if(computed_hash != walker->hash.load())
            result = false;
        
        order.pop();
    }
    return result;
}

#endif //MERKLETREE_H

/*
     // Else we need an intermediary node
     else {
         newNode = new MerkleNode();
         switch(next->key.load() % 2) {
             case LEFT :
                 newNode->left.store(next);
                 break;
             case RIGHT :
                 newNode->right.store(next);
                 break;
         }
         switch(dir) {
             case LEFT :
                 if(walker->left.compare_exchange_weak(next, newNode)) {
                     do {
                         old_key = next->key.load();
                         new_key = old_key >> 1;
                     } while(!next->key.compare_exchange_weak(old_key, new_key));
                     walker = newNode;
                 } else {
                     walker = walker->left.load();
                     delete newNode;
                 }
                 break;
             case RIGHT :
                 if(walker->right.compare_exchange_weak(next, newNode)) {
                     do {
                         old_key = next->key.load();
                         new_key = old_key >> 1;
                     } while(!next->key.compare_exchange_weak(old_key, new_key));
                     walker = newNode;
                 } else {
                     walker = walker->right.load();
                     delete newNode;
                 }
                 break;
         }
     }
 */
