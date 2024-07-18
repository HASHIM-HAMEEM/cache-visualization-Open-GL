#pragma once
#include <vector>
#include <list>
#include <unordered_map>

class Cache {
public:
    enum class ReplacementPolicy {
        LRU,
        FIFO
    };

    Cache(int size, ReplacementPolicy policy);

    void access(int address);
    const std::vector<int>& getState() const;
    float getHitRate() const;

private:
    int size;
    ReplacementPolicy policy;
    std::vector<int> state;
    std::unordered_map<int, std::list<int>::iterator> addressMap;
    std::list<int> accessOrder;
    int hits;
    int accesses;

    void evict();
    void updateAccessOrder(int address);
};

