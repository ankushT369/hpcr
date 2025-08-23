// thread-cache.hpp (replace existing VecCache)
#include <vector>
#include <memory>
#include <stdexcept>


template<typename T>
class VecCache {
private:
  std::vector<std::shared_ptr<T>> threadCache;
public:
  void push(std::shared_ptr<T> value) {
    threadCache.push_back(std::move(value));
  }

  std::shared_ptr<T>& back() {
    if (threadCache.empty()) throw std::runtime_error("Cache is empty");
    return threadCache.back();
  }

  const std::shared_ptr<T>& back() const {
    if (threadCache.empty()) throw std::runtime_error("Cache is empty");
    return threadCache.back();
  }

  bool empty() const { return threadCache.empty(); }
  void clean() { threadCache.clear(); }
  size_t size() const { return threadCache.size(); }

  auto begin() { return threadCache.begin(); }
  auto end() { return threadCache.end(); }
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



