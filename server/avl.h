#include <iostream>

class Node {


public:
    int key;
    Node *left;
    Node *right;
    int height;
    Node(int key){
        this->key = key;
        this->left = NULL;
        this->right = NULL;
        this->height = 1;
    }
    Node(int key, Node *left, Node *right){
        this->key = key;
        this->left = left;
        this->right = right;
        this->height = 1;
    }
    ~Node(){
        delete left;
        delete right;
    }


};


class AVLTree {

public:
    
    int height(Node *node);
    int balance(Node *node);
    Node *rotateRight(Node *node);
    Node *rotateLeft(Node *node);
    Node *root;
    Node *insert(Node *node, int key);
    Node *remove(Node *node, int key);
    Node *min(Node *node);
    Node *max(Node *node);
    Node *search(Node *node, int key);

};