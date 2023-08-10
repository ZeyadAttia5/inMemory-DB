#include "hashtable.h"
#include <cstdlib>
#include <cstring>
#include <bits/stdc++.h>

// Function that returns true if n
// is prime else returns false
bool isPrime(int n)
{
    // Corner cases
    if (n <= 1)
        return false;
    if (n <= 3)
        return true;

    // This is checked so that we can skip
    // middle five numbers in below loop
    if (n % 2 == 0 || n % 3 == 0)
        return false;

    for (int i = 5; i * i <= n; i = i + 6)
        if (n % i == 0 || n % (i + 2) == 0)
            return false;

    return true;
}

// Function to return the smallest
// prime number greater than N
int nextPrime(int N)
{

    // Base case
    if (N <= 1)
        return 2;

    int prime = N;
    bool found = false;

    // Loop continuously until isPrime returns
    // true for a number greater than n
    while (!found)
    {
        prime++;

        if (isPrime(prime))
            found = true;
    }

    return prime;
}

u_int64_t hash(std::string key, int size)
{

    u_int64_t hash = 0;

    for (int i = 0; key[i] != '\0'; i++)
    {

        hash += key[i] * (i + 1) * (i + 1);
    }

    hash = (hash * 15485863) % size;

    return hash;
}

HashTable *initHashTable(int size)
{

    HashTable *ht = (HashTable *)malloc(sizeof(HashTable));

    ht->size = size;

    dict table1 = {size, 0, (HNode **)malloc(sizeof(HNode *) * size)};

    dict table2 = {size, 0, (HNode **)malloc(sizeof(HNode *) * size)};

    ht->tables[0] = table1;
    ht->tables[1] = table2;

    for (int i = 0; i < size; i++)
    {

        ht->tables[0].table[i] = NULL;
        ht->tables[1].table[i] = NULL;
    }
    return ht;
}

dict initDict(int size)
{

    dict *newDict = (dict *)malloc(sizeof(dict));

    newDict->size = size;

    newDict->taken = 0;

    newDict->table = (HNode **)malloc(sizeof(HNode *) * size);

    for (int i = 0; i < size; i++)
    {

        newDict->table[i] = NULL;
    }

    return *newDict;
}

std::string _getHelper(dict *dict, std::string key)
{

    int index = hash(key, dict->size);

    HNode *node = dict->table[index];

    while (node != NULL)
    {

        if (node->key == key)
        {

            return node->value;
        }

        node = node->next;
    }

    return "";
}

void _set_helper(dict *dict, std::string key, std::string value)
{

    int index = hash(key, dict->size);

    HNode *node = dict->table[index];

    while (node != NULL)
    {

        if (node->key == key)
        {

            node->value = value;

            return;
        }

        node = node->next;
    }

    // TODO: why *2?

    HNode *newNode = new HNode;

    std::cout << "adding " << key << " to index " << index << std::endl;

    std::cout << "size: " << dict->size << std::endl;

    newNode->key = key;
    newNode->value = value;
    newNode->next = dict->table[index];

    dict->table[index] = newNode;

    dict->taken++;
}

int const N = 4;

void resizeUp(HashTable *ht, int newSize)
{

    std::cout << "resizing Up     ";

    ht->tables[1].size = newSize;

    ht->tables[1].table = (HNode **)realloc(ht->tables[1].table, sizeof(HNode *) * newSize);

    ht->tables[1].taken = 0;

    std::cout << "Done" << std::endl;
}

void resizeDown(HashTable *ht, int newSize)
{

    std::cout << "resizing Down    ";

    ht->tables[1].size = newSize;

    /*
        Bug:
            //->> the following line causes the memory of ht->tables[1].table to be unreadable

            I tried freeing the memory of ht->tables[1].table before malloc, but it didnt work.
            free(ht->tables[1].table);
    */
    ht->tables[1].table = (HNode **)realloc(ht->tables[1].table, sizeof(HNode *) * newSize);

    ht->tables[1].taken = 0;

    std::cout << "Done" << std::endl;
}

void migrate(HashTable *ht)
{

    if (!ht->resizing)
        return;

    HNode *node;

    std::string key;
    std::string value;

    int index;

    for (int i = 0; i < N; i++)
    {
        index = ht->resizeIndex;

        if (index >= ht->tables[0].size && ht->tables[0].taken != 0)
        {

            ht->resizeIndex = 0;
            index = 0;
        }
        else if (index >= ht->tables[0].size || ht->tables[0].taken == 0)
        {

            std::cout << "finished migration" << std::endl;

            ht->resizeIndex = 0;

            ht->resizing = false;

            // free(&ht->tables[0]);

            ht->tables[0] = ht->tables[1];

            ht->tables[1] = initDict(ht->tables[0].size);

            ht->size = ht->tables[0].size;

            ht->taken = ht->tables[0].taken;

            return;
        }

        node = ht->tables[0].table[index];

        ht->resizeIndex++;

        if (node == NULL)
            continue;

        key = node->key;

        value = node->value;

        _set_helper(&ht->tables[1], key, value);

        ht->tables[0].table[index] = node->next;

        // free(node);

        ht->tables[0].taken--;
    }
}

std::string get(HashTable *ht, std::string key)
{

    double loadFactor = (double)ht->taken / (double)ht->size;

    if (!ht->resizing && loadFactor >= 1.0)
    {

        ht->resizing = true;

        int newSize = nextPrime(ht->size * 2);

        resizeUp(ht, newSize);
    }

    if (!ht->resizing && loadFactor <= 0.25)
    {

        ht->resizing = true;

        int newSize = nextPrime(ht->size / 2);

        resizeDown(ht, newSize);
    }

    migrate(ht);

    std::string value = _getHelper(&ht->tables[0], key);

    if (value == "")
    {

        value = _getHelper(&ht->tables[1], key);
    }

    return value;
}

void set(HashTable *ht, std::string key, std::string value)
{

    double loadFactor = (double)ht->taken / (double)ht->size;

    if (!ht->resizing && loadFactor >= 1.0)
    {

        ht->resizing = true;

        int newSize = nextPrime(ht->size * 2);

        resizeUp(ht, newSize);
    }

    if (!ht->resizing && loadFactor <= 0.25)
    {

        int newSize = nextPrime(ht->size / 2);
        if (newSize < ht->size)
        {
            ht->resizing = true;
            resizeDown(ht, newSize);
        } // handles corner case: ht->size = 2
    }

    migrate(ht);

    if (ht->resizing)

        _set_helper(&ht->tables[1], key, value);

    else

        _set_helper(&ht->tables[0], key, value);

    ht->taken = ht->tables[1].taken + ht->tables[0].taken;
}

void printHashTable(HashTable *ht)
{

    std::cout << "dict 0 " << ht->tables[0].size << std::endl;
    HNode *node;

    for (int i = 0; i < ht->tables[0].size; i++)
    {

        node = ht->tables[0].table[i];

        while (node != NULL)
        {

            std::cout << "(" << node->key << ": " << node->value << ") -> ";

            node = node->next;
        }

        std::cout << std::endl;
    }

    std::cout << std::endl
              << "dict 1 " << ht->tables[1].size << std::endl;

    for (int i = 0; i < ht->tables[1].size; i++)
    {

        node = ht->tables[1].table[i];

        while (node != NULL)
        {

            std::cout << "(" << node->key << ": " << node->value << ") -> ";

            node = node->next;
        }

        std::cout << std::endl;
    }
}

bool contains(HashTable *ht, std::string key)
{

    return get(ht, key) != "";
}

void _remove_helper(dict *dict, std::string key)
{

    int index = hash(key, dict->size);

    HNode *node = dict->table[index];

    HNode *prev = NULL;

    while (node != NULL)
    {

        if (node->key == key)
        {

            if (prev == NULL)
            {

                dict->table[index] = node->next;
            }
            else
            {

                prev->next = node->next;
            }

            free(node);

            return;
        }

        prev = node;

        node = node->next;
    }

    dict->taken--;
}

void remove(HashTable *ht, std::string key)
{

    _remove_helper(&ht->tables[0], key);

    _remove_helper(&ht->tables[1], key);

    ht->taken = ht->tables[1].taken + ht->tables[0].taken;
}

/* int main(){



    int size = 4;



    HashTable* ht = initHashTable(size);

    std::string key = "amr";
    std::string value = "hegazy";

    set(ht,key,value);

    key = "zeyad";
    value = "atteya";

    set(ht,key,value);

    key = "ahmed";
    value = "mohamed";

    set(ht,key,value);

    key = "omar";
    value = "saqr";

    set(ht,key,value);

    key = "youssef";
    value = "magdy";

    set(ht,key,value);

    key = "karim";
    value = "singer";

    set(ht,key,value);

    // std::cout << "hash " << hash("youssef",11) << std::endl;


    // char* value = get(ht,"amr");

    // std::cout << value << std::endl;

    printHashTable(ht);


    std::cout << "taken " << ht->taken << std::endl;
    std::cout << "size " << ht->size << std::endl;


    return 0;
} */
