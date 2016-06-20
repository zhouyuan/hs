/*************************************************
Copyright: Intel
Author: http://timday.bitbucket.org/lru.html modified by Chendi.Xue
Date:2016/Mar/07
Description:LRU class define
**************************************************/

#ifndef LRU_MAP_H
#define LRU_MAP_H

#include <cassert>
#include <list>
#include <map>
#include <mutex>

// Class providing fixed-size (by number of records)
// LRU-replacement cache of a function with signature
// V f(K).
// MAP should be one of std::map or std::unordered_map.
// Variadic template args used to deal with the
// different type argument signatures of those
// containers; the default comparator/hash/allocator
// will be used.
// when doing specification, should be LRU<int, int> myLRU_list( C_Entry, list_size );

namespace rbc {
template < typename K, typename V > class LRU_MAP{
public:

    typedef K key_type;
    typedef V value_type;

    // Key access history, most recent at back
    typedef std::list<key_type> key_tracker_type;

    // Key to value and key history iterator
    typedef std::map<
      key_type,
      std::pair<
        value_type,
        typename key_tracker_type::iterator
        >
    > key_to_value_type;

    // Constuctor specifies the cached function and
    // the maximum number of records to be stored
    LRU_MAP(
        value_type (*f)(const key_type&),
        size_t c
    ): _fn(f), _capacity(c)
    {
      assert(_capacity!=0);
    }

    // Obtain the cached keys, most recently used element
    // at head, least recently used at tail.
    // This method is provided purely to support testing.
    void get_keys(key_type* dst){
        typename key_tracker_type::const_reverse_iterator src = _key_tracker.rbegin();
        while (src!=_key_tracker.rend()) {
            *dst++ = *src++;
        }
        return;
    }

    // Obtain value of the cached function for k
    value_type get_value(const key_type& k) {

      _lock.lock();
      // Attempt to find existing record
      const typename key_to_value_type::iterator it =_key_to_value.find(k);

      if (it==_key_to_value.end()) {

        // We don't have it:

        // Evaluate function(in our case, the funtion is create new C_Entry)
        const value_type v=_fn(k);
        insert(k,v);

        _lock.unlock();
        // Return the freshly computed value
        return v;

      } else {

        // We do have it:

        // Update access record by moving
        // accessed key to back of list
        _key_tracker.splice( _key_tracker.end(), _key_tracker, (*it).second.second );

        _lock.unlock();
        // Return the retrieved value
        return (*it).second.first;
      }
    }

    // Record a fresh key-value pair in the cache
    void insert(const key_type& k,const value_type& v) {
      // Method is only called on cache misses
      assert(_key_to_value.find(k)==_key_to_value.end());

      // Make space if necessary
      if (_key_to_value.size()==_capacity){
          evict();
      }

      // Record k as most-recently-used key
      typename key_tracker_type::iterator it = _key_tracker.insert(_key_tracker.end(),k);
      // Create the key-value entry,
      // linked to the usage record.
      _key_to_value.insert(
        std::make_pair(k, std::make_pair(v,it))
      );
      // No need to check return,
      // given previous assert.
    }

    // Purge the least-recently-used element in the cache
    void evict() {
      // Assert method is never called when cache is empty
      assert(!_key_tracker.empty());

      // Identify least recently used key
      const typename key_to_value_type::iterator it =_key_to_value.find(_key_tracker.front());
      assert(it!=_key_to_value.end());

      // Erase both elements to completely purge record
      _key_to_value.erase(it);
      _key_tracker.pop_front();
    }
private:

    // The function to be cached
    value_type (*_fn)(const key_type&);
    // Maximum number of key-value pairs to be retained
    const size_t _capacity;
    // mutex lock of LRU list
    std::mutex _lock;
    // Key access history
    key_tracker_type _key_tracker;
    // Key-to-value lookup
    key_to_value_type _key_to_value;
};
}

#endif
