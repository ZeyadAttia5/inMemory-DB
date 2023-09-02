
#include <stddef.h>
#include <vector>
#include <stdint.h>

class Heap
{
private:
    // heap is a vector of uint64_t which are timers :)
    std::vector<uint64_t> heap;

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
    uint64_t poll();
    void add(uint64_t value);
    void heap_print();
    bool isEmpty();
    size_t size();
    Heap()
    {
        // Initialize the heap as an empty vector
        std::vector<uint64_t> emptyHeap;
        heap = emptyHeap;
    }
};
