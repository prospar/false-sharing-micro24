#ifndef __MEM_RUBY_STRUCTURES_PER_BLOCK_ENTRY_HH__
#define __MEM_RUBY_STRUCTURES_PER_BLOCK_ENTRY_HH__

#include <iostream>
#include <string>
#include <vector>

#include "base/types.hh"
#include "mem/ruby/common/Address.hh"
#include "mem/ruby/common/WriteMask.hh"
#include "mem/ruby/structures/FSGAEntry.hh"

class PerBlockEntry
{
  public:

    Tick tick_at_last_access; // FalseSharing: track replacement candidate
    bool isPrivatized; // FalseSharing: reserved to prevent replacement
    bool trueSharing; // FalseSharing: Identify a TS instance

    Tick prvStart; // FalseSharing: track start of privatization
    std::vector<FSGAEntry> offsetEntryList;

    PerBlockEntry();
    ~PerBlockEntry();

    std::vector<FSGAEntry> getOffsetEntryList();
    void setOffsetEntryList(std::vector<FSGAEntry> offsetEntryList);
    void setTrueSharing();
    bool checkTrueSharing();
    bool checkHysteresisStatus();
    Tick getLastAccessTick();
    void resetEntry();

    void print(std::ostream& out) const;
};
std::ostream& operator<<(std::ostream& out, const PerBlockEntry& obj);
#endif //__MEM_RUBY_STRUCTURES_PER_BLOCK_ENTRY_HH__