/**
 * FSPerCoreOptionalMetadata:
 * Implement field and methods for optional per core metadata
 */

#include "mem/ruby/structures/FSOptionalPerCoreState.hh"

#include "base/intmath.hh"
#include "base/logging.hh"
#include "debug/PrvDebug.hh"
#include "mem/ruby/protocol/RubyRequestType.hh"

FSOptionalPerCoreState::FSOptionalPerCoreState(
    FSOptionalPerCoreStateParams* params)
    : SimObject(params), tracking_width(params->tracking_width),
      assoc_own(params->assoc_own), size_own(params->size_own),
      offset_bit_index(params->offset_bit_index)
{
    if (size_own == 0 || assoc_own == 0) {
        assoc_own = 0;
        set_own = 1;
        set_index_bits = floorLog2(set_own);
        perCoreMetadata.resize(set_own);
        inform("A fully associative PerCore MD store created with infinite\
             size, and tracking_granularity:%d\n",
               tracking_width);
    } else {
        assert(isPowerOf2(size_own));
        assert(isPowerOf2(assoc_own));
        set_own = size_own / assoc_own;
        set_index_bits = floorLog2(set_own);
        perCoreMetadata.resize(set_own);
        inform("A PerCore MD store created with size: %d, associativy: %d,\
             num_set:%d, and tracking_granularity:%d\n",
               size_own, assoc_own, set_own, tracking_width);
    }
}

FSOptionalPerCoreState::~FSOptionalPerCoreState() {}

bool
FSOptionalPerCoreState::validMetadataEntry(Addr addr)
{
    Addr block_num = makeLineAddress(addr);
    int set_index = findSetIndex(block_num);

    bool status = false;
    auto iter = perCoreMetadata[set_index].find(block_num);
    if (iter != perCoreMetadata[set_index].end()) {
        status = true;
    }

    return status;
}

void
FSOptionalPerCoreState::addOptionalMetadataEntry(Addr addr)
{
    m_pm_write += 2;
    Addr block_num = makeLineAddress(addr);
    int set_index = findSetIndex(addr);
    FSPerCoreStateEntry* accessMetadataEntry =
        new FSPerCoreStateEntry(tracking_width);
    accessMetadataEntry->setLastAccessTick(curTick());
    if (validMetadataEntry(addr)) {
        bool duplicatePAMAddition = false;
        assert(duplicatePAMAddition);
    }
    if (perCoreMetadata[set_index].size() < assoc_own) {
        perCoreMetadata[set_index][block_num] = *accessMetadataEntry;
        //inform("Entry added for block %#x", block_num);
    } else {
        bool excessEntryInPAM = false;
        assert(excessEntryInPAM);
        Addr victim = findVictimOwnEntry(set_index);
        perCoreMetadata[set_index].erase(victim);
        perCoreMetadata[set_index][block_num] = *accessMetadataEntry;
    }

    //DPRINTF(OptionalMetadata,
    //        "Metadata entry added for Block: %#x; Size: %d\n",
    //        makeLineAddress(addr), perCoreMetadata[set_index].size());
}

FSPerCoreStateEntry
FSOptionalPerCoreState::getBlockAccess(Addr addr)
{
    // FalseSharing: one read for read vector
    // one read for write vector
    m_pm_read += 2;
    uint64_t block_num = makeLineAddress(addr);
    int set_index = findSetIndex(addr);
    // FalseSharing: set associative change
    if (!validMetadataEntry(addr)) {
        warn_once("PAM entry %#x was not allocated for D cache block",\
            block_num);
        // FalseSharing: Metadata is not allocated while data is allocated
        bool blockAllocateMetadataNot=false;
        assert(blockAllocateMetadataNot);

        FSPerCoreStateEntry* newEntry =
            new FSPerCoreStateEntry(tracking_width);

        if (perCoreMetadata[set_index].size() < assoc_own)
            perCoreMetadata[set_index][block_num] = *newEntry;
        else {
            Addr victim = findVictimOwnEntry(set_index);
            perCoreMetadata[set_index].erase(victim);
            perCoreMetadata[set_index][block_num] = *newEntry;
        }
    }

    //DPRINTF(OptionalMetadata, "PerCoreMetadata for block: %#x \n",
    // block_num);
    return perCoreMetadata[set_index][block_num];
}

// FalseSharing: opType true: read, false: write
void
FSOptionalPerCoreState::updateOptionalMetadataEntry(Addr addr,
                                                    int accessedBytes,
                                                    bool opType)
{
    // FalseSharing: update require read and write
    // to both vector
    ++m_pm_read;
    ++m_pm_write;
    Addr block_num = makeLineAddress(addr);
    int set_index = findSetIndex(addr);
    Addr startOffset = getOffset(addr);
    int startIndex = startOffset / tracking_width;
    int totalBytesAccessed = accessedBytes/tracking_width;

    // Falsesharing: set associative changes
    if (!validMetadataEntry(addr)) {
        warn_once("PAM entry for block %#x was not allocated",\
            block_num);
        bool pamEntryNotPresent = false;
        assert(pamEntryNotPresent);
        FSPerCoreStateEntry* newEntry =
            new FSPerCoreStateEntry(tracking_width);
        if (perCoreMetadata[set_index].size() < assoc_own)
            perCoreMetadata[set_index][block_num] = *newEntry;
        else {
            Addr victim = findVictimOwnEntry(set_index);
            perCoreMetadata[set_index].erase(victim);
            perCoreMetadata[set_index][block_num] = *newEntry;
            DPRINTF(OptionalMetadata, "Block %#x evicts entry for\
                block %#x",block_num, victim);
        }
        DPRINTF(OptionalMetadata, "MD entry added for: %#x\n",\
        block_num);

    } else {
        FSPerCoreStateEntry existingEntry =
            perCoreMetadata[set_index][block_num];
        DPRINTF(OptionalMetadata, "Before MD update for offset: %#x %d %s\n",\
            block_num, startOffset, existingEntry);

        for (int i = startIndex; i < (startIndex + totalBytesAccessed); i++) {
            existingEntry.setByteAccess(i, true, opType);
        }
        //existingEntry.setLastAccessTick(curTick());
        // FalseSharing: set associative changes
        perCoreMetadata[set_index][block_num] = existingEntry;

        DPRINTF(OptionalMetadata, "After Metadata update for offset:\
        %#x size: %d %s\n", block_num, accessedBytes, existingEntry);
    }
}

void
FSOptionalPerCoreState::removeOptionalMetadataEntry(Addr addr)
{
    uint64_t block_num = makeLineAddress(addr);
    int set_index = findSetIndex(addr);
    // FalseSharing: set associative changes
    if (perCoreMetadata[set_index].erase(block_num)) {
        //DPRINTF(OptionalMetadata, " Entry for block %#x removed.\n",
        //        block_num);
    }
}

void
FSOptionalPerCoreState::resetMetadataEntry(Addr addr)
{
    // FalseSharing: resetting metadata
    // initalize bits to 0.
    m_pm_write += 2;
    Addr block_num = makeLineAddress(addr);
    int set_index = findSetIndex(addr);
    FSPerCoreStateEntry* entry = new FSPerCoreStateEntry(tracking_width);
    // FalseSharing: set associative changes
    entry->setLastAccessTick(curTick());
    perCoreMetadata[set_index][block_num] = *entry;
}

// FalseSharing: used for for private block
bool
FSOptionalPerCoreState::accessToNewBytes(Addr physicalAddr, int num_of_bytes,
                                         RubyRequestType accessType)
{
    // FalseSharing: checking new byte access require one read
    ++m_pm_read;

    Addr block_num = makeLineAddress(physicalAddr);
    int set_index = findSetIndex(block_num);
    // FalseSharing: set associative changes
    FSPerCoreStateEntry existingEntry = perCoreMetadata[set_index][block_num];
    //existingEntry.setLastAccessTick(curTick());

    Addr startOffset = getOffset(physicalAddr);
    int startIndex = startOffset / tracking_width;
    // FalseSharing: changeD this
    int totalBytesAccessed = num_of_bytes/tracking_width;
    // Irrespective of ST or LD if core has accessed byte earlier.
    // As block is privatized, read_write permission
    bool firstAccess = false;

    DPRINTF(OptionalMetadata,"Access to new bytes: %#x %d %s\n",
     physicalAddr, num_of_bytes, existingEntry);

    // NO LD/ST to a byte earlier
    for (int i = startIndex; i < (startIndex+totalBytesAccessed); i++) {
        // FalseSharing: A LD by core
        if (RubyRequestType_to_string(accessType) == "LD") {

            // check if a write is done, read is allowed
            if (!existingEntry.getByteAccess(i, false)) {
                // FalseSharing: read requires two read to PAM entry
                ++m_pm_read;
                // FalseSharing: check if a read is done earlier
                if (!existingEntry.getByteAccess(i, true)) {
                    firstAccess = true;
                    break;
                }
            }
        } else if (RubyRequestType_to_string(accessType) == "IFETCH") {
            bool metadataUpdateForICache = false;
            assert(metadataUpdateForICache);
        } else { // FalseSharing: an ST by core
                 // RMW_Read and RMW_Write are classified as Store
            if (!existingEntry.getByteAccess(i, false)) {
                firstAccess = true;
                break;
            }
        }
    }
    return firstAccess;
}

/**
 * @brief find set of block in own access MDtable
 *
 * @param addr: block address
 * @return index of set
 */
// FalseSharing: find the set the entry belongs
int
FSOptionalPerCoreState::findSetIndex(Addr addr)
{
    return bitSelect(addr, offset_bit_index,
                     offset_bit_index + set_index_bits - 1);
}

/**
 * @brief identify victim entry for replacement
 *
 * @param index_set
 * @return block address
 */
Addr
FSOptionalPerCoreState::findVictimOwnEntry(int index_set)
{
    std::unordered_map<Addr, FSPerCoreStateEntry>::iterator it;
    Addr victimAddr = 0;
    Tick currentTick = curTick();

    // FalseSharing: Ideally PAM have no replacement due one to one mapping
    // with Dcache
    bool victimRequiredInPAM = false;
    assert(victimRequiredInPAM);

    // check for all entry within set
    for (it = perCoreMetadata[index_set].begin();
         it != perCoreMetadata[index_set].end(); ++it) {
        // FalseSharing: other approach could be to replace first victim found
        if (it->second.getLastAccessTick() < currentTick) {
            victimAddr = it->first;
            currentTick = it->second.getLastAccessTick();
        }
    }
    return victimAddr;
}

void
FSOptionalPerCoreState::updateMDCommBit(Addr addr, bool status)
{
    ++m_pm_write;
    Addr block_num = makeLineAddress(addr);
    int set_index = findSetIndex(block_num);
    if (!validMetadataEntry(addr)) {
        warn_once("Block %#x entry does not exists for MD bit update.\n",\
            block_num);
        bool blockNotExistsNDBitSet = false;
        assert(blockNotExistsNDBitSet);
    } else {
        FSPerCoreStateEntry existingEntry =
             perCoreMetadata[set_index][block_num];
        existingEntry.setSendAccessMD(status);
        perCoreMetadata[set_index][block_num] = existingEntry;
    }
}


bool
FSOptionalPerCoreState::getMDCommBit(Addr addr)
{
    ++m_pm_read;
    bool sendMetadata = false;

    Addr block_num = makeLineAddress(addr);
    int set_index = findSetIndex(block_num);
    if (!validMetadataEntry(addr)) {
        warn_once("Block %#x entry does not exists to fetch MD Comm.\n",\
            block_num);
        bool blockNotExistsMDBitFetch = false;
        assert(blockNotExistsMDBitFetch);
    } else {
        FSPerCoreStateEntry existingEntry =
             perCoreMetadata[set_index][block_num];
        sendMetadata = existingEntry.getSendAccessMD();
    }
    return sendMetadata;
}

FSOptionalPerCoreState*
FSOptionalPerCoreStateParams::create()
{
    return new FSOptionalPerCoreState(this);
}

void
FSOptionalPerCoreState::regStats()
{
    SimObject::regStats();
    m_pm_read.name(name()+".total_read_pm")
        .desc("total read to the private access metadata by core");
    m_pm_write.name(name()+".total_write_pm")
        .desc("total write to the private access metadata by core");
    m_prv_wr.name(name()+".total_prv_write")
        .desc("total write to private line");
    m_prv_rd.name(name()+".total_prv_read")
        .desc("total read to private line");
    m_evc_shared.name(name()+".evc_shared")
        .desc("total eviction MD sent in S");
    m_evc_owner.name(name()+".evc_owner")
        .desc("total eviction MD sent in M");
    m_prv_replacement.name(name()+".prv_line_replacement")
        .desc("total replacement of prv line at L1 controller");
}
