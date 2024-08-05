

#include "mem/ruby/structures/PrivatizationStat.hh"

#include "base/logging.hh"

// using namespace std;

namespace Stats
{

PrivatizationStat::PrivatizationStat() {}

PrivatizationStat::~PrivatizationStat()
{
    inform("Called Destructor of PRIVATIZATION STAT");
}

void
PrivatizationStat::setPrivatizationEntry(Addr addr, PrivatizationEntry entry)
{
    Addr block_num = makeLineAddress(addr);
    if (statExists(addr)) {
        std::vector<PrivatizationEntry> currList = prvRegions[block_num];
        currList.push_back(entry);
        prvRegions[block_num] = currList;
    } else {
        std::vector<PrivatizationEntry> currList;
        currList.push_back(entry);
        prvRegions[block_num] = currList;
    }
}
PrivatizationEntry
PrivatizationStat::getPrivatizationEntry(Addr addr) const
{
    Addr block_num = makeLineAddress(addr);
    PrivatizationEntry currEntry;
    auto it = prvRegions.find(block_num);
    std::vector<PrivatizationEntry> currList = it->second;
    currEntry = currList[currList.size() - 1];
    return currEntry;
}
void
PrivatizationStat::setStartRegion(Addr addr, Tick currentTick)
{
    Addr block_num = makeLineAddress(addr);
    if (statExists(addr)) {
        std::vector<PrivatizationEntry> currList = prvRegions[block_num];
        PrivatizationEntry* entry = new PrivatizationEntry();
        // check for current
        entry->setStartOfRegion(currentTick);
        currList.push_back(*entry);
        prvRegions[block_num] = currList;

    } else {
        std::vector<PrivatizationEntry> currList;
        PrivatizationEntry* entry = new PrivatizationEntry();
        // check for current
        entry->setStartOfRegion(currentTick);
        currList.push_back(*entry);
        prvRegions[block_num] = currList;
    }
}

Tick
PrivatizationStat::getStartRegion(Addr addr)
{
    Addr block_num = makeLineAddress(addr);

    std::vector<PrivatizationEntry> currList = prvRegions[block_num];
    return currList.back().getStartOfRegion();
}

void
PrivatizationStat::setEndRegion(Addr addr, Tick currentTick)
{
    Addr block_num = makeLineAddress(addr);

    std::vector<PrivatizationEntry> currList = prvRegions[block_num];
    currList.back().setEndOfRegion(currentTick);
    prvRegions[block_num] = currList;
}

Tick
PrivatizationStat::getEndRegion(Addr addr)
{
    Addr block_num = makeLineAddress(addr);
    // auto iter = prvRegions.find(block_num);
    std::vector<PrivatizationEntry> currList = prvRegions[block_num];
    return currList.back().getEndOfRegion();
}

bool
PrivatizationStat::statExists(Addr addr)
{
    bool found = false;
    Addr block_num = makeLineAddress(addr);

    // FalseSharing: set associative changes
    auto iter = prvRegions.find(block_num);
    if (iter != prvRegions.end())
        found = true;

    return found;
}

void
PrivatizationStat::print(std::ostream& os) const
{
    for (const auto& it : prvRegions) {
        os << "Block " << it.first << " privatization records::::\n";
        std::vector<PrivatizationEntry> currRecords = it.second;
        std::vector<PrivatizationEntry>::iterator itVec = currRecords.begin();
        for (; itVec != currRecords.end(); ++itVec)
            os << *itVec << "\n";
    }
    os << std::flush;
}

} // namespace Stats
