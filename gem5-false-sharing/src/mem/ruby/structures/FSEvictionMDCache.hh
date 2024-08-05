#ifndef __MEM_RUBY_STRUCTURES_FS_EVICTION_MD_CACHE_HH__
#define __MEM_RUBY_STRUCTURES_FS_EVICTION_MD_CACHE_HH__

#include <unordered_map>

#include "base/logging.hh"
#include "base/statistics.hh"
#include "base/types.hh"
#include "debug/OptionalMetadata.hh"
#include "mem/ruby/common/Address.hh"
#include "params/FSEvictionMDCache.hh"
#include "sim/sim_object.hh"

class FSEvictionMDCache : public SimObject
{
  private:
  public:
    std::unordered_map<Addr, bool> evictionMDEntryMap;
    // FalseSharing: a vector would suffice,
    // if entry exist a Eviction MD Ack is yet to arrive
    std::vector<Addr> evictionMDEntry;
    uint32_t assoc_emd;
    uint32_t size_emd;
    uint32_t set_emd;     // number of set in the own access metadata table
    int offset_bit_index; // bits to address bytes of a block, UNUSED
    int set_index_bits;   // FalseSharing: bits to identify a set, UNUSED
    int maxSize  =1024;
    FSEvictionMDCache(FSEvictionMDCacheParams* params);

    ~FSEvictionMDCache();

    void addEvictionMDEntry(Addr addr);

    void removeEvictionMDEntry(Addr addr);

    bool isMDEntryPresent(Addr addr);

    // FalseSharing: stat might come handy later
    //void regStats();
    //Stats::Scalar m_mdAck_read;
    //Stats::Scalar m_mdAck_write;
};

#endif //__MEM_RUBY_STRUCTURES_FS_EVICTION_MD_CACHE_HH__
