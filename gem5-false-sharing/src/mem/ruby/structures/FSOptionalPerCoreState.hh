#ifndef __MEM_RUBY_STRUCTURES_FS_OPTIONAL_PER_CORE_STATE_HH__
#define __MEM_RUBY_STRUCTURES_FS_OPTIONAL_PER_CORE_STATE_HH__

#include <unordered_map>

#include "base/logging.hh"
#include "base/statistics.hh"
#include "base/types.hh"
#include "debug/OptionalMetadata.hh"
#include "mem/ruby/common/Address.hh"
#include "mem/ruby/protocol/RubyRequestType.hh"
#include "mem/ruby/structures/FSPerCoreStateEntry.hh"
#include "params/FSOptionalPerCoreState.hh"
#include "sim/sim_object.hh"

class FSOptionalPerCoreState : public SimObject
{
  private:
  public:
    std::unordered_map<Addr, FSPerCoreStateEntry> optional_per_core_map;
    std::vector<std::unordered_map<Addr, FSPerCoreStateEntry>> perCoreMetadata;
    uint32_t tracking_width;
    uint32_t assoc_own;
    uint32_t size_own;
    uint32_t set_own;     // number of set in the own access metadata table
    int offset_bit_index; // bits to address bytes of a block
    int set_index_bits;   // bits to identify a set

    FSOptionalPerCoreState(FSOptionalPerCoreStateParams* params);
    ~FSOptionalPerCoreState();

    void addOptionalMetadataEntry(Addr addr);

    FSPerCoreStateEntry getBlockAccess(Addr addr);

    void updateOptionalMetadataEntry(Addr addr, int accessedBytes,
                                     bool opType);

    void removeOptionalMetadataEntry(Addr addr);

    bool validMetadataEntry(Addr addr);

    void resetMetadataEntry(Addr addr);

    bool accessToNewBytes(Addr addr, int accessedBytes,
                          RubyRequestType accessType);

    int findSetIndex(Addr addr);

    Addr findVictimOwnEntry(int set_index);

    void updateMDCommBit(Addr addr, bool status);
    bool getMDCommBit(Addr addr);

    void regStats();
    // stat for privatization
    //Stats::Vector2D;
    //Stats::Scalar ;
    Stats::Scalar m_pm_read;
    Stats::Scalar m_pm_write;
    Stats::Scalar m_prv_rd;
    Stats::Scalar m_prv_wr;
    Stats::Scalar m_evc_shared;
    Stats::Scalar m_evc_owner;
    Stats::Scalar m_prv_replacement;

};

#endif //__MEM_RUBY_STRUCTURES_FS_OPTIONAL_PER_CORE_STATE_HH__
