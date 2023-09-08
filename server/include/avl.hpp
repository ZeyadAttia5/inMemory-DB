#include <iostream>
#include <string>

#ifndef AVL_HPP
#define AVL_HPP

class Node
{

public:
    int key;
    std::string value;
    Node *left;
    Node *right;
    int height;
    Node(int key, std::string value)
    {
        this->key = key;
        this->value = value;
        this->left = NULL;
        this->right = NULL;
        this->height = 1;
    }
    Node(int key, std::string value, Node *left, Node *right)
    {
        this->key = key;
        this->value = value;
        this->left = left;
        this->right = right;
        this->height = 1;
    }
    ~Node()
    {
        delete left;
        delete right;
    }
};

class AVLTree
{

    

    public:
        int size = 0;
        int height(Node *node);
        int balance(Node *node);
        Node *rotateRight(Node *node);
        Node *rotateLeft(Node *node);
        Node *root;
        Node *insert(Node *node, int key, std::string value);
        Node *remove(Node *node, int key);
        Node *min(Node *node);
        Node *max(Node *node);
        Node *search(Node *node, int key);
        bool contains(Node *node, int key);
        void deleteTree(Node *node);
};

#endif