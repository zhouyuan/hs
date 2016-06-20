#include "rbc/CacheEntry.h"


namespace rbc {
CacheEntry::CacheEntry(uint64_t object_size){
    init( object_size );
}

CacheEntry::~CacheEntry(){
    delete inflight_flush_ops;
}

void CacheEntry::init( uint64_t object_size ){
    this->object_size = object_size;
    history_count = 0;
    if_dirty = false;
    if_fully_cached = false;
    new_update_after_flush = false;
    inflight_flush_ops = new List( object_size/4096 );
}

bool CacheEntry::data_map_lookup( uint64_t offset, uint64_t length ){
    if( this->data_map.empty() ){
        return false;
    }
    EntryMap::iterator it = this->data_map.lower_bound(offset);
    it = it-- ;
    if( it->first <= offset && (it->second + it->first) >= (offset + length) )
        return true;
    else
        return false;
}

void CacheEntry::data_map_update( uint64_t offset, uint64_t length ){
    // return the biggest addr <= offset
    history_count++;
    std::map<uint64_t, uint64_t>::iterator it_lower = data_map.lower_bound(offset);
    std::map<uint64_t, uint64_t>::iterator it_upper = data_map.lower_bound(offset+length);
    uint64_t tmp;

    if( data_map.empty() ){
        data_map[ offset ] = length;
        //printf("[empty]%s data_map offset:%lu, length:%lu\n", name.c_str(), offset, length);
    }else{
        if(it_lower == data_map.end() || it_lower->first > offset) it_lower--;
        if(it_upper == data_map.end() || it_upper->first > (offset+length)) it_upper--;
        //1)left overlap, 2)non-overlap, 3)totally overlap, 4)right overlap
        if( it_lower == data_map.end() ){
            if( it_upper != data_map.end() ){
                if( it_upper->first + it_upper->second >= (offset + length) ){
                    tmp = it_upper->first - offset + it_upper->second ;
                    data_map[offset] = tmp;
                    data_map.erase( it_upper );
                    //printf("[update]size: %lu, %s data_map offset:%lu, length:%lu, new offset: %lu\n", data_map.size(), name.c_str(), offset, tmp, offset);
                }else{
                    data_map[offset] = length;
                    data_map.erase( it_upper );
                    //printf("[update]size: %lu, %s data_map offset:%lu, length:%lu, new offset: %lu\n", data_map.size(), name.c_str(), offset, length, offset);
                }
            }else{
                data_map[offset] = length;
                //printf("[insert]size: %lu, %s data_map offset:%lu, length:%lu, it->offset:%lu\n", data_map.size(), name.c_str(), offset, length, it_upper->first);
            }
        }else if( it_lower == it_upper ){
            std::map<uint64_t, uint64_t>::iterator it = it_lower;
            if( ((it->first + it->second) >= offset) && ((it->first + it->second) <= (offset + length)) ){
                it->second = offset - it->first + length;
                //printf("[update]size: %lu, %s data_map offset:%lu, length:%lu, new offset: %lu\n", data_map.size(), name.c_str(), it->first, it->second, offset);
            }else if( it->first + it->second < offset ){
                data_map[ offset ] = length;
                //printf("[insert]size: %lu, %s data_map offset:%lu, length:%lu, <it->first, it->second>:%lu, %lu\n", data_map.size(), name.c_str(), offset, length, it->first, it->second);
            }
        }else{
        //it_lower != it_upper
            if( it_lower->first+it_lower->second < offset ){
                if( it_upper->first + it_upper->second >= (offset + length) ){
                    tmp = it_upper->first - offset + it_upper->second ;
                    data_map[offset] = tmp;
                    data_map.erase( it_upper );
                    //printf("[update]size:%lu, %s data_map offset:%lu, length:%lu, new offset: %lu\n", data_map.size(), name.c_str(), offset, tmp, offset);
                }else{
                    data_map[offset] = length;
                    data_map.erase( it_upper );
                    //printf("[update]size:%lu, %s data_map offset:%lu, length:%lu, new offset: %lu\n", data_map.size(), name.c_str(), offset, length, offset);
                }
            }else{
                if( offset + length <= (it_upper->first + it_upper->second) ){
                    it_lower->second = it_upper->first + it_upper->second - it_lower->first;
                }else{
                    it_lower->second = offset + length - it_lower->first;
                }
                data_map.erase(it_upper);
                //printf("[update]size:%lu, %s data_map offset:%lu, length:%lu, new offset: %lu\n",data_map.size(), name.c_str(), it_lower->first, it_lower->second, offset);
            }
        }
    }
    if(data_map.size() == 1){
        std::map<uint64_t, uint64_t>::iterator it_begin = data_map.begin();
        if(it_begin->first == 0 && it_begin->second == object_size)
            if_fully_cached = true;
    }
}

uint64_t CacheEntry::get_size( uint64_t &bl_count ){
    uint64_t size = 0;
    bl_count = 0;
    for(EntryMap::iterator it = this->data_map.begin(); it!=this->data_map.end(); it++){
        size += it->second;
        bl_count++;
    }
    return size;
}

uint64_t CacheEntry::get_offset(){
    entry_lock.lock();
    uint64_t offset = this->offset;
    entry_lock.unlock();
    return offset;
}

EntryMap CacheEntry::get_data_map(){
    return data_map;
}
}
