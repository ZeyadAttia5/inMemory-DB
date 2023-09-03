#include "heap.hpp"
#include <iostream>

/* Heap_Node */
uint64_t Heap_Node::get_ttl()
{
    return this->TTL;
}
std::string Heap_Node::get_Hkey()
{
    return this->Hkey;
}
std::string Heap_Node::get_Aname()
{
    return this->Aname;
}
int Heap_Node::get_Akey()
{
    return this->Akey;
}

bool Heap_Node::get_type(){
    return this->type;
}

uint64_t Heap_Node::get_monotonic_usec()
{
	timespec tv = {0, 0};
	clock_gettime(CLOCK_MONOTONIC, &tv);
	return uint64_t(tv.tv_sec) * 1000000 + tv.tv_nsec / 1000;
}

/* Heap_Node */

int64_t Heap::getParentIndex(size_t i)
{
    if (i == 0)
    {
        return -1;
    }
    int64_t idx = (i - 1) / 2;
    return idx;
}
int64_t Heap::getLeftIndex(size_t i) { return i * 2 + 1; }
int64_t Heap::getRightIndex(size_t i) { return i * 2 + 2; }

bool Heap::hasParent(size_t i)
{
    bool parentStat = getParentIndex(i) >= 0;
    return parentStat;
}
bool Heap::hasLeft(size_t i) { return getLeftIndex(i) < heap.size(); }
bool Heap::hasRight(size_t i) { return getRightIndex(i) < heap.size(); }

uint64_t Heap::getParent(size_t i) { return (hasParent(i)) ? heap[getParentIndex(i)].get_ttl() : 0; }
uint64_t Heap::getLeft(size_t i) { return (hasLeft(i)) ? heap[getLeftIndex(i)].get_ttl() : 0; }
uint64_t Heap::getRight(size_t i) { return (hasRight(i)) ? heap[getRightIndex(i)].get_ttl() : 0; }

bool Heap::isEmpty() { return heap.size() == 0; }
size_t Heap::size() { return heap.size(); }

// swap two elements in the heap
void Heap::swap(size_t i, size_t j)
{
    if (i < heap.size() && j < heap.size())
    {
        std::swap(heap[i], heap[j]);
    }
}

// peek at the top of the heap (the smallest element)
uint64_t Heap::peek()
{
    if (heap.size() == 0)
    {
        std::cerr << "Heap is empty." << std::endl;
        return -1;
    }
    return heap[0].get_ttl();
}

// poll the top of the heap (the smallest element)
Heap_Node Heap::poll()
{
    if (heap.size() == 0)
    {
        std::cerr << "Heap is empty." << std::endl;
    }
    else
    {

        Heap_Node item = heap[0];
        heap[0] = heap[heap.size() - 1];
        heap.pop_back();
        heapifyDown();
        return item;
    }
}

// add an element to the heap
void Heap::add(uint64_t value, std::string Aname, int Akey)
{
    Heap_Node *node = new Heap_Node(value, Aname, Akey);
    heap.push_back(*node);
    heapifyUp();
}
void Heap::add(uint64_t value, std::string Hkey)
{
    Heap_Node *node = new Heap_Node(value, Hkey);
    heap.push_back(*node);
    heapifyUp();
}

// heapify up
void Heap::heapifyUp()
{
    size_t index;
    if (this->isEmpty())
    {
        index = 0;
    }
    else
    {

        index = heap.size() - 1;
    }
    while (hasParent(index) && getParent(index) > heap[index].get_ttl())
    {
        swap(getParentIndex(index), index);
        index = getParentIndex(index);
    }
}

// heapify down
void Heap::heapifyDown()
{
    size_t index = 0;
    while (hasLeft(index))
    {
        size_t smallerChildIndex = getLeftIndex(index);
        if (hasRight(index) && getRight(index) < getLeft(index))
        {
            smallerChildIndex = getRightIndex(index);
        }
        if (heap[index].get_ttl() < heap[smallerChildIndex].get_ttl())
        {
            break;
        }
        else
        {
            swap(index, smallerChildIndex);
        }
        index = smallerChildIndex;
    }
}

// print the heap
void Heap::heap_print()
{
    for (size_t i = 0; i < heap.size(); i++)
    {
        if (hasLeft(i))
        {
            std::cout << " PARENT : " << heap[i].get_ttl() << " LEFT CHILD : " << heap[getLeftIndex(i)].get_ttl();
        }
        if (hasRight(i))
        {
            std::cout << " RIGHT CHILD :" << heap[getRightIndex(i)].get_ttl() << std::endl;
        }
        else
        {
            std::cout << std::endl;
        }
    }
    std::cout << std::endl;
}

int main()
{
    Heap *heap = new Heap();

    heap->add(5, "ahmed");
    heap->add(3, "ahed");
    heap->add(4, "amed");
    heap->add(1, "ahme");
    heap->add(2, "ahmed");
    heap->add(0, "hed");
    std::cout << heap->peek() << std::endl;

    heap->heap_print();

    // tests heapifyDown/poll
    for (size_t i = 0; i < 6; i++)
    {
        // std::cout << heap->peek() << std::endl;
        std::cout << heap->poll().get_ttl() << std::endl;
    }
}