#include "heap.hpp"
#include <iostream>

size_t Heap::heap_getParentIndex(size_t i)  { return (i + 1) / 2 - 1; }
size_t Heap::heap_getLeftIndex(size_t i)    { return i * 2 + 1; }
size_t Heap::heap_getRightIndex(size_t i)   { return i * 2 + 2; }

bool Heap::heap_hasParent(size_t i) { return heap_getParentIndex(i) >= 0; }
bool Heap::heap_hasLeft(size_t i)   { return heap_getLeftIndex(i) >= 0; }
bool Heap::heap_hasRight(size_t i)  { return heap_getRightIndex(i) >= 0; }

uint64_t Heap::heap_getParent(size_t i) { return (heap_hasParent(i)) ? heap[heap_getParentIndex(i)] : 0; }
uint64_t Heap::heap_getLeft(size_t i)   { return (heap_hasLeft) ? heap[heap_getLeftIndex(i)] : 0; }
uint64_t Heap::heap_getRight(size_t i)  { return (heap_hasRight) ? heap[heap_getRightIndex(i)] : 0; }

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

// peek at the top of the heap (the smallest element)
uint64_t Heap::peek()
{
    if (heap.size() == 0)
    {
        std::cerr << "Heap is empty." << std::endl;
        return -1;
    }
    return heap[0];
}

// poll the top of the heap (the smallest element)
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
    heap_heapifyDown();
    return item;
}

// add an element to the heap
void Heap::add(uint64_t value)
{
    heap.push_back(value);
    heap_heapifyUp();
}

// heapify up
void Heap::heap_heapifyUp()
{
    size_t index = heap.size() - 1;
    while (heap_hasParent(index) && heap_getParentIndex(index) > heap[index])
    {
        swap(heap_getParentIndex(index), index);
        index = heap_getParentIndex(index);
    }
}

// heapify down
void Heap::heap_heapifyDown()
{
    size_t index = 0;
    while (heap_hasLeft(index))
    {
        size_t smallerChildIndex = heap_getLeftIndex(index);
        if (heap_hasRight(index) && heap_getRightIndex(index) < heap_getLeftIndex(index))
        {
            smallerChildIndex = heap_getRightIndex(index);
        }
        if (heap[index] < heap[smallerChildIndex])
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