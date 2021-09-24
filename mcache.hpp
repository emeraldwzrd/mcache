#pragma once
#include <vector>
#include <functional>
#include <utility>
#include <queue>
#include <chrono>
#include <thread>
#include <mutex>

using namespace std;

template <typename KeyType, typename ValueType>
class MCache {
private:
  struct KeyListType {
    pair<KeyType*, ValueType*> entry;
    KeyListType *next;
  };
  struct FindHelper {
    KeyListType* keyListNode;
    unsigned int index;
    bool found;
  };
  const unsigned int MAX_SIZE = 10000000;
  const pair<KeyType*, ValueType*> initializer;
  unsigned int size;
  unsigned int capacity;
  vector<KeyListType*> mmap;
  queue<pair<long, KeyType> > timedQueue;
  thread timedCacheT;
  mutable mutex mutexL;

  FindHelper findKeyValue(KeyType k, bool insert) {
    size_t targetKeyHash = hash<KeyType>()(k);
    unsigned int index = targetKeyHash % this->capacity;
    KeyListType* currEntry = this->mmap[index];
    FindHelper fh = {nullptr, 0, false};
    // Loop until we find the key or we've reached the end of the list
    while(currEntry != nullptr) {
      size_t foundKeyHash = hash<KeyType>()(*currEntry->entry.first);
      if(foundKeyHash == targetKeyHash) {
        fh.found = true;
        break;
      }
      // If we have reached the end of the list and we want to insert we
      // need to keep the current node to add to the list, so break
      if(currEntry->next == nullptr && insert) {
        break;
      }
      currEntry = currEntry->next;
    }
    fh.keyListNode = currEntry;
    fh.index = index;
    return fh;
  }

  bool insertToKeyList(KeyType k, ValueType v) {
    FindHelper kv = findKeyValue(k, true);
    if(kv.found) {
      return false;
    }
    KeyListType* newEntry = new KeyListType({{new KeyType(k), new ValueType(v)}, nullptr});
    if(kv.keyListNode == nullptr) {
      this->mmap[kv.index] = newEntry;
    } else {
      kv.keyListNode->next = newEntry;
    }
    this->size++;
    return true;
  }

  void removeKeyList(KeyListType* kl) {
    if(kl != nullptr)  {
      if(kl->entry.first != nullptr) {
        free(kl->entry.first);
      }
      if(kl->entry.second != nullptr) {
        free(kl->entry.second);
      }
      free(kl);
    }
  }

  bool removeFromKeyList(KeyType k) {
    size_t targetKeyHash = hash<KeyType>()(k);
    unsigned int index = targetKeyHash % this->capacity;
    if(this->mmap[index] == nullptr) {
      return false;
    }
    KeyListType* currEntry = this->mmap[index];
    KeyListType* prevEntry = nullptr;
    while(currEntry != nullptr) {
      size_t foundKeyHash = hash<KeyType>()(*(currEntry->entry.first));
      if(foundKeyHash == targetKeyHash) {
        if(prevEntry == nullptr) {
          this->mmap[index] = nullptr;
        } else {
          prevEntry->next = currEntry->next;
        }
        removeKeyList(currEntry);
        this->size--;
        return true;
      }
      prevEntry = currEntry;
      currEntry = currEntry->next;
    }
    return false;
  }

  void manageTimedCache() {
    while(true) {
      auto now = chrono::duration_cast<chrono::seconds>(
                          chrono::system_clock::now().time_since_epoch()).count();
      // Loop through all entries that had an expiration time before now and remove them
      while(!this->timedQueue.empty() && 
             this->timedQueue.front().first <= now) {
        removeFromKeyList(this->timedQueue.front().second);
        this->timedQueue.pop();
      }
      this_thread::sleep_for(chrono::seconds(1));
    }
  }

  void setTimedCache(unsigned int seconds, KeyType k) {
     auto now = chrono::duration_cast<chrono::seconds>(
                           chrono::system_clock::now().time_since_epoch()).count();
     auto future = now + seconds;
     this->timedQueue.push({future, k});
  }

public:
  MCache() : initializer(nullptr, nullptr) {
     this->timedCacheT = thread(&MCache::manageTimedCache, this);
     this->capacity = MAX_SIZE;
     this->size = 0;
     this->mmap.resize(capacity, nullptr);
  }

  bool insert(KeyType k, ValueType v, unsigned int seconds) {
     unique_lock lock(this->mutexL);
     insertToKeyList(k, v);
     if(seconds > 0) {
        setTimedCache(seconds, k);
     }
     lock.unlock();
     return true;
  }

  bool insert(KeyType k, ValueType v) {
    if(this->size == this->capacity) {
      return false;
    }
    return insert(k, v, 0);
  }
  
  ValueType* get(KeyType k) {
    FindHelper kv = findKeyValue(k, false);
    if(!kv.found) {
      return nullptr;
    }
    return new ValueType(*kv.keyListNode->entry.second);
  }
  
  bool remove(KeyType k) {
    unique_lock lock(this->mutexL);
    removeFromKeyList(k);
    lock.unlock();
    return true;
  }

  unsigned int getSize() {
    return this->size;
  }

  unsigned int getTimedQueueSize() {
    return this->timedQueue.size();
  }
};

