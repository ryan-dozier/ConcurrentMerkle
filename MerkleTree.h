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

#define LEFT 0
#define RIGHT 1

enum NodeType { HASH, DATA };

template<typename T>
class MerkleTree {
public:
    class MerkleNode;
    //TODO: There may be a better wayt to declare this sentinal node value
    inline static MerkleNode* nullNode = nullptr;

    //TODO: I would prefer to utilize different a different hashing function, but this will work for the moment.
    inline static std::hash<T> hash_data;
    inline static std::hash<std::string> hash_hashes;

    MerkleTree() {
        // nullNode is a singleton, only want to change it if it has not been initialized
        this->root.store(new MerkleNode());
    };
    // TODO: This needs a full traversal to delete all nodes, will work on this later
    ~MerkleTree() {
        this->post_delete(root.load());
        delete nullNode;
    };

    // Inserts a value into the tree
    void insert(T &v) {
        size_t hash = hash_data(v);
        this->update(hash, v);
    };

    // Removes a value into the tree
    void remove(T v) {
        size_t hash = hash_data(v);
        T temp = NULL;
        this->update(hash, temp);
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
    void update(size_t hash, T &val);

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
    std::atomic<T> val;
    std::atomic<std::size_t> key;
    NodeType type;
    //TODO: This likely should be changed to be a string, but c++ doesnt have atomic string support. will have to think about this.
    std::atomic<std::size_t> hash;
    std::atomic<MerkleNode*> left;
    std::atomic<MerkleNode*> right;

    MerkleNode(size_t _hash, size_t key, T &v) {
        this->val.store(v);
        this->hash.store(_hash);
        this->key.store(key);
        this->type = DATA;
        this->left.store(nullNode);
        this->right.store(nullNode);
    };

    MerkleNode() {
        this->hash.store(0);
        this->val.store(NULL);
        this->key.store(0);
        this->type = HASH;
        this->left.store(nullNode);
        this->right.store(nullNode);
    };

    ~MerkleNode() {};

private:
};


template<typename T>
void MerkleTree<T>::update(std::size_t hash, T &val) {
    MerkleNode* walker = this->root.load();
    MerkleNode* prev = walker;
    short prevDir = 0;
    MerkleNode* next = nullNode;
    short nextDir = 0;
    size_t old_key;
    size_t new_key;


    std::stack<MerkleTree<T>::MerkleNode*> visited;
    std::size_t key = hash;
    bool finished = false;

    while(!finished) {
        // mark the current node as a parent node in need of updating
        switch(key % 2) {
            case LEFT :
                next = walker->left.load();
                nextDir = LEFT;
                break;
            case RIGHT :
                next = walker->right.load();
                nextDir = RIGHT;
                break;
        }
    
        
        if(next != nullNode) {
            // in the case that we are at a non-null node we simply remove a bit from the key and contintue the traversal
            key >>= 1;
        } else {
            // TODO: This may need to be done using a descriptor instead of several atomic variables.
            // CASE HashNode (nonleaf)
            if(walker->type == HASH) {
                // if the node is not a leaf node we decrement the key
                key >>= 1;
                MerkleNode* newNode = new MerkleNode(hash, key, val);

                switch (nextDir) {
                    case LEFT :
                        if(walker->left.compare_exchange_weak(nullNode, newNode)) {
                            finished = true;
                        } else {
                            next = walker->left.load();
                            delete newNode;
                        }
                        break;
                    case RIGHT :
                        if(walker->right.compare_exchange_weak(nullNode, newNode)) {
                            finished = true;
                        } else {
                            next = walker->right.load();
                            delete newNode;
                        }
                        break;
                }
            }
            // CASE DataNode (leaf)
            else {
                // There are two cases for leaf nodes
                // 1) the leaf node has the same hash as the current operation. In this case we update the value of the leaf.
                if(hash == walker->hash.load()) {
                    // this is where the update should be performed, will have to think about how interleaving will be.
                    walker->val.store(val);
                    finished = true;

                }
                // 2) the leaf has a different hash, we will need to add an intermediary node as the pending operation,
                //    and the current leaf have a common key bit to this point. To do this we will create a new node,
                //    determine which direction the current leaf node will branch from the intermediary, then compare
                //    and swap the new node to the previous node.
                else {
                    MerkleNode* newNode = new MerkleNode();

                    // Determine which direction the current leaf lies on the new parent node
                    switch (walker->key.load() % 2) {
                        case LEFT :
                            newNode->left.store(walker);
                            break;
                        case RIGHT :
                            newNode->right.store(walker);
                            break;
                    }

                    // TODO: I believe this can be done without walker and next. I think we can look ahead at
                    //          walker->left and ->right to determine if at the bottom of the tree instead of using next,
                    //          then we can easily keep track of the previous and which direction which would save some
                    //          operations. This is an optimization and may be worth doing in a future iteration.
                    // This is the last visited node
                    // check which direction walker lies on previous, this if/else block is symmetric for left/right
                    switch (prevDir) {
                        case LEFT :
                            // Attempt to CAS the new intermediary node.
                            if(prev->left.compare_exchange_weak(walker, newNode)) {
                                // Decrement the key by a bit now that there is a new interemediary node
                                do {
                                    old_key = walker->key.load();
                                    new_key = old_key >> 1;
                                } while(!walker->key.compare_exchange_weak(old_key, new_key));
                                
                                // set the next node to visit
                                next = newNode;
                            } else {
                                // If the CAS fails then another thread has performed an operation, because nodes are not removed from the tree,
                                // we can assume an intermediary node has been added. The CAS was on prev->direction, therefore load prev->direction
                                // and visit there next.
                                next = prev->left.load();
                                delete newNode;
                            }
                            break;
                        case RIGHT :
                            if(prev->right.compare_exchange_weak(walker, newNode)) {
                                do {
                                    old_key = walker->key.load();
                                    new_key = old_key >> 1;
                                } while(!walker->key.compare_exchange_weak(old_key, new_key));
                                next = newNode;
                            } else {
                                next = prev->right.load();
                                delete newNode;
                            }
                            break;
                    }
                }
            }
        }
        
        // TODO: I think the code would be cleaner by utilizing multiple types of nodes, leaf and nonleaf. I'll have to
        //          look at how to do this in c++, in java it would be by utilizing instanceof. But for c++ I'm not
        //          entirely sure how to do this.

        // We only want to update hashes of non-leaf nodes, this vector is used to keep track of which nodes will be affected by
        // update operation.
        if(walker->type == HASH)
            visited.push(walker);

        // continue the traversal
        prev = walker;
        prevDir = nextDir;
        walker = next;
    }
    
    // This section performs the hashing operations on the visited nodes simulating a recursive call stack.
    while(!visited.empty()) {

        // Grab the top node from the stack
        walker = visited.top();
        visited.pop();
        MerkleNode* left;
        MerkleNode* right;
        std::size_t temp;
        std::size_t newVal;
        std::string concatHash;
        do {
            concatHash = "";
            // grab a temporary copy of the hash, this will be used to check if the state has changed.
            temp = walker->hash.load();

            left = walker->left.load();
            right = walker->right.load();

            // obtain the hashes from the child nodes
            if(left != nullNode)
                concatHash += std::to_string(left->hash.load());
            if(right != nullNode)
                concatHash += std::to_string(right->hash.load());

            // compute the new hashes
            newVal = hash_hashes(concatHash);
            // Attempt to compare and swap the newly computed hash, if it fails another thread has updated the hash. Need to reload the
            // values and recompute the hashes for the next iteration.
        } while(!walker->hash.compare_exchange_weak(temp, newVal));
    }
}

template<typename T>
bool MerkleTree<T>::contains(std::size_t hash) {
    bool result = false;
    size_t key = hash;
    MerkleNode* walker = this->root.load();
    for (int i = 0; i < MAXBITS && walker != MerkleTree<T>::nullNode; i++) {

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

        if(curr->val.load() == NULL)
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
