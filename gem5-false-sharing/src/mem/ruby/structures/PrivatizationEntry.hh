
// A class to store a map for each block start and end
// region of privatization

#ifndef __MEM_RUBY_STRUCTURES_PRIVATIZATION_ENTRY_HH__
#define __MEM_RUBY_STRUCTURES_PRIVATIZATION_ENTRY_HH__

#include <iostream>

#include "base/types.hh"
#include "mem/ruby/common/Address.hh"

class PrivatizationEntry
{
  private:
    // track whether both start and end of region marked
    bool status;
    Tick tickOfInitiation;
    Tick tickOfTermination;

  public:
    PrivatizationEntry();
    ~PrivatizationEntry();
    Tick getStartOfRegion() const;
    Tick getEndOfRegion() const;
    bool getStatus() const;
    void setStartOfRegion(Tick startTime);
    void setEndOfRegion(Tick endTime);
    void setStatus(bool status);
    void print(std::ostream& out) const;
};

// std::ostream& operator<<(std::ostream& os, const PrivatizationEntry& obj);

inline std::ostream&
operator<<(std::ostream& out, const PrivatizationEntry& obj)
{
    obj.print(out);
    out << std::flush;
    return out;
}
#endif //__MEM_RUBY_STRUCTURES_PRIVATIZATION_ENTRY_HH__
