//
// Created by Ryan Dozier on 1/6/20.
//

#ifndef MERKLETREE_H
#define MERKLETREE_H

#include <iostream>
#include <atomic>
#include <stack>
#include <string>
#include <boost/lexical_cast.hpp>
#include <boost/functional/hash.hpp>

namespace ConcurrentVerifier {

// Merkle trees contain two types of nodes, HASH, and DATA
enum NodeType { HASH, DATA };
enum OpType { ADD = 1, REMOVE = -1 };

/**
 * Class MerkleTree
 *
 */
template<typename T>
class MerkleTreeVerifier {
    
protected:
    class MerkleNode;
    class Descriptor;
    
public:
    
    //TODO: There may be a better way to declare this sentinal node value
    inline static MerkleNode* nullNode = nullptr;
    
    /**
     * Constructor to create a merkle tree object. The required parameter is a hashing function
     * which takes an std::string and returns the hash as an std::string
     */
    MerkleTreeVerifier(int index_bits, std::string (*hash_func)(std::string)) {
        this->hashFunc = hash_func;
        this->index_bits = index_bits;
        this->num_child = 1 << index_bits;
        this->root.store(new MerkleNode(this->num_child));
    };

    ~MerkleTreeVerifier() {
        // call recursive helper function
        this->post_delete(root.load());
        delete nullNode;
    };
    
    // Inserts a value into the tree
    void insert(T &v) {
        //std::string* hash = new std::string(hashFunc(std::to_string(*v)));
        size_t hash = hasher(boost::lexical_cast<std::string>(*v));
        this->update(hash, hash, v, ADD); // TODO: remove one of the parameters, hash == key for boost
    };
    
    // Removes a value into the tree
    void remove(T &v) {
        size_t hash = hasher(boost::lexical_cast<std::string>(*v));
        this->update(hash, hash, v, REMOVE); // TODO: remove one of the parameters, hash == key for boost
    };
    
    // TODO: This function will require a lot of thought as it will likely have to be blocking. I want to have
    //          insert/remove/contains flushed out first. My current idea is to have validate block access to the tree
    //          during the operation, but to allow other threads to add pending operations to a queue. The contains
    //          operation will be able to look at the pending operations and the tree to determine if the item is
    //          present. There is also a possiblility of method crossing if a remove and insert are called during the
    //          validate operation.
    bool validate();
    
    // checks if a value is in the tree.
    bool contains(T v) {
        //std::string hash = hashFunc(std::to_string(*val));
        //size_t key = gen_key(hash);
        size_t hash = hasher(boost::lexical_cast<std::string>(*v));
        return this->contains(hash, hash); // TODO: remove one of the parameters, hash == key for boost
    };
    
    // returns the value of the root hash
    std::size_t getRootValue() { return root.load()->hash.load(); };
    
    // prints all DATA Nodes in postorder traversal. (Mainly for debugging)
    void print_values() { print_values(this->root.load()); }
    

private:
    // root node
    std::atomic<MerkleNode*> root;
    
    // hash function
    std::string (*hashFunc)(std::string);
    boost::hash<std::string> hasher;

    // hashing function to generate node keys
    std::hash<std::string> gen_key;
    
    // The update function performs both inserts and removes depending on the parameters.
    void update(std::size_t hash, size_t key, T &val, OpType op);
    
    // finishOp allows other executing threads to help finish the operation
    void finishOp(Descriptor* job);
    
    // underlying contains operation, it generates the hash / key we are looking for
    bool contains(std::size_t hash, std::size_t key);

    int num_child;
    int index_bits;

    /**
    * This function does a postorder traversal of the tree and deallocates the used memory.
    * It is NOT thread safe.
    */
    void post_delete(MerkleNode* node) {
        if (node != nullNode) {
            for(int i = 0; i < this->num_child; i++)
                post_delete(node->children[i]->load());
            //delete node->hash.load();
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
            for(int i = 0; i < this->num_child; i++)
                print_values(node->children[i].load());

            if(node->type == DATA) {
                std::cout << *(node->val) << std::endl;
            }
        }
    };
};

/**
* Class MerkleNode
* This class handles induvidual nodes within the merkle tree. Most items have been left as public for ease of use.
* However, because this is a protected class, and the MerkleTree object only contains a MerkleNode as a private variable
* nothing in this class can be modified by the user.
*/
template<typename T>
class MerkleTreeVerifier<T>::MerkleNode {
public:
    // What is stored by the tree
    T val;
    // the "key" is the remaining path in the tree, as intermediary nodes are added this determines left/right
    std::size_t key;
    // HASH or DATA, a HASH node points to other HASH or DATA nodes, and its value is the hash(child_hash_0 + ... child_hash_N)
    NodeType type;
    std::atomic<std::size_t> hash;
    std::atomic<int> count;
    // The description of a pending operation on this node, other threads can help complete it.
    std::atomic<Descriptor*> desc;
    std::vector<std::atomic<MerkleNode*>*> children;
    
    MerkleNode(int child_nodes, size_t _hash, T &v, OpType op) {
        this->val = v;
        this->hash.store(_hash);
        this->type = DATA;
        // TODO: check that op is the correct value
        this->count.store(op);
        this->desc.store(nullptr);
        for(int i = 0; i < child_nodes; i++)
            this->children.push_back(new std::atomic<MerkleNode*>(nullptr));
    };
    
    MerkleNode(int child_nodes, size_t _hash, size_t key, T &v) {
        this->val = v;
        this->hash.store(_hash);
        this->key = key;
        this->type = DATA;
        this->count.store(0);
        this->desc.store(nullptr);
        for(int i = 0; i < child_nodes; i++)
            this->children.push_back(new std::atomic<MerkleNode*>(nullptr));
    };
    
    MerkleNode(int child_nodes) {
        this->hash.store(0);
        this->val = NULL;
        this->key = 0;
        this->type = HASH;
        this->desc.store(nullptr);
        for(int i = 0; i < child_nodes; i++)
            this->children.push_back(new std::atomic<MerkleNode*>(nullptr));
    };
    
    ~MerkleNode() {
        delete this->desc.load();
    };
    
    void resetChildNodes() {
        for(int i = 0; i < this->children.size(); i++) {
            this->children[i]->store(nullptr);
        }
    }
};

/**
* Class Descriptor
* This class is used to describe pending operations that need to occur. Typically Descriptor objects are needed 
* when multiple words need to be atomically modified atomically.
*/
template<typename T>
class MerkleTreeVerifier<T>::Descriptor
{
public:
    // Whether the operation has been completed
    bool pending;
    // We have two types of operations, HASH node, and DATA node, there may be additional ones for remove.
    // For each of these, the operation is slightly different.
    NodeType nodeType;
    OpType opType;
    MerkleNode* parent;
    MerkleNode* oldChild;
    MerkleNode* child;
    int prevCount;
    int dir;
    size_t key;
    
    
    Descriptor() {
        this->pending = true;
        this->typeOp = HASH;
    };
    
    Descriptor(MerkleNode* _parent, MerkleNode* _child, MerkleNode* _oldChild, int _dir, size_t _key) {
        this->pending = true;
        this->nodeType = HASH;
        this->parent = _parent;
        this->child = _child;
        this->oldChild = _oldChild;
        this->dir = _dir;
        this->key = _key;
    };
    
    Descriptor(MerkleNode* _child, OpType _op) {
        this->pending = true;
        this->nodeType = DATA;
        this->child = _child;
        this->opType = _op;
    };
    
    ~Descriptor() {};
    
    void setCount(int _update) {
        this->prevCount = _update;
    };

    void setDataDescriptor(MerkleNode* _parent, int _dir) {
        this->parent = _parent;
        this->dir = _dir;
    };
    
    void setHashDescriptor(MerkleNode* _parent, MerkleNode* _child, MerkleNode* _oldChild, int _dir, size_t _key) {
        this->parent = _parent;
        this->child = _child;
        this->oldChild = _oldChild;
        this->dir = _dir;
        this->key = _key;
    };
private:
};

template<typename T>
void MerkleTreeVerifier<T>::update(std::size_t hash, std::size_t key, T &val, OpType op) {
    // walks through the tree
    MerkleNode* walker = this->root.load();
    MerkleNode* next = nullptr;
    Descriptor* currentDesc;
    std::stack<MerkleTreeVerifier<T>::MerkleNode*> visited;

    // Allocate the Data node to insert
    MerkleNode* dataNode = new MerkleNode(this->num_child, hash, val, op);
    Descriptor* dataDesc = new Descriptor(dataNode, op);
    
    // These are used if an intermediary node is needed
    MerkleNode* hashNode = nullptr;
    Descriptor* hashDesc = nullptr;
    int dir;
    bool finished = false;
    
    while(!finished) {
        // After insertion we will need to update the hashes along the path. Nodes on the path are stored in the stack
        visited.push(walker);
        
        // Grab the current descriptor
        currentDesc = walker->desc.load();
        // Finish its pending operation
        finishOp(currentDesc);
        
        
        // Determine which direction we need to traverse and load next based on the direction
        dir = key % this->num_child;
        next = walker->children[dir]->load();
        
        // In this function we stop if the next type is DATA or nullptr.
        // Therefore walker must be a HASH node, we can try to insert a DATA node at next
        if(next == nullptr) {
            // set the value of the new node's key
            dataNode->key = key >> this->index_bits;
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
            if(next->hash == hash) { // TODO: once this is changed to long for hashes, a hash comparison is fine
                // walker node is the target, all we have to do is modify the count then update the hashes.
                next->count.fetch_add(op);
                finished = true;
            } else {
                /**
                 * Next is currently at a differing DATA node along the path the inserted node
                 * needs to be apended. An intermediary hash node needs to be created and placed
                 * between walker and next.
                 */
                
                // Check if we have an already allocated node
                if(hashNode == nullptr) {
                    // Create a new node and descriptor
                    hashNode = new MerkleNode(this->num_child);
                    hashDesc = new Descriptor(walker, hashNode, next, dir, next->key >> this->index_bits);
                }
                else {
                    // Clean the allocated node and reinitialize the descriptor
                    hashNode->resetChildNodes();
                    hashDesc->setHashDescriptor(walker, hashNode, next, dir, next->key >> this->index_bits);
                }

                // Check which direction next should be apended to the intermediary node
                hashNode->children[next->key % this->num_child]->store(next);

                // Try to swap the new descriptor
                if(walker->desc.compare_exchange_weak(currentDesc, hashDesc)) {
                    // finish the operation
                    finishOp(hashDesc);
                    
                    // Get ready for the next loop iteration
                    key >>= this->index_bits;
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
            key >>= this->index_bits;
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
        MerkleNode* current;
        visited.pop();
        std::size_t oldHash;
        std::size_t newVal;
        do {
            // TODO: there may be a more efficient way to do this string arithmetic
            newVal = 0;
            // grab a temporary copy of the hash, used to detect state changes
            oldHash = walker->hash.load();
            for(int i = 0; i < this->num_child; i++) {
                current = walker->children[i]->load();
                
                // obtain the hashes from the child node
                if(current != nullptr && current->count.load() > 0)
                    newVal ^= current->hash.load();
            }
            
            // compute the new hashes
            newVal = hasher(boost::lexical_cast<std::string>(newVal));
            // Attempt to compare and swap the newly computed hash, if it fails another thread has
            // updated the hash. Need to reload the values and recompute the hashes for the next iteration.
        } while(!walker->hash.compare_exchange_weak(oldHash, newVal));
        
        // TODO: this is a memory leak, but need to think about how to solve it. The issue is oldhash could be referenced by other threads as they work.
        // TODO: Thought 1 : maybe collect discareded old strings to remove later on qqueue?
        //delete oldHash;
    }
}


// Allows threads to help complete a pending operation
template<typename T>
void MerkleTreeVerifier<T>::finishOp(Descriptor* job) {
    if(job != nullptr) {
        std::atomic<MerkleNode*>* update_node;
        // only try to finish the operation if pending
        if(job->pending) {
            // This switch grabs the address atomic variable that will be updated.
            update_node = job->parent->children[job->dir % this->num_child];

            switch(job->nodeType) {
                // For HASH operations, we are adding an intermediary HASH node between an existing HASH Node
                // and a DATA node. (The insert operation must be along the same key path) This operation 
                // requires two "atomic" operations. First we need to swap in the new intermediary node, then
                // we need to modify the old DATA node's key value.
                case HASH :
                    update_node->compare_exchange_weak(job->oldChild, job->child);
                    if(job->oldChild->key != job->key)
                        job->oldChild->key = job->key;
                    break;
                // For DATA node Operations we simply have to compare and swap in the new DATA node into the parent.
                case DATA :
                    update_node->compare_exchange_weak(nullNode, job->child);
                    break;
            }
            // Mark the job as completed.
            job->pending = false;
        }
    }
}

/**
 * TODO: I think I need to add support on contains to verify the hashes are consistant. This may be difficult as pending operations could be occuring.
 */
template<typename T>
bool MerkleTreeVerifier<T>::contains(std::size_t hash, std::size_t key) {
    std::stack<MerkleTreeVerifier<T>::MerkleNode*> visited;
    bool result = false;
    MerkleNode* walker = this->root.load();
    while (walker != nullptr) {
        visited.push(walker);
        // Arrived at a leaf node.
        if (walker->type == DATA) {
            // check if the leaf contains the correct value
            if (walker->hash.load() == hash)
                result = true;
            // break from the loop, if the hash was not equal the value is not in the tree as we are at the bottom
            break;
        }
        
        // If there are further parent nodes determine which direction to continue the search and set the walker to the next node to search.
        walker = walker->children[key % this->num_child]->load();

        // decrement the key and continue.
        key >>= this->index_bits;
    }
    
    // TODO: This is where the hashes likely need to be verified.
    
    return result;
}

// TODO: This will not currently work with concurrent execution. However it does work sequentially for testing at the moment.
template<typename T>
bool MerkleTreeVerifier<T>::validate() {
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
        
        for(int i = 0; i < this->num_child; i++) {
            if (curr->children[i]->load() != nullNode)
                stk.push(curr->children[i]->load());
        }
    }
    
    MerkleNode *walker, *current;
    std::size_t computed_hash;
    
    //TODO: This could probably be optimized some.
    while (!order.empty()) {
        computed_hash = 0;
        walker = order.top();
        
        for(int i = 0; i < this->num_child; i++) {
            current = walker->children[i]->load();
            
            // obtain the hashes from the child node
            if(current != nullNode && current->count.load() > 0)
                computed_hash ^= current->hash.load();
        }
        
        computed_hash = hasher(boost::lexical_cast<std::string>(computed_hash));
        if(computed_hash != walker->hash.load())
            result = false;
        
        order.pop();
    }
    return result;
}

} // end Concurrent Namespace
#endif //MERKLETREE_H
