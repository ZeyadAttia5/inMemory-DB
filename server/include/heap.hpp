
#include <stddef.h>
#include <vector>
#include <stdint.h>
#include <string>

#ifndef HEAP_HPP
#define HEAP_HPP

class Heap_Node
{
private:
    uint64_t TTL;

    std::string Hkey;   //key of the hashtable node

    std::string Aname;  //name of the AVL tree
    int Akey;           //key of the AVL node
    bool type;          //true for AVL, false for hashtable

    uint64_t get_monotonic_usec();

public:
    Heap_Node(uint64_t TTL, std::string Aname, int Akey)   //constructor for AVL Node
    {
        this->TTL = get_monotonic_usec() + TTL * 1000;
        this->Aname = Aname;
        this->Akey = Akey;
        type = true;
    }
    Heap_Node(uint64_t TTL, std::string Hkey)        //constructor for hashtable Node
    {
        this->TTL = get_monotonic_usec() + TTL * 1000;
        this->Hkey = Hkey;
        type = false;
    }
    uint64_t get_ttl();
    std::string get_Hkey();
    std::string get_Aname();
    int get_Akey();
    bool get_type();
};


class Heap
{
private:
    std::vector<Heap_Node> heap;

    void swap(size_t i, size_t j);

    int64_t getParentIndex(size_t i);
    int64_t getLeftIndex(size_t i);
    int64_t getRightIndex(size_t i);

    bool hasParent(size_t i);
    bool hasLeft(size_t i);
    bool hasRight(size_t i);

    uint64_t getParent(size_t i);
    uint64_t getLeft(size_t i);
    uint64_t getRight(size_t i);

    void heapifyUp();
    void heapifyDown();

public:
    uint64_t peek();
    Heap_Node poll();
    void add(uint64_t value, std::string Hkey);
    void add(uint64_t value, std::string Aname, int Akey);
    void heap_print();
    bool isEmpty();
    size_t size();
    
    Heap()
    {
        // Initialize the heap as an empty vector
        std::vector<Heap_Node> emptyHeap;
        heap = emptyHeap;
    }
};


#endif