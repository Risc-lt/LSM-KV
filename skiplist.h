#pragma once
#include <cassert>
#include <climits>
#include <ctime>
#include <iostream>
#include <list>
#include <vector>
#include <cstddef>
#include <map>

// define the node type and constants
const int nodeType_Head = 1;
const int nodeType_Data = 2;
const int nodeType_End = 3;
const int MAX_Level = 8;
const double Jump_Probability = 0.5;

// structure definition of node and skiplist
template <typename K, typename V>
struct Node{
    K key;
    V value;
    int type;
    // pointers to next nodes in differernt levels of a node
    std::vector<Node<K, V>*> next; 

    // constructor
    Node(){}; // default constructor

    // constructor for a storage node
    Node(K key, V value, int height){
        this->key = key;
        this->value = value;
        this->type = nodeType_Data;

        for(int i = 0; i < height; i++){
            // initialize the next pointers to nullptr
            next.push_back(nullptr);
        }
    };

    // constructor for a head or end node
    Node(int type){
        this->type = type;
    };

    // destructor
    ~Node(){
        for(auto &pointer : next){
            delete pointer;
        }
        next.clear();
    };
};

template <typename K, typename V>
class Skiplist{
private:
    size_t size;
    Node<K, V>* head;

    // height of a node
    int randomLevel();
    // insert / delete / search a node
    Node<K, V>* insertNode(K elemKey, V elemValue);
    Node<K, V>* findNode(K elemKey);
    void deleteNode(K elemKey);

public:
    // constructor
    Skiplist();

    // destructor
    ~Skiplist();

    // interface for user
    Node<K, V>* insert(K elemKey, V elemValue);
    Node<K, V>* find(K elemKey);
    void remove(K elemKey);

    // copy current skiplist to a list
    void copyAll(std::list<std::pair<K, V>> &list);

    // get size of skiplist
    size_t getSize();

    void clear(); // clear all the vectors in skiplist
    void tranverse(); // tranverse the skiplist to print all the elements
};

// random level generator
template <typename K, typename V>
int Skiplist<K, V>::randomLevel(){
    int level = 1;
    while(level < MAX_Level && (rand() * 1.0 / INT_MAX) <= Jump_Probability){
        level++;
    }
    return level;
}

// initialize a skiplist of two nodes
template <typename K, typename V>
Skiplist<K, V>::Skiplist(){
    // get the random seed
    srand(time(NULL));
    
    // initialize the head and end node
    head = new Node<K, V>(nodeType_Head);
    Node<K, V>* end = new Node<K, V>(nodeType_End);

    for(int i = 0; i < MAX_Level; i++){
        head->next.push_back(end);
        end->next.push_back(nullptr);
    }

    // record the size
    size = 0;
}

// destructor of skiplist
template <typename K, typename V>
Skiplist<K, V>::~Skiplist(){
    Node<K, V>* cur = head;
    while(cur != nullptr){
        Node<K, V>* next = cur->next[0];
        delete cur;
        cur = next;
    }
    delete cur;
}

// insert a node into the skiplist
template <typename K, typename V>
Node<K, V>* Skiplist<K, V>::insertNode(K elemKey, V elemValue){
    // store every node that has been visited before the insertion point in each level
    std::map<int, std::vector<Node<K, V>*>> findPath;

    /*******************************************
        Step 1: Check the key and find the path 
    ********************************************/

    // tranverse the skiplist to check if the key exists
    Node<K, V>* tryFind = this->findNode(elemKey);
    if(tryFind != nullptr){
        tryFind->value = elemValue;
        return tryFind;
    }

    // build a new node if the key does not exist
    int newNode_level = randomLevel();

    Node<K, V>* newNode = new Node<K, V>(elemKey, elemValue, newNode_level);
    Node<K, V>* iter = head;

    // find the path to insert the new node
    // move the iterator to the node before the insertion point(i.e. A(iter)->new->B)
    while(iter->next[0]->type != nodeType_End){
        bool ifJump = false;

        // tranverse all level of current node from top to bottom
        for(int i = iter->next.size() - 1; i >= 0; i--){
            findPath[i].push_back(iter);
            Node<K, V>* level_next = iter->next[i];

            // avoid iterator to be out of range
            assert(level_next != nullptr);

            // if the next node is the end node, jump to the next level
            if(level_next->type == nodeType_End){
                continue;
            }

            // if the next node is the data node, compare the key
            // if the key of the next node is larger than the new key, jump to the node
            if(level_next->key < elemKey){
                iter = level_next;
                ifJump = true;
                findPath[i].push_back(iter);
                break;
            }

            // if the key of the next node is smaller than the new key, continue to the next level
            // continue to the next level without doing anything
        }

        // if the iterator does not jump to the next node after tranversing all levels
        // just insert the new node after the iterator
        if(!ifJump){
            break;
        }
    }

    /*******************************************
        Step 2: Insert the new node 
    ********************************************/

    // insert the new node into the skiplist
    Node<K, V>* iterNext = iter->next[0];

    //  --------         ----------      ---------------------
    // |        |       |          |    |                     | 
    // |  iter  |   ->  |  newNode | -> |       iterNext      |
    // |        |       |          |    |                     |
    //  --------          --------       ---------------------

    // update all levels between iter node and newNode
    for(int i = 0; i < std::min(newNode_level, (int)iter->next.size()); i++){
        iter->next[i] = newNode;
    }

    // update the node before iter node to point to the new node
    for(auto mapIter = findPath.begin(); mapIter != findPath.end(); mapIter++){
        // get the level from the element of map
        int level = mapIter->first;

        // exit if the level is too high
        if(level > (int)newNode->next.size())
            break;
        
        // find the closest node before the new node in the level
        Node<K, V>* levelLastNode = mapIter->second[mapIter->second.size() - 1];

        // set the node to point to the new node
        levelLastNode->next[level] = newNode;
    }

    // update the new node to point to the rest of the nodes
    for(int i = 0; i < newNode_level; i++){
        // tranverse the level from bottom to top
        while(iterNext->type != nodeType_End && i + 1 > (int)iterNext->next.size()){
            // jump to the next node if the level is higher than the current node 
            iterNext = iterNext->next[iterNext->next.size() - 1];
        }
        newNode->next[i] = iterNext;
    }
    // update the size of the skiplist
    this->size++;

    return newNode;
}

// find a node in the skiplist
template <typename K, typename V>
Node<K, V>* Skiplist<K, V>::findNode(K elemKey){
    Node<K, V>* iter = head;

    // tranverse the skiplist to find the node
    while(iter->next[0]->type != nodeType_End){
        bool ifJump = false;

        // tranverse all level of current node from top to bottom
        for(int i = iter->next.size() - 1; i >= 0; i--){
            Node<K, V>* level_next = iter->next[i];

            // avoid iterator to be out of range
            assert(level_next != nullptr);

            // if the next node is the end node, jump to the next level
            if(level_next->type == nodeType_End)
                continue;
            
            // if the next node is the data node, compare the key
            // if the key of the next node is larger than the new key, jump to the node
            if(level_next->key < elemKey){
                iter = level_next;
                ifJump = true;
                break;
            } 

            // if the key of the next node is what we search for
            else if(level_next->key == elemKey){
                return level_next;
            }

            // if the key of the next node is smaller than the new key, continue to the next level
            // continue to the next level without doing anything
        }

        // if the iterator does not jump to the next node after tranversing all levels
        // the node should just be the next one but not found
        if(!ifJump){
            break;
        }
    }
    
    return nullptr;
}

// delete a node in the skiplist
template <typename K, typename V>
void Skiplist<K, V>::deleteNode(K elemKey){
    // store every node that has been visited before the deletion point in each level
    std::map<int, std::vector<Node<K, V>*>> findPath;

    /*******************************************
        Step 1: Check the key and find the path 
    ********************************************/

    // check whether the to-delete node exists
    Node<K, V>* delNode = this->findNode(elemKey);
    if(delNode == nullptr)
        return;
    
    // find the path to delete the node
    Node<K, V>* iter = head;
    while(iter->next[0]->type != nodeType_End){
        bool ifJump = false;

        // tranverse all level of current node from top to bottom
        for(int i = iter->next.size() - 1; i >= 0; i--){
            findPath[i].push_back(iter);
            Node<K, V>* level_next = iter->next[i];

            // avoid iterator to be out of range
            assert(level_next != nullptr);

            // if the next node is the end node, jump to the next level
            if(level_next->type == nodeType_End)
                continue;
            
            // if the next node is the data node, compare the key
            // if the key of the next node is larger than the new key, jump to the node
            if(level_next->key < elemKey){
                iter = level_next;
                ifJump = true;
                findPath[i].push_back(iter);
                break;
            }

            // if the next node is the head node, print error message
            if(level_next->type == nodeType_Head){
                std::cout << "Error: head again\n" << iter->key << std::endl;
                this->tranverse();
                exit(0);
            }

            // if the key of the next node is what we search for
            if(i == 0 && level_next->key == elemKey){
                // already jump the loop, set ifJump to false in case of trapped in loop again
                ifJump = false; 
                break;
            }

            // if the key of the next node is smaller than the new key, continue to the next level
            // continue to the next level without doing anything
        }

        // if the iterator does not jump to the next node after tranversing all levels
        // the node should just be the next one but not found
        if(!ifJump){
            break;
        }
    }

    // check if the node to delete is found
    assert(iter->next[0] == delNode);

    /*******************************************
        Step 2: Delete the node 
    ********************************************/

    Node<K,V>* delNodeNext  = delNode->next[0];

    //  --------        ----------      ---------------------
    // |        |      |          |    |                     | 
    // |  iter  |  ->  | delNode  | -> |     delNodeNext     |
    // |        |      |          |    |                     |
    //  --------         --------       ---------------------
    
    // update all levels between delNode and iter node
    for(unsigned int i = 0; i < std::min(delNode->next.size(), iter->next.size()); i++){
        iter->next[i] = delNode->next[i];
    }

    // update the node before iter node to point to the new node
    for(auto mapIter = findPath.begin(); mapIter != findPath.end(); mapIter++){
        // get the level from the element of map
        int level = mapIter->first;

        // find the closest node before the new node in the level
        Node<K, V>* levelLastNode = mapIter->second[mapIter->second.size() - 1];

        // if the level of delNodeNext is too low, search for the next node
         while(delNodeNext->type != nodeType_End && level + 1 > (int)delNodeNext->next.size()){
            delNodeNext = delNodeNext->next[delNodeNext->next.size() - 1];
        }

        // set the node to point to the new node
        levelLastNode->next[level] = delNodeNext;
    }
    delete delNode;
    this->size--;
    
    return;
}

// get the size of the skiplist
template <typename K, typename V>
size_t Skiplist<K, V>::getSize(){
    return this->size;
}

// clear all the nodes in skiplist
template <typename K, typename V>
void Skiplist<K, V>::clear(){
    Node<K, V>* iter = head;
    Node<K, V>* del_node = nullptr;

    while(iter->type != nodeType_End){
        del_node = iter;
        iter = iter->next[0];

        if(del_node->type == nodeType_Data){
            delete del_node;
        }
    }
    // set the size to 0
    this->size = 0;

    for(int i = 0; i < MAX_Level; i++){
        // iter -> end for now
        head->next[i] = iter;
    }
}

// tranverse the skiplist to print all the elements
template <typename K, typename V>
void Skiplist<K, V>::tranverse(){
    std::cout << "####################" << std::endl;
    Node<K, V>* iter = head;
    while(iter->type != nodeType_End){
        
        // if current node is head
        if(iter->type == nodeType_Head){
            std::cout << "Head level [";

            for(size_t i = 0; i < iter->next.size(); i++){
                if(iter->next[i]->type == nodeType_End)
                    std::cout << "End, ";
                else if(iter->next[i]->type == nodeType_Head)
                    std::cout << "Head, ";
                else if(iter->next[i]->type == nodeType_Data)
                    std::cout << iter->key << ", ";
            }

            std::cout << "]" << std::endl;
            iter = iter->next[0];
            continue;
        }

        // if current node is data
        std::cout << "key: " << iter->key << "value: " << iter->value << " level [";
        for(size_t i = 0; i < iter->next.size(); i++){
            if(iter->next[i]->type == nodeType_End)
                std::cout << "End, ";
            else if(iter->next[i]->type == nodeType_Head)
                std::cout << "Head, ";
            else if(iter->next[i]->type == nodeType_Data)
                std::cout << iter->next[i]->key << ", ";
        }
        std::cout << "]" << std::endl;

        iter = iter->next[0];
    }
}

/***************************************
        interface for user
****************************************/
// insert
template <typename K, typename V>
Node<K, V>* Skiplist<K, V>::insert(K elemKey, V elemValue){
    return this->insertNode(elemKey, elemValue);
}

// find
template <typename K, typename V>
Node<K, V>* Skiplist<K, V>::find(K elemKey){
    return this->findNode(elemKey);
}

// remove
template <typename K, typename V>
void Skiplist<K, V>::remove(K elemKey){
    this->deleteNode(elemKey);
}

// copy all the elements in skiplist to a list
template <typename K, typename V>
void Skiplist<K, V>::copyAll(std::list<std::pair<K, V>> &list){
    Node<K, V>* iter = head;
    while(iter->type != nodeType_End){
        if(iter->type == nodeType_Data){
            list.push_back(std::make_pair(iter->key, iter->value));
        }
        iter = iter->next[0];
    }
}