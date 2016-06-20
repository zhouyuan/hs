#include <cassert>
#ifndef CACHEMAP_H
#define CACHEMAP_H

namespace rbc {
template < typename K, typename V > class CacheMap{
public:
    typedef K key_type;
    typedef V value_type;

    typedef std::map< key_type, value_type > key_to_value_type;

    CacheMap( value_type (*f)(const key_type&) ): _fn(f){}

    // Record a fresh key-value pair in the cache
    void insert(const key_type& k,const value_type& v) {
      // Method is only called on cache misses
      assert(_key_to_value.find(k)==_key_to_value.end());

      // Create the key-value entry,
      // linked to the usage record.
      _key_to_value.insert(
        std::make_pair(k, v)
      );
    }

    void evict(const key_type& k) {
      // Identify least recently used key
      _lock.lock();
      const typename key_to_value_type::iterator it =_key_to_value.find(k);
      if(it!=_key_to_value.end()){
          _key_to_value.erase(it);
      }
      _lock.unlock();
    }

    void get_values( value_type* dst ){
        _lock.lock();
        typename key_to_value_type::iterator it = _key_to_value.begin();
        for(; it != _key_to_value.end(); it++){
            *dst++ = it->second;
        }
        _lock.unlock();
    }

    ssize_t size(){
        _lock.lock();
        ssize_t size = _key_to_value.size();
        _lock.unlock();
        return size;
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

        _lock.unlock();
        // Return the retrieved value
        return (*it).second;
      }
    }
private:

    // The function to be cached
    value_type (*_fn)(const key_type&);
    // mutex lock of LRU list
    std::mutex _lock;
    // Key-to-value lookup
    key_to_value_type _key_to_value;
};
}
#endif
