/**
 * FSEvictionMDCache:
 * Implement field and methods for eviction MD
 */

#include "mem/ruby/structures/FSEvictionMDCache.hh"

#include "base/intmath.hh"
#include "base/logging.hh"

FSEvictionMDCache::FSEvictionMDCache(
    FSEvictionMDCacheParams* params)
    : SimObject(params), assoc_emd(params->assoc_emd),
     size_emd(params->size_emd),
     offset_bit_index(params->offset_bit_index)
{
    if (size_emd == 0 || assoc_emd == 0) {
        assoc_emd = 8;
        size_emd = 8;
        //evictionMDEntry.resize(size_emd);
        inform("A fully associative EvictonMD cache created with 8 size");
    } else {
        assert(isPowerOf2(size_emd));
        assert(isPowerOf2(assoc_emd));
        //evictionMDEntry.resize(size_emd);
        inform("A EvictionMD cache of size: %d, associativity: %d is created",
               size_emd, assoc_emd);
    }
    DPRINTF(OptionalMetadata, "A per core evictionMD cache created.\n");
}

FSEvictionMDCache::~FSEvictionMDCache() {}

bool
FSEvictionMDCache::isMDEntryPresent(Addr addr)
{
    uint64_t block_num = makeLineAddress(addr);

    bool status = false;
    // std::vector<Addr>::iterator it;
    std::unordered_map<Addr, bool>::iterator it =
                 evictionMDEntryMap.find(block_num);

    if (!(it == evictionMDEntryMap.end()))
        status = true;
    return status;
}

void
FSEvictionMDCache::addEvictionMDEntry(Addr addr)
{
    Addr block_num = makeLineAddress(addr);
    if (evictionMDEntryMap.size() == size_emd)
        warn_once("*******Eviction MD table OVERFLOW********: %d",
                             evictionMDEntryMap.size());
    evictionMDEntryMap[block_num] = true;
    if (evictionMDEntryMap.size() > 1024)
        warn_once("****** More than 1024 entries required");
    DPRINTF(OptionalMetadata,
            "MD entry added for Block on eviction: %#x; Size: %d\n",
            makeLineAddress(addr), evictionMDEntry.size());
}

void
FSEvictionMDCache::removeEvictionMDEntry(Addr addr)
{
    uint64_t block_num = makeLineAddress(addr);
    std::unordered_map<Addr, bool>::iterator it  =
     evictionMDEntryMap.find(block_num);
    if (!(it == evictionMDEntryMap.end())) {
        evictionMDEntryMap.erase(block_num);
        DPRINTF(OptionalMetadata, "MD Entry for block %#x removed.\n",
            block_num);
    }

}

FSEvictionMDCache*
FSEvictionMDCacheParams::create()
{
    return new FSEvictionMDCache(this);
}

/*
void
FSEvictionMDCache::regStats()
{
    SimObject::regStats();
    m_pm_read.name(name()+".total_read_pm")
    .desc("total read to the private access metadata by core");
    m_pm_write.name(name()+".total_write_pm")
    .desc("total write to the private access metadata by core");

}
*/
