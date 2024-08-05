/**
 * FalseSharing:  Define structure for an overlap detection entry
 * Tracks:
 *          1. # of INValidation for cache block
 *          2. Potential False Sharing candidate
 */

#include "mem/ruby/structures/FSODEntry.hh"

FSODEntry::FSODEntry()
{
    // DPRINTF(FSODDebugFlag,"ODEntry  created.")
    isCoherenceMiss = true;
    isPFS = false;
    invalidationCount = 0;
}

FSODEntry::~FSODEntry() {}

bool
FSODEntry::getCoherenceMissBit()
{
    return this->isCoherenceMiss;
}

void
FSODEntry::setCoherenceMissBit(bool isCoherenceMiss)
{
    this->isCoherenceMiss = isCoherenceMiss;
}

bool
FSODEntry::getPfsBit()
{
    return this->isPFS;
}

void
FSODEntry::setPfsBit(bool isPFS)
{
    this->isPFS = isPFS;
}

void
FSODEntry::setInvalidationCount(int invalidationCount)
{
    this->invalidationCount = invalidationCount;
}

int
FSODEntry::getInvalidationCount()
{
    return this->invalidationCount;
}
