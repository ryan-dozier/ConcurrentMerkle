//
//  SequentialMerkle.h
//  ConcurrentMerkle
//
//  Created by n00b on 1/27/20.
//

#ifndef SequentialMerkle_h
#define SequentialMerkle_h

#include <mutex>

namespace Sequential {

enum Direction { LEFT, RIGHT };
enum NodeType { HASH, DATA };


template<typename T>
class MerkleTree {
public:
    class MerkleNode;
    
    MerkleTree(std::string (*hash_func)(std::string)) {
        this->hashFunc = hash_func;
        this->root = new MerkleNode();
    };
    
    ~MerkleTree() {
        this->post_delete(root);
    };
    
    
    std::string getRootValue(){ return this->root->hash; };
    
    // Inserts a value into the tree
    void insert(T v) {
        this->lock.lock();
        std::string hash = hashFunc(std::to_string(v));
        size_t key = gen_key(hash);
        this->update(this->root, hash, key, v);
        this->lock.unlock();
    };
    
    // Removes a value into the tree
    void remove(T v) {
        this->lock.lock();
        std::string* hash = hashFunc(std::to_string(v));
        size_t key = gen_key(*hash);
        // TODO: this is probably wrong, will need to debug later
        T temp = NULL;
        this->update(this->root, "", key, temp);
        this->lock.unlock();
    };

    bool validate() {
        this->lock.lock();
        bool result = this->validate(this->root);
        this->lock.unlock();
        return result;
    };
    
    // checks if a value is in the tree.
    bool contains(T val) {
        this->lock.lock();
        std::string hash = hashFunc(std::to_string(val));
        size_t key = gen_key(hash);
        bool result = this->contains(this->root, hash, key);
        this->lock.unlock();
        return result;
    };
    
    void print_values() {
        print_values(this->root);
    }
    
private:
    std::mutex lock;
    void update(MerkleNode* walker, std::string hash, size_t key, T val);
    bool contains(MerkleNode* walker, std::string hash, std::size_t key);
    std::string (*hashFunc)(std::string);
    std::hash<std::string> gen_key;
    MerkleNode* root;
    
    bool validate(MerkleNode* node);
    
    void print_values(MerkleNode* node) {
        if (node != nullptr) {
            print_values(node->left);
            print_values(node->right);
            if(node->type == DATA) {
                std::cout << node->val << std::endl;
            }
        }
    }
    
    void post_delete(MerkleNode* node) {
        if (node == nullptr) {
            post_delete(node->left);
            post_delete(node->right);
            delete node;
        }
    }
    
};

template<typename T>
class MerkleTree<T>::MerkleNode {
public:
    
    MerkleNode(std::string _hash, size_t key, T &v) {
        this->val = v;
        this->hash = _hash;
        this->key = key;
        this->type = DATA;
        this->left = nullptr;
        this->right = nullptr;
    };
    
    MerkleNode() {
        this->hash = "";
        this->val = NULL;
        this->key = 0;
        this->type = HASH;
        this->left = nullptr;
        this->right = nullptr;
    };
    
    ~MerkleNode() {};
    
    T val;
    std::size_t key;
    NodeType type;
    //TODO: This likely should be changed to be a string, but c++ doesnt have atomic string support. will have to think about this.
    std::string hash;
    MerkleNode* left;
    MerkleNode* right;
};

template<typename T>
bool MerkleTree<T>::contains(MerkleNode* walker, std::string hash, std::size_t key) {
    if (walker == nullptr)
        return false;
    
    if (walker->type == DATA) {
        if(hash.compare(walker->hash) == 0)
            return true;
        else
            return false;
    }
    
    switch (key % 2) {
        case LEFT :
            return contains(walker->left, hash, key >> 1);
            break;
        case RIGHT :
            return contains(walker->right, hash, key >> 1);
            break;
    }
}

template<typename T>
void MerkleTree<T>::update(MerkleNode* walker, std::string hash, std::size_t key, T val) {
    Direction dir;
    MerkleNode* next;
    MerkleNode* newNode;
    switch(key % 2) {
        case LEFT :
            dir = LEFT;
            next = walker->left;
            break;
        case RIGHT :
            dir = RIGHT;
            next = walker->right;
            break;
    }
    
    if(next == nullptr) {
        switch(dir) {
            case LEFT :
                walker->left = new MerkleNode(hash, key >> 1, val);
                break;
            case RIGHT :
                walker->right = new MerkleNode(hash, key >> 1, val);
                break;
        }
    } else {
        switch(next->type) {
            case DATA :
                newNode = new MerkleNode();
                switch(next->key % 2) {
                    case LEFT :
                        newNode->left = next;
                        break;
                    case RIGHT :
                        newNode->right = next;
                        break;
                }
                next->key >>= 1;
                
                switch(dir) {
                    case LEFT :
                        walker->left = newNode;
                        break;
                    case RIGHT :
                        walker->right = newNode;
                        break;
                }
                
                this->update(newNode, hash, key >> 1, val);
                break;


            case HASH :
                this->update(next, hash, key >> 1, val);
                break;
        }
    }
        
    std::string newHash = "";
    // obtain the hashes from the child nodes
    if(walker->left != nullptr)
        newHash += walker->left->hash;

    if(walker->right != nullptr)
        newHash += walker->right->hash;
    
    // compute the new hashes
    walker->hash = hashFunc(newHash);
}

template<typename T>
bool MerkleTree<T>::validate(MerkleNode* node) {
    if (node == nullptr)
        return true;
    if(!validate(node->left))
        return false;
    if(!validate(node->right))
        return false;
    
    std::string newHash = "";

    switch(node->type) {
        case DATA :
            newHash = sha256(std::to_string(node->val));
            break;
        case HASH :
            // obtain the hashes from the child nodes
            if(node->left != nullptr)
                newHash += node->left->hash;

            if(node->right != nullptr)
                newHash += node->right->hash;
            
            // compute the new hashes
            newHash = hashFunc(newHash);
            break;
    }
    
    if(newHash.compare(node->hash) != 0)
        return false;
    else
        return true;
}

} // END namespace

#endif /* SequentialMerkle_h */
