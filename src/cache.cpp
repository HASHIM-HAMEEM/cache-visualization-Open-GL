#include "cache.h"

Cache::Cache(int s, ReplacementPolicy p) : size(s), policy(p), state(s, -1), hits(0), accesses(0) {}

void Cache::access(int address) {
    ++accesses;

    auto it = std::find(state.begin(), state.end(), address);
    if (it != state.end()) {
        ++hits;
        if (policy == ReplacementPolicy::LRU) {
            updateAccessOrder(address);
        }
    } else {
        if (state.size() >= size) {
            evict();
        }
        state.push_back(address);
        if (policy == ReplacementPolicy::LRU || policy == ReplacementPolicy::FIFO) {
            accessOrder.push_front(address);
            addressMap[address] = accessOrder.begin();
        }
    }
}

void Cache::evict() {
    int evictAddress;
    if (policy == ReplacementPolicy::LRU || policy == ReplacementPolicy::FIFO) {
        evictAddress = accessOrder.back();
        accessOrder.pop_back();
        addressMap.erase(evictAddress);
    }

    auto it = std::find(state.begin(), state.end(), evictAddress);
    if (it != state.end()) {
        state.erase(it);
    }
}

void Cache::updateAccessOrder(int address) {
    accessOrder.erase(addressMap[address]);
    accessOrder.push_front(address);
    addressMap[address] = accessOrder.begin();
}

const std::vector<int>& Cache::getState() const {
    return state;
}

float Cache::getHitRate() const {
    return accesses > 0 ? static_cast<float>(hits) / accesses : 0.0f;
}
