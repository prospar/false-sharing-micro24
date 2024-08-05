
// A class to store a map for each block start and end
// region of privatization

#ifndef __MEM_RUBY_STRUCTURES_PRIVATIZATION_STAT_HH__
#define __MEM_RUBY_STRUCTURES_PRIVATIZATION_STAT_HH__

#include <iostream>
#include <map>
#include <vector>

#include "base/statistics.hh"
#include "mem/ruby/structures/PrivatizationEntry.hh"

namespace Stats
{

class PrivatizationStat : public Info
{
  private:
  public:
    PrivatizationStat();
    ~PrivatizationStat();
    std::map<Addr, std::vector<PrivatizationEntry>> prvRegions;

    void setPrivatizationEntry(Addr addr, PrivatizationEntry entry);
    PrivatizationEntry getPrivatizationEntry(Addr addr) const;
    void setStartRegion(Addr addr, Tick currentTick);
    Tick getStartRegion(Addr addr);
    void setEndRegion(Addr addr, Tick currentTick);
    Tick getEndRegion(Addr addr);
    bool statExists(Addr addr);
    void print(std::ostream& out) const;
};

inline std::ostream&
operator<<(std::ostream& out, const PrivatizationStat& obj)
{
    obj.print(out);
    out << std::flush;
    return out;
}

} // namespace Stats
#endif //__MEM_RUBY_STRUCTURES_PRIVATIZATION_STAT_HH__
