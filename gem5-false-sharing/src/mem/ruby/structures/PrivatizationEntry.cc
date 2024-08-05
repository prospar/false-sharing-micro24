
#include "mem/ruby/structures/PrivatizationEntry.hh"

using namespace std;

PrivatizationEntry::PrivatizationEntry()
{
    status=false;
}

PrivatizationEntry::~PrivatizationEntry()
{
}

Tick
PrivatizationEntry::getStartOfRegion() const
{
    return this->tickOfInitiation;
}

Tick
PrivatizationEntry::getEndOfRegion() const
{
    return this->tickOfTermination;
}

bool
PrivatizationEntry::getStatus() const
{
    return this->status;
}

void
PrivatizationEntry::setStartOfRegion(Tick startTime)
{
    this->tickOfInitiation = startTime;

}

void
PrivatizationEntry::setEndOfRegion(Tick endTime)
{
    this->tickOfTermination = endTime;
}

void
PrivatizationEntry::setStatus(bool status)
{
    this->status = status;
}

void
PrivatizationEntry::print(std::ostream& os) const
{
    os << getStatus();
    os << " B: " << getStartOfRegion();
    os << " E: " << getEndOfRegion();
    os << " Len: "<<(int)(getEndOfRegion() - getStartOfRegion())
     << std::flush;
}

// overiding method of Info Class "src/base/stats/info.hh"