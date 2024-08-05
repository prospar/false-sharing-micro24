/**
 * FalseSharing:  Structure of Global Access tracking
 */

#include "mem/ruby/structures/FSGAEntry.hh"

#include <vector>

std::ostream&
operator<<(std::ostream& out, const FSGAEntry& obj)
{
    obj.print(out);
    out << std::flush;
    return out;
}

FSGAEntry::FSGAEntry() {
    lastWriterID = -1;
    // FalseSharing: used only if opt_readers flag provided in argument
    lastReaderID = -1;
    multipleReadersStatus = false;
}

FSGAEntry::~FSGAEntry() {}

int
FSGAEntry::getLastWriterID()
{
    return this->lastWriterID;
}

void
FSGAEntry::setLastWriterID(int lastWriterID)
{
    this->lastWriterID = lastWriterID;
}

IntVec
FSGAEntry::getSharerIDs()
{
    return this->sharerIDs;
}

void
FSGAEntry::setSharerIDs(IntVec sharerIDs)
{
    this->sharerIDs = sharerIDs;
}

void
FSGAEntry::updateSharer(int sharer)
{
    std::vector<int>::iterator it;
    bool notPresent = true;
    for (it = sharerIDs.begin(); it != sharerIDs.end(); ++it) {
        if (sharer == *it)
            notPresent = false;
    }
    if (notPresent)
        sharerIDs.push_back(sharer);
}

void
FSGAEntry::resetSharerIDs()
{
    this->sharerIDs.clear();
}

void
FSGAEntry::setLastReaderID(int readerID)
{
    this->lastReaderID = readerID;
}

int
FSGAEntry::getLastReaderID()
{
    return this->lastReaderID;
}

bool
FSGAEntry::getMultipleReadersStatus()
{
    return this->multipleReadersStatus;
}

void
FSGAEntry::setMultipleReadersStatus(bool status)
{
    this->multipleReadersStatus = status;
}

void
FSGAEntry::removeSharer(int sharer)
{
    std::vector<int>::iterator it;
    bool sharerExists = false;
    for (it = sharerIDs.begin(); it != sharerIDs.end(); ++it) {
        if (sharer == *it) {
            sharerExists = true;
            break;
        }
    }
    if (sharerExists)
        sharerIDs.erase(it);
}

void
FSGAEntry::resetReadersInfo()
{
    this->multipleReadersStatus = false;
    this->lastReaderID = -1;
}


void
FSGAEntry::print(std::ostream& out) const
{
    out << lastWriterID;
    for (const auto& it : sharerIDs) {
        out << " " << it;
    }
}