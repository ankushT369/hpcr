#include <thread>
#include <vector>
#include <memory>


template<typename T>
class VecCache {
private:
  std::vector<T> threadCache;
public:
  void push(const T& value) { threadCache.push_back(value); }

  void pop() { if (!threadCache.empty()) threadCache.pop_back(); }

  T back() {
    if (threadCache.empty()) throw std::runtime_error("Cache is empty");
    return threadCache.back();
  }

  T front() {
    if (threadCache.empty()) throw std::runtime_error("Cache is empty");
    return threadCache[0];
  }

  bool empty() const { return threadCache.empty(); }

  void clean() { threadCache.clear(); }

  size_t size() const { return threadCache.size(); }

};

template<typename T>
class LRUCache {

};

template<typename T>
class LFUCache {

};

template<typename T>
class CachePool {
private:
  std::vector<std::unique_ptr<VecCache<T>>> caches;
public:
  CachePool(size_t poolSize) {
    caches.reserve(poolSize);
    for(size_t i = 0; i < poolSize; i++) {
      caches.push_back(std::make_unique<VecCache<T>>());
    }
  }

  VecCache<T>* getCache(size_t index) {
    if(index >= caches.size()) return nullptr;
    return caches[index].get();
  }

  size_t size() const { return caches.size(); }

  auto begin() const { return caches.begin(); }
  auto end() const { return caches.end(); }
};
