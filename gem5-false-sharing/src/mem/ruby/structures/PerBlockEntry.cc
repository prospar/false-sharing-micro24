#include "mem/ruby/structures/PerBlockEntry.hh"

PerBlockEntry::PerBlockEntry()
{
    trueSharing = false;
    isPrivatized = false;
}

PerBlockEntry::~PerBlockEntry()
{
}

std::vector<FSGAEntry>
PerBlockEntry::getOffsetEntryList()
{
    return offsetEntryList;
}

void
PerBlockEntry::setOffsetEntryList(std::vector<FSGAEntry> offsetEntryList)
{
    this->offsetEntryList = offsetEntryList;
}

void
PerBlockEntry::setTrueSharing()
{
    trueSharing = true;
}

bool
PerBlockEntry::checkTrueSharing()
{
    return trueSharing;
}


bool
PerBlockEntry::checkHysteresisStatus()
{
    // if (hysteresis_counter > HYST_THRESHOLD) {
    //     return true;
    // } else {
    //     return false;
    // }
    return false;
}

Tick
PerBlockEntry::getLastAccessTick()
{
    return tick_at_last_access;
}

void
PerBlockEntry::resetEntry()
{
    this->isPrivatized = false;
    // FalseSharing: required to capture the false sharing arising
    // from common initialization pattern
    this->trueSharing = false;
    // FalseSharing: set associative changes
    this->tick_at_last_access = curTick();
    std::vector<FSGAEntry>::iterator it;
    for (it = getOffsetEntryList().begin();
            it != getOffsetEntryList().end(); ++it) {
        it->setLastWriterID(-1);
        it->resetSharerIDs();
    }
}

void
PerBlockEntry::print(std::ostream& out) const
{
    out << trueSharing;
    for (const auto& it : offsetEntryList) {
        out << " " << it;
    }
    out << "\n";
}

std::ostream&
operator<<(std::ostream& out, const PerBlockEntry& obj)
{
    obj.print(out);
    out << std::flush;
    return out;
}
