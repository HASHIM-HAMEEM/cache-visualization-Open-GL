#include <iostream>
#include "renderer.h"
#include "cache.h"

int main() {
    int cacheSize = 10;
    Cache::ReplacementPolicy policy = Cache::ReplacementPolicy::LRU;
    Cache cache(cacheSize, policy);
    Renderer renderer(800, 600);

    std::vector<int> memoryAccesses = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 11, 12, 13, 14, 15, 3, 4, 5};

    while (!renderer.shouldClose()) {
        for (int address : memoryAccesses) {
            cache.access(address);
            renderer.render(cache);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));  // Simulate time delay between accesses
        }
    }

    return 0;
}
