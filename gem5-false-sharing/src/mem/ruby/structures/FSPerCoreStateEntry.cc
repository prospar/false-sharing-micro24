/**
 * Store Metadata at percore for reducing Global Access update and accesses
 *
 */

#include "mem/ruby/structures/FSPerCoreStateEntry.hh"

#include <iostream>

#include "mem/ruby/system/RubySystem.hh"

// #include "debug/flag_name.hh"

FSPerCoreStateEntry::FSPerCoreStateEntry(uint32_t tracking_width)
{
    blockAccessRead.resize(RubySystem::getBlockSizeBytes() / tracking_width,
                           false);
    blockAccessWrite.resize(RubySystem::getBlockSizeBytes() / tracking_width,
                            false);
    tick_at_last_access = curTick();
    sendMDonEviction = false;
}

FSPerCoreStateEntry::FSPerCoreStateEntry()
{
    uint32_t tracking_width = RubySystem::tracking_width;
    blockAccessRead.resize(RubySystem::getBlockSizeBytes() / tracking_width,
                           false);
    blockAccessWrite.resize(RubySystem::getBlockSizeBytes() / tracking_width,
                            false);
    tick_at_last_access = curTick();
    sendMDonEviction = false;
}

FSPerCoreStateEntry::~FSPerCoreStateEntry() {}

void
FSPerCoreStateEntry::setByteAccess(uint16_t byteOffset, bool value,
                                   bool opType)
{
    // opType: true = RD access; false = WR access
    if (opType) {
        // FalseSharing: W->R, do not set read bit
        // FalseSharing: on a read after write set read
        // checking for write bit will require one extra load
        //if (!blockAccessWrite[byteOffset])
        this->blockAccessRead[byteOffset] = value;
    } else {
        this->blockAccessWrite[byteOffset] = value;
        //blockAccessRead[byteOffset] = !value; // on WR unset RD bits
    }
}

bool
FSPerCoreStateEntry::getByteAccess(uint16_t byteOffset, bool opType)
{
    if (opType) // true read acccess
        return this->blockAccessRead[byteOffset];
    else
        return this->blockAccessWrite[byteOffset];
}

Tick
FSPerCoreStateEntry::getLastAccessTick()
{
    return this->tick_at_last_access;
}

void
FSPerCoreStateEntry::setLastAccessTick(Tick tick)
{
    this->tick_at_last_access = tick;
}

bool
FSPerCoreStateEntry::getSendAccessMD()
{
    return sendMDonEviction;
}

void
FSPerCoreStateEntry::setSendAccessMD(bool status)
{
    this->sendMDonEviction = status;
}

void
FSPerCoreStateEntry::print(std::ostream& os) const
{
    os << "[";
    for (int i = 0; i < this->blockAccessRead.size(); i++) {
        os << (int)this->blockAccessRead[i];
    }
    os <<" ";
    for (int i = 0; i < this->blockAccessWrite.size(); i++) {
        os << (int)this->blockAccessWrite[i];
    }
    os << " " << this->sendMDonEviction << " ]" << std::flush;
}
