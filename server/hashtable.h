
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>

typedef struct HNode{

    std::string key;
    std::string value;


    struct HNode *next;

} HNode;

typedef struct dict {

    int size;
    int taken;

    HNode **table;

} dict;

typedef struct HashTable{

    int size;
    int taken;
    bool resizing = false;
    int resizeIndex = 0;
    dict tables[2];

} HashTable;


HashTable* initHashTable(int size);

u_int64_t hash(std::string key);

void insert(HashTable* ht, std::string key, std::string value);

std::string get(HashTable* ht, std::string key);

void remove(HashTable* ht, std::string key);

void printHashTable(HashTable* ht);

void set(HashTable* ht, std::string key, std::string value);

bool contains(HashTable* ht, std::string key);

