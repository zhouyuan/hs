/*************************************************
Copyright: Intel
Author: http://timday.bitbucket.org/lru.html modified by Chendi.Xue
Date:2016/Mar/07
Description:LRU class define
**************************************************/

#ifndef LRU_LIST_H
#define LRU_LIST_H

#include <cassert>
#include <list>
#include <map>
#include <mutex>
#include <algorithm>

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

template < typename K > class LRU_LIST{
public:

    typedef K key_type;

    // Key access history, most recent at back
    typedef std::list<key_type> key_tracker_type;

    // Constuctor specifies the cached function and
    // the maximum number of records to be stored
    LRU_LIST(
        size_t c
    ): _capacity(c)
    {
      assert(_capacity!=0);
    }

    // Obtain the cached keys, most recently used element
    // at head, least recently used at tail.
    // This method is provided purely to support testing.
    void get_keys(key_type* dst, uint64_t length = 0, bool most_recent_top = true){
        uint64_t count = 0;
        _lock.lock();
        if(most_recent_top){
            typename key_tracker_type::const_reverse_iterator src = _key_tracker.rbegin();
            while (src!=_key_tracker.rend()) {
                *dst++ = *src++;
                count++;
                if( count == length ){
                    _lock.unlock();
                    return;
                }
            }
        }else{
            typename key_tracker_type::const_iterator src = _key_tracker.begin();
            while (src!=_key_tracker.end()) {
                *dst++ = *src++;
                count++;
                if( count == length ){
                    _lock.unlock();
                    return;
                }
            }
        }
        _lock.unlock();
        return;
    }

    // Obtain value of the cached function for k
    key_type touch_key(const key_type& k) {

      _lock.lock();
      // Attempt to find existing record
      const typename key_tracker_type::iterator it = std::find(_key_tracker.begin(),_key_tracker.end(), k );

      if (it==_key_tracker.end()) {

        // We don't have it:
        insert(k);

        _lock.unlock();
        // Return the freshly computed value
        return k;

      } else {

        // We do have it:

        // Update access record by moving
        // accessed key to back of list
        _key_tracker.splice( _key_tracker.end(), _key_tracker, it );

        _lock.unlock();
        // Return the retrieved value
        return *it;
      }
    }

    void remove( const key_type& k ){
        _lock.lock();
        if(std::find(_key_tracker.begin(), _key_tracker.end(), k ) !=_key_tracker.end())
            _key_tracker.remove(k);
        _lock.unlock();
    }

    // Record a fresh key-value pair in the cache
    void insert(const key_type& k) {
      // Method is only called on cache misses
      assert( std::find(_key_tracker.begin(), _key_tracker.end(), k ) ==_key_tracker.end() );

      // Make space if necessary
      if (_key_tracker.size()==_capacity){
          evict();
      }

      // Record k as most-recently-used key
      typename key_tracker_type::iterator it = _key_tracker.insert(_key_tracker.end(),k);
    }

    // Purge the least-recently-used element in the cache
    void evict() {
      // Assert method is never called when cache is empty
      assert(!_key_tracker.empty());

      // Erase both elements to completely purge record
      _key_tracker.pop_front();
    }

    uint64_t get_length(){
        return _key_tracker.size();
    }
private:

    // The function to be cached
    key_type (*_fn)(const key_type&);
    // Maximum number of key-value pairs to be retained
    const size_t _capacity;
    // mutex lock of LRU list
    std::mutex _lock;
    // Key access history
    key_tracker_type _key_tracker;
};

}
#endif
