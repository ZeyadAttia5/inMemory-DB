#include "heap.hpp"
#include <iostream>

size_t Heap::heap_getParentIndex(size_t i)
{
    return (i + 1) / 2 - 1;
}

size_t Heap::heap_getLeftIndex(size_t i)
{
    return i * 2 + 1;
}

size_t Heap::heap_getRightIndex(size_t i)
{
    return i * 2 + 2;
}

bool Heap::heap_hasParent(size_t i)
{
    return heap_getParentIndex(i) >= 0;
}
bool Heap::heap_hasLeft(size_t i)
{
    return heap_getLeftIndex(i) >= 0;
}
bool Heap::heap_hasRight(size_t i)
{
    return heap_getRightIndex(i) >= 0;
}

uint64_t Heap::heap_getParentIndex(size_t i)
{
    return heap[heap_getParentIndex(i)];
}
uint64_t Heap::heap_getLeftIndex(size_t i)
{
    return heap[heap_getLeftIndex(i)];
}
uint64_t Heap::heap_getRightIndex(size_t i)
{
    return heap[heap_getRightIndex(i)];
}

void Heap::swap(size_t i, size_t j)
{
    if (i < heap.size() && j < heap.size())
    {
        std::swap(heap[i], heap[j]);
    }
    else
    {
        std::cerr << "Index out of range." << std::endl;
    }
}

//peek at the top of the heap (the smallest element)
uint64_t Heap::peek()
{
    if (heap.size() == 0)
    {
        std::cerr << "Heap is empty." << std::endl;
        return -1;
    }
    return heap[0];
}

//poll the top of the heap (the smallest element)
uint64_t Heap::poll()
{
    if (heap.size() == 0)
    {
        std::cerr << "Heap is empty." << std::endl;
        return -1;
    }
    uint64_t item = heap[0];
    heap[0] = heap[heap.size() - 1];
    heap.pop_back();
    heapifyDown();
    return item;
}

//add an element to the heap
void Heap::add(uint64_t value){
    heap.push_back(value);
    heapifyUp();
}