/**
 * FalseSharing:  Data Structure to hold Overlap Detection Entry for all
 * blocks.
 *
 *
 * DEBUG_FLAG: OverlapDetection (src/mem/ruby/Sconscript file)
 * DPRINTF(DEBUG_FLAG,"statement") --> (current_tick,
 *                                object_name, statement)
 *
 */

#include "mem/ruby/structures/FSPerCoreOD.hh"

#include "debug/OverlapDetection.hh"

//#include "sim/system.hh" //-- to access blocksize

#define FS_THRESHOLD 500

FSPerCoreOD::FSPerCoreOD(FSPerCoreODParams* params)
    : SimObject(params), od_size(params->od_size)
{
    DPRINTF(OverlapDetection, "FSPerCoreOD memory created.\n");
}

FSPerCoreOD::~FSPerCoreOD() {}

/**
 * Update to use od_size:
 *   Assert(): Number_of_OD_Entries <= od_size
 *   If od_size = 0, infinite length of table
 */
void
FSPerCoreOD::setFSODEntry(Addr addr)
{
    if (!blockPresent(addr)) {
        Addr block_num = makeLineAddress(addr);
        FSODEntry* newODEntry = new FSODEntry();
        od_state_data[block_num] = *newODEntry;
        DPRINTF(OverlapDetection, "Line %#x allocate in OD table .\n",
                block_num);
        delete newODEntry;
    } else
        DPRINTF(OverlapDetection, "Entry already exist for Line %#x.\n",
                makeLineAddress(addr));
}

bool
FSPerCoreOD::unsetFSODEntry(Addr addr)
{
    uint64_t block_num = makeLineAddress(addr);
    if (od_state_data.erase(block_num) == 1) {
        DPRINTF(OverlapDetection, "OD Table Size:%lu", od_state_data.size());
        return true;
    } else
        return false;
}

bool
FSPerCoreOD::updateFSODEntry(Addr addr)
{
    uint64_t block_num = makeLineAddress(addr);
    if (blockPresent(addr)) {
        FSODEntry existingEntry = od_state_data[block_num];
        if (existingEntry.getInvalidationCount() > FS_THRESHOLD) {
            DPRINTF(
                OverlapDetection,
                "Inv Count for Block %#x above threshold curr Inv: %llu.\n",
                addr, existingEntry.getInvalidationCount());
        }
        existingEntry.setInvalidationCount(
            existingEntry.getInvalidationCount() + 1);
        // FIXME: Is this store required?
        od_state_data[block_num] = existingEntry;
        DPRINTF(OverlapDetection, "Cache Block Entry %#x updated.\n",
                block_num);
        return true;
    }
    return false;
}

void
FSPerCoreOD::updatePFSBit(Addr addr, bool status)
{
    uint64_t block_num = makeLineAddress(addr);
    if (blockPresent(block_num)) {
        od_state_data[block_num].setPfsBit(status);
    }
}

void
FSPerCoreOD::updateCoherenceMissBit(Addr addr, bool status)
{
    Addr block_num = makeLineAddress(addr);
    if (blockPresent(block_num)) {
        od_state_data[block_num].setCoherenceMissBit(status);
    }
}

bool
FSPerCoreOD::blockPresent(Addr addr)
{
    uint64_t block_num = addr >> 6; // FIXME: Use constant
    auto it = od_state_data.find(block_num);
    if (it != od_state_data.end()) {
        return true;
    }
    return false;
}

bool
FSPerCoreOD::verifyPFSandCM(Addr addr)
{
    Addr block_num = makeLineAddress(addr);
    if (blockPresent(addr) && od_state_data[block_num].getPfsBit() &&
        od_state_data[block_num].getCoherenceMissBit()) {
        return true;
    }
    return false;
}

bool
FSPerCoreOD::isCoherenceMiss(Addr addr)
{
    Addr block_num = makeLineAddress(addr);
    if (od_state_data.find(block_num) != od_state_data.end()) {
        return od_state_data[block_num].getCoherenceMissBit();
    }
    return false;
}

// If not included linking error
FSPerCoreOD*
FSPerCoreODParams::create()
{
    return new FSPerCoreOD(this);
}
