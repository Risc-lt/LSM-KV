#pragma once
#include <vector>
#include <cstddef>

template <typename K, typename V>
struct Node{
    K key;
    V value;
    int type;
    std::vector<Node<K, V>*> next; // pointers to next nodes in tower

    // constructor
    Node(){}; // default constructor

    Node(K key, V value, int height){
        this->key = key;
        this->value = value;
        this->type = height;

        for(int i = 0; i < height; i++){
            next.push_back(nullptr);
        }
    };

    // destructor
    ~Node(){};
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
    Skiplist(){};

    // destructor
    ~Skiplist(){};

    // interface for user
    Node<K, V>* insert(K elemKey, V elemValue);
    Node<K, V>* find(K elemKey);
    void remove(K elemKey);
};