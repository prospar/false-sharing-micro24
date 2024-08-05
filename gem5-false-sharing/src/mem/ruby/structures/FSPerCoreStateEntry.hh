/**
 * Entry : OptionalPerCore Metadata
 */

#ifndef __MEM_RUBY_STRUCTURES_FS_PERCORE_STATE_ENTRY_HH__
#define __MEM_RUBY_STRUCTURES_FS_PERCORE_STATE_ENTRY_HH__

#include <cstdint>

#include "base/statistics.hh"
#include "base/types.hh"
#include "mem/ruby/common/BoolVec.hh"

class FSPerCoreStateEntry
{
  private:
    // FalseSharing: a bit to track whether to send MD on eviction
    bool sendMDonEviction;
    Tick tick_at_last_access;

  public:
    BoolVec blockAccessRead;
    BoolVec blockAccessWrite;

    FSPerCoreStateEntry(uint32_t tracking_width);
    FSPerCoreStateEntry();
    ~FSPerCoreStateEntry();
    // opType: True: read and False: write
    void setByteAccess(uint16_t byteOffset, bool value, bool opType);

    bool getByteAccess(uint16_t byteOffset, bool opType);
    bool getSendAccessMD();
    void setSendAccessMD(bool status);
    Tick getLastAccessTick();

    void setLastAccessTick(Tick tick);

    void print(std::ostream& out) const;
};

inline std::ostream&
operator<<(std::ostream& out, const FSPerCoreStateEntry& obj)
{
    obj.print(out);
    out << std::flush;
    return out;
}

#endif //__MEM_RUBY_STRUCTURES_FS_PERCORE_STATE_ENTRY_HH__
