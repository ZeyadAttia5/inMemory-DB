
#include <stddef.h>
#include <vector>
#include <stdint.h>

class Heap
{
private:
    // heap is a vector of uint64_t which are timers :)
    std::vector<uint64_t> heap;
    void swap(size_t i, size_t j);

public:
    size_t heap_getParentIndex(size_t i);
    size_t heap_getLeftIndex(size_t i);
    size_t heap_getRightIndex(size_t i);

    bool heap_hasParent(size_t i);
    bool heap_hasLeft(size_t i);
    bool heap_hasRight(size_t i);

    uint64_t heap_getParentIndex(size_t i);
    uint64_t heap_getLeftIndex(size_t i);
    uint64_t heap_getRightIndex(size_t i);

    uint64_t peek();
    uint64_t poll();
    void add(uint64_t value);
};
