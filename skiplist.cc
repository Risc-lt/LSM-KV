#pragma once
#include <climits>
#include <ctime>
#include <list>
#include <vector>
#include <cstddef>

// define the node type and constants
const int nodeType_Head = 1;
const int nodeType_Data = 2;
const int nodeType_End = 3;
const int MAX_Level = 8;
const double Jump_Probability = 0.5;


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
        this->type = height;

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

    // height of a tower
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

    void clear();
    void tranverse();
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
}