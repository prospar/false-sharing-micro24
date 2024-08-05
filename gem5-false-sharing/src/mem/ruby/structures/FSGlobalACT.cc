/**
 * FalseSharing:  Global access tracking structure
 * Debug Flag : GlobalAccess (src/mem/ruby/Sconscript file)
 * global_act_size: param to decide size of AcT table in terms of entry
 * PerBlockEntry: (C+logC)*64 per block wher C is number of cores in system.
 *       size 0 infinite memory
 *       size other than 0:  total N entries with assoc same as LLC/Directory.
 *     // TODO: add false sharing ratio as a command line argument
 */

#include "mem/ruby/structures/FSGlobalACT.hh"

#include "base/intmath.hh"
#include "debug/RubyStats.hh"
#include "mem/ruby/system/RubySystem.hh"

std::ostream&
operator<<(std::ostream& out, const FSGlobalACT& obj)
{
    obj.print(out);
    out << std::flush;
    return out;
}

FSGlobalACT::FSGlobalACT(FSGlobalACTParams* params)
    : SimObject(params),
      //   event([this] { metadataResetEvent(); }, name() + ".event"),
      global_act_size(params->global_act_size),
      tracking_width(params->tracking_width), assoc_act(params->assoc_act),
      offset_bit_index(params->offset_bit_index),
      reset_interval(params->reset_interval), opt_readers(params->opt_readers),
      report_pc(params->report_pc)
{
    if (global_act_size == 0 || assoc_act == 0) {
        assoc_act = 0; // infinite entries in set
        global_act_size = 0;
        set_act = 1;
        num_set_bits = floorLog2(set_act);
        glb_act_data.resize(set_act);
        inform("Global AcT table of infinite size with tracking_granularity:"
               "%d\n",
               tracking_width);
        // FalseSharing: schedule the metadata reset event

    } else {
        assert(isPowerOf2(global_act_size));
        assert(isPowerOf2(assoc_act));
        set_act = global_act_size / assoc_act;
        num_set_bits = floorLog2(set_act); //
        glb_act_data.resize(set_act);
        inform("Global AcT table of size: %d, associativy: %d, num_set:%d, and"
               " tracking_granularity:%d\n",
               global_act_size, assoc_act, set_act, tracking_width);
    }
}

FSGlobalACT::~FSGlobalACT() {}
/**
 * Method: Allocating new entry for a cacheblock
 * @readOrWrite: type of request
 * @addr: physical address, makeLineAddress(addr) to obtain block start address
 * @accessed_byte_num: no of byte to access in current request
 * @machineId: requestor Id
 */
//  Allocate entry on receiving metdata update
void
FSGlobalACT::setFSGAEntry(Addr addr, int accessed_byte_num,
                          MachineID machineId, bool readOrWrite)
{
    Addr block_num = makeLineAddress(addr);
    // FalseSharing: find set and tag
    int set_index = findSetAct(addr);

    // int coreID = machineId.getNum();
    // int startOffset = getOffset(addr);
    //  Addr block_num = makeLineAddress(addr);
    PerBlockEntry* blockEntry = new PerBlockEntry();
    std::vector<FSGAEntry> offsetEntryList;
    // FalseSharing: update only on metadata messages
    /*
    // metadata entry
    int startIndex = startOffset / tracking_width;
    int lastIndex = (startOffset + accessed_byte_num - 1) / tracking_width;

    for (int metadataIndex = startIndex; metadataIndex <= lastIndex;
         metadataIndex++) {
        FSGAEntry* newEntry = new FSGAEntry();
        newEntry->offset = metadataIndex;
        if (readOrWrite) // true : read
        {
            newEntry->updateSharer(coreID);
        } else {
            newEntry->setLastWriterID(coreID);
        }
        offsetEntryList.push_back(*newEntry);
    }
    */
    // Allocate an empty entry
    blockEntry->offsetEntryList = offsetEntryList;
    blockEntry->tick_at_last_access = curTick();

    // global_act_data[block_num] = *blockEntry;
    if (global_act_size > 0) {
        // FalseSharing: check if number of entries in the set
        // is equal to associativity if no, add entry to vector
        // assoc = 0 mean full associative
        if (glb_act_data[set_index].size() < assoc_act || assoc_act == 0)
            glb_act_data[set_index][block_num] = *blockEntry;

        else { // FalseSharing: replace the block
            // TODO: decide on whether to allocate a block or not
            Addr victimKey = findVictimAcTEntry(set_index);
            glb_act_data[set_index].erase(victimKey);
            glb_act_data[set_index][block_num] = *blockEntry;
        }
    } else { // infinite memory, no need for replacement
        glb_act_data[set_index][block_num] = *blockEntry;
    }
}

/**
 * FalseSharing: update the AcT table based on OWN accesss MD  of core.
 * @Param: addr: line starting address
 * @Param: accessMetadata: access metadata send by a core
 * @Param: machineID: sender core ID
 * Updates the global access tracking table
 */
Addr
FSGlobalACT::updateFSGAEntry(Addr addr, FSPerCoreStateEntry accessMetadata,
                             MachineID machineId)
{
    // FalseSharing: The read and write bit vector mapped as 8 byte row
    // The update will done in two step
    // i: read entire row ii: write the entire row
    // ++m_sm_read;
    ++m_sm_write;

    int set_index = findSetAct(addr);
    Addr victim_addr = MaxAddr; // max value an addr can represent
    // block_num always less than this as lower 6 bits capped
    int coreID = machineId.getNum();
    Addr block_num = makeLineAddress(addr);

    if (blockEntryExists(addr)) {
        PerBlockEntry currEntry = glb_act_data[set_index][block_num];
        std::vector<FSGAEntry> existingOffsets =
            currEntry.getOffsetEntryList();
        std::vector<FSGAEntry> firstAccessOffset;
        bool newOffsetAccess = false;
        // with granularity G, access metadata contains BLOCKSIZE/G indices
        // FalseSharing: Iterate over all access metadata entry
        for (int i = 0; i < BLOCKSIZE / tracking_width; i++) {
            if (accessMetadata.blockAccessWrite[i] ||
                accessMetadata.blockAccessRead[i]) {
                bool firstAccess = true;

                //  FalseSharing: find entry for the current byte in existing
                //  entries
                std::vector<FSGAEntry>::iterator it;
                for (it = existingOffsets.begin(); it != existingOffsets.end();
                     ++it) {
                    if (it->offset == i) {
                        firstAccess = false;
                        // FalseSharing: if an ST done, update as last writer
                        if (accessMetadata.blockAccessWrite[i]) {
                            // FalseSharing: check whether lastWriter is other
                            // than current core : WW TS instance
                            if (it->getLastWriterID() != -1 &&
                                it->getLastWriterID() != coreID) {
                                currEntry.setTrueSharing();
                            }
                            // FalseShaaing: check if any existing reader,
                            // other than core itself
                            if (opt_readers) {
                                // FalseSharing: If multiple reader, set TS
                                // or check if last reader is other than core
                                if ((it->getMultipleReadersStatus())||
                                    (it->getLastReaderID() != -1 &&
                                    it->getLastReaderID() != coreID)) {
                                    currEntry.setTrueSharing();
                                }
                            } else { // FalseSharing: check for all sharers
                                std::vector<int> sharers = it->getSharerIDs();
                                std::vector<int>::iterator it;
                                for (it = sharers.begin();
                                     it != sharers.end(); ++it) {
                                    if (*it != coreID) {
                                        currEntry.setTrueSharing();
                                        break;
                                    }
                                }
                            }
                            it->setLastWriterID(coreID);
                        }
                        // FalseSharing: if a LD done, update core as sharer
                        if (accessMetadata.blockAccessRead[i]) {
                            // If other core is writer of byte, update TS bit
                            if (it->getLastWriterID() != -1 &&
                                it->getLastWriterID() != coreID) {
                                currEntry.setTrueSharing();
                            }
                            // FalseSharing: optimize reader info
                            if (opt_readers) {
                                // FalseSharing: If multiple reader bit is set
                                // no action
                                if (!(it->getMultipleReadersStatus())) {
                                    // FalseSharing: last reader different
                                    // from core set multiple reader
                                    if (it->getLastReaderID() != -1 &&
                                        it->getLastReaderID() != coreID) {
                                        it->setMultipleReadersStatus(true);
                                    } else {
                                        it->setLastReaderID(coreID);
                                    }
                                }
                            } else {
                                it->updateSharer(coreID);
                            }
                        }
                        break;
                    }
                }

                // FalseSharing: first access to byte, assign new entry
                if (firstAccess) {
                    FSGAEntry* newEntry = new FSGAEntry();
                    newEntry->offset = i;
                    if (accessMetadata.blockAccessWrite[i]) {
                        newEntry->setLastWriterID(coreID);
                    }
                    if (accessMetadata.blockAccessRead[i]) {
                        // FalseSharing: optimize reader info
                        if (opt_readers) {
                            // FalseSharing: since first access to byte
                            // simple update readerID
                            newEntry->setLastReaderID(coreID);
                            newEntry->setMultipleReadersStatus(false);
                        } else {
                            newEntry->updateSharer(coreID);
                        }
                    }
                    firstAccessOffset.push_back(*newEntry);
                    newOffsetAccess = true;
                    // delete newEntry;
                }
            }
        }
        // FalseSharing: update existing list only if access to a new offset
        if (newOffsetAccess) {
            for (FSGAEntry entry : firstAccessOffset) {
                existingOffsets.push_back(entry);
            }
        }
        // FalseSharing: update the access metadata entry
        currEntry.setOffsetEntryList(existingOffsets);
        // FalseSharing: update tick of last access
        currEntry.tick_at_last_access = curTick();
        glb_act_data[set_index][block_num] = currEntry;
    } else {
        PerBlockEntry* currEntry = new PerBlockEntry();
        std::vector<FSGAEntry> blockEntries;
        for (int i = 0; i < BLOCKSIZE / tracking_width; i++) {
            if (accessMetadata.blockAccessWrite[i] ||
                accessMetadata.blockAccessRead[i]) {
                FSGAEntry* newEntry = new FSGAEntry();
                newEntry->offset = i;
                if (accessMetadata.blockAccessWrite[i])
                    newEntry->setLastWriterID(coreID);

                if (accessMetadata.blockAccessRead[i]) {
                    // FalseSharing: optimize reader info
                    if (opt_readers) {
                        // FalseSharing: since first access to byte
                        // simple update readerID
                        newEntry->setLastReaderID(coreID);
                        newEntry->setMultipleReadersStatus(false);
                    } else {
                        newEntry->updateSharer(coreID);
                    }
                }
                blockEntries.push_back(*newEntry);
                // delete newEntry;
            }
        }
        currEntry->setOffsetEntryList(blockEntries);
        currEntry->tick_at_last_access = curTick();

        if (glb_act_data[set_index].size() < assoc_act || assoc_act == 0)
            glb_act_data[set_index][block_num] = *currEntry;
        else { // FalseSharing: replace the block
            Addr victimKey = findVictimAcTEntry(set_index);
            if (glb_act_data[set_index][victimKey].isPrivatized) {
                inform("A privatized block %#x is being replaced by"
                " %#x", victimKey, block_num);
            }
            glb_act_data[set_index].erase(victimKey);
            glb_act_data[set_index][block_num] = *currEntry;
            victim_addr = victimKey;
            ++m_sam_eviction; // Eviction of a SAM entry
        }
        // delete currEntry;
    }

    return victim_addr;
}

void
FSGlobalACT::removeFSGAEntry(Addr addr)
{
    int set_index = findSetAct(addr);
    Addr block_num = makeLineAddress(addr);
    auto it = glb_act_data[set_index].find(block_num);

    // if entry exists then remove it
    if (it != glb_act_data[set_index].end())
        glb_act_data[set_index].erase(block_num);
}

// FalseSharing: update to use the set indexing
bool
FSGlobalACT::blockEntryExists(Addr addr)
{
    bool found = false;
    int set_index = findSetAct(addr);
    Addr block_num = makeLineAddress(addr);

    // FalseSharing: set associative changes
    auto iter = glb_act_data[set_index].find(block_num);
    if (iter != glb_act_data[set_index].end())
        found = true;

    return found;
}

bool
FSGlobalACT::checkPrivatizationStatus(Addr addr, Addr inst_pc)
{
    int set_index = findSetAct(addr);
    Addr block_num = makeLineAddress(addr);
    bool privatization = false;
    ++m_sm_read; // FalseSharing: read the bit
    if (blockEntryExists(addr)) {
        // FalseSharing: set associative changes
        PerBlockEntry currEntry = glb_act_data[set_index][block_num];
        // FalseSharing: update tick of last access
        glb_act_data[set_index][block_num].tick_at_last_access = curTick();

        // if (currEntry.checkTrueSharing()) {
        //    conflictStatus = true;
        // } else {
        //     // Iterate over each accessed byte of block and identify whether
        //     // TS instance
        //     std::vector<FSGAEntry> existingOffsets =
        //         currEntry.getOffsetEntryList();

        //     for (FSGAEntry curr : existingOffsets) {
        //         int coreID = curr.getLastWriterID();
        //         if (coreID != -1) { // a last writer for byte
        //             IntVec sharers = curr.getSharerIDs();
        //             // If reader exists TS
        //             IntVec::iterator it;
        //             for (it = sharers.begin(); it != sharers.end(); ++it) {
        //                 if (*it != coreID) {
        //                     conflictingAccess = true;
        //                     break;
        //                 }
        //             }
        //             if (conflictingAccess) {
        //                 break;
        //             }
        //         }
        //     }
        // }

        // Conflict identified during MD update,
        // simply checking TS bit suffice
        if (!currEntry.checkTrueSharing()) {
            //if (report_pc) {
            inform("Block %#x  and inst %#x participates in false sharing"
            " repair at tick: %llu\n", block_num, inst_pc, curTick());
            //}
            privatization = true;
            // FalseSharing: Reset the access metadata, Now resettting SAM
            // entry after arrival of UP_MD from all sharer
            // resetMetadataonPrivatization(block_num);
            // FalseSharing: set private bit
            glb_act_data[set_index][block_num].isPrivatized = true;

            // FalseSharing: Add block to privatization stats
            // FalseSharing: log enabled privatization length
            glb_act_data[set_index][block_num].prvStart = curTick();

        } else {
            // FalseSharing: true sharing, reset metadata,
            // start new epoch of false sharing
            // FalseSharing: privatization does not materialize due to TS bit
            m_prv_ts_bit++;
            resetMetadataEntry(block_num);
        }
    }
    return privatization;
}

/**
 * FalseSharing: Update GA entry access metadata based on GETX/GETS/UPGRADE
 * @Param:  addr                    physical address for request
 *          accessed_byte_num       byte requested for read or write
 *          machineID               Requestor coreID and MachineType
 *          readOrWrite             Identify an LOAD/STORE, True: LD, False: ST
 */
void
FSGlobalACT::updateGAEntryonRequest(Addr addr, int accessed_byte_num,
                                    MachineID machineId, bool readOrWrite)
{
    int set_index = findSetAct(addr);
    int coreID = machineId.getNum();
    int startOffset = getOffset(addr);
    Addr block_num = makeLineAddress(addr);

    PerBlockEntry currEntry = glb_act_data[set_index][block_num];

    std::vector<FSGAEntry> existingOffsets = currEntry.getOffsetEntryList();
    std::vector<FSGAEntry> firstAccessOffset;

    int startIndex = startOffset / tracking_width;
    int lastIndex = (startOffset + accessed_byte_num - 1) / tracking_width;

    // FalseSharing: the first access to a byte by a core
    // entry does not exist in the metadata table
    bool firstAccess = true;
    bool newOffsetAccess = false;
    for (int metadataIndex = startIndex; metadataIndex <= lastIndex;
         metadataIndex++) {
        std::vector<FSGAEntry>::iterator it;
        for (it = existingOffsets.begin(); it != existingOffsets.end(); ++it) {
            if (it->offset == metadataIndex) {
                firstAccess = false; // FalseSharing: entry exist
                if (readOrWrite) {
                    // update reader if not the last writer
                    if ((it->getLastWriterID() != -1) &&
                        (it->getLastWriterID() != coreID)) {
                        currEntry.setTrueSharing();
                    }
                    if (opt_readers) {
                        if (!(it->getMultipleReadersStatus())) {
                            if (it->getLastReaderID() != -1 &&
                                it->getLastReaderID() != coreID) {
                                it->setMultipleReadersStatus(true);
                            } else
                                it->setLastReaderID(coreID);
                        }
                    } else
                        it->updateSharer(coreID);

                } else {
                    // FalseSharing: Identify the WW instance of true sharing
                    // while updating last writer
                    if ((it->getLastWriterID() != -1) &&
                        (it->getLastWriterID() != coreID)) {
                        currEntry.setTrueSharing();
                    }
                    it->setLastWriterID(coreID);
                }
                break;
            }
        }
        // FalseSharing: first to the byte of the block,
        // add an entry
        if (firstAccess) {
            FSGAEntry* newEntry = new FSGAEntry();
            newEntry->offset = metadataIndex;
            if (readOrWrite) {
                // update reader if not the last writer
                if (opt_readers) {
                    newEntry->setLastReaderID(coreID);
                    newEntry->setMultipleReadersStatus(false);
                } else
                    newEntry->updateSharer(coreID);
            } else
                newEntry->setLastWriterID(coreID);
            firstAccessOffset.push_back(*newEntry);
            newOffsetAccess = true;
        }
    }

    // FalseSharing: update existing list only if access to a new offset
    if (newOffsetAccess) {
        for (FSGAEntry entry : firstAccessOffset) {
            existingOffsets.push_back(entry);
        }
    }
    // persist reader writer changes to list in existing entry
    currEntry.setOffsetEntryList(existingOffsets);
    // persist the changes to the table
    glb_act_data[set_index][block_num] = currEntry;
}

// FalseSharing: TODO:: test out writemask
WriteMask
FSGlobalACT::generateMask(Addr addr, MachineID machineId)
{
    int set_index = findSetAct(addr);
    int coreID = machineId.getNum();
    WriteMask writeback_mask = WriteMask();
    Addr block_num = makeLineAddress(addr);

    PerBlockEntry currEntry = glb_act_data[set_index][block_num];

    std::vector<FSGAEntry> existingOffsets = currEntry.getOffsetEntryList();
    for (FSGAEntry curr : existingOffsets) {
        if (curr.getLastWriterID() == coreID) {
            // FalseSharing: prefdefined Len is provided as 1.
            // With granularity G, each offset correspond to G bytes
            // offset will be effectively offset*G
            // Len will be G
            // TODO: verify the mask
            // old code: writeback_mask.setMask(curr.offset, 1);
            writeback_mask.setMask(curr.offset * tracking_width,
                                   tracking_width);
        }
    }

    return writeback_mask;
}

// FalseSharing: utilize the function to mark true sharing
// FalseSharing: The method use to identify conflict
// on a getX getS to a private block
bool
FSGlobalACT::checkAccessConflict(Addr phyAddr, int numOfBytes,
                                 MachineID machineId, bool opType)
{
    int set_index = findSetAct(phyAddr);
    bool conflictStatus = false;
    int coreID = machineId.getNum();
    Addr block_num = makeLineAddress(phyAddr);
    int startOffset = getOffset(phyAddr);
    int startIndex = startOffset / tracking_width;
    int lastIndex = (startOffset + numOfBytes - 1) / tracking_width;
    // FalseSharing: access conflict check
    ++m_sm_read;
    PerBlockEntry currEntry = glb_act_data[set_index][block_num];
    // FalseSharing: update tick of last access
    glb_act_data[set_index][block_num].tick_at_last_access = curTick();

    std::vector<FSGAEntry> existingOffsets = currEntry.getOffsetEntryList();

    for (FSGAEntry curr : existingOffsets) {
        for (int index = startIndex; index <= lastIndex; index++) {
            if (curr.offset == index) {
                // Irrespective of LD or ST, last writer other than core
                // denotes a conflict
                if (curr.getLastWriterID() != -1 &&
                    curr.getLastWriterID() != coreID) {
                    currEntry.setTrueSharing();
                    conflictStatus = true;
                    break;
                } else {
                    if (opt_readers && (!opType)) {
                        if (!curr.getMultipleReadersStatus()) {
                            if (curr.getLastReaderID() != coreID)
                                currEntry.setTrueSharing();
                        } else {
                            currEntry.setTrueSharing();
                        }
                    } else {
                        for (int i = 0; i < curr.getSharerIDs().size(); i++) {
                            if (coreID != curr.getSharerIDs()[i] && !opType) {
                                // FalseSharing: A WR/RW true sharing
                                currEntry.setTrueSharing();
                                conflictStatus = true;
                                break;
                            }
                        }
                    }
                    if (conflictStatus)
                        break;
                }
            }
        }
        if (conflictStatus)
            break;
    }
    if (conflictStatus) {
        // inform("Block %#x termination in progress. %llu\n", block_num,
        //        curTick());
        // FalseSharing: update the end tick for privatization region
        inform("Block %#x has length of privatization episode: %llu\n",
               block_num, curTick() - currEntry.prvStart);
    } else {
        // update MD in globalAccess table only if no conlfict
        updateGAEntryonRequest(phyAddr, numOfBytes, machineId, opType);
    }

    DPRINTF(PrvDebug, "Block Addr: %#x coreID:%s R/W:%d\n", phyAddr, machineId,
            opType);

    return conflictStatus;
}

// FalseSharing: resetting of MD on every N cycle not a goood
// choice
void
FSGlobalACT::resetMetadataEntry(Addr addr)
{
    int set_index = findSetAct(addr);
    Addr block_num = makeLineAddress(addr);
    // FalseSharing: reset entire entry two update
    // i) read entire row ii) write entire row
    // ++m_sm_read; // the write is added to read energy cal
    ++m_sm_write;
    PerBlockEntry currEntry = glb_act_data[set_index][block_num];
    // FalseSharing: reset MD only if TS bit is set
    if (currEntry.checkTrueSharing()) {
        std::vector<FSGAEntry> existingOffsets =
            currEntry.getOffsetEntryList();
        currEntry.isPrivatized = false;

        currEntry.trueSharing = false;
        currEntry.tick_at_last_access = curTick();
        std::vector<FSGAEntry>::iterator it;
        for (it = existingOffsets.begin(); it != existingOffsets.end(); ++it) {
            it->setLastWriterID(-1);
            if (opt_readers) {
                it->setLastReaderID(-1);
                it->setMultipleReadersStatus(false);
            } else
                it->resetSharerIDs();
        }
        currEntry.setOffsetEntryList(existingOffsets);
    }

    glb_act_data[set_index][block_num] = currEntry;
}

void
FSGlobalACT::resetMetadataonPrivatization(Addr addr)
{
    int set_index = findSetAct(addr);
    Addr block_num = makeLineAddress(addr);
    // FalseSharing: entire read vector and write vector will reset
    // FalseSharing: one write for a block reset in one go
    ++m_sm_write;
    // FalseSharing: one write for setting private bit.
    // ++m_sm_write;
    PerBlockEntry currEntry = glb_act_data[set_index][block_num];
    std::vector<FSGAEntry> existingOffsets = currEntry.getOffsetEntryList();
    currEntry.isPrivatized = true;
    // FalseSharing: required to capture the false sharing arising
    currEntry.trueSharing = false;
    currEntry.tick_at_last_access = curTick();
    std::vector<FSGAEntry>::iterator it;
    for (it = existingOffsets.begin(); it != existingOffsets.end(); ++it) {
        // FalseSharing: consider one write per reset
        it->setLastWriterID(-1);
        if (opt_readers) {
            it->setLastReaderID(-1);
            it->setMultipleReadersStatus(false);
        } else
            it->resetSharerIDs();
    }
    // currEntry.setOffsetEntryList(existingOffsets);
    // removeFSGAEntry(block_num);
    // PerBlockEntry* newEntry = new PerBlockEntry();
    // newEntry->isPrivatized = true;
    // newEntry->trueSharing = false;
    // newEntry->tick_at_last_access = curTick();
    // glb_act_data[set_index][block_num] = *newEntry;
}

void
FSGlobalACT::print(std::ostream& out) const
{
    for (int i = 0; i < glb_act_data.size(); ++i) {
        for (const auto& it : glb_act_data[i]) {
            out << " " << it.first << " " << it.second;
        }
    }
}

// FalseSharing: find the set the entry belongs
int
FSGlobalACT::findSetAct(Addr addr)
{
    // FalseSharing: infinite size AcT table
    if (num_set_bits == 0)
        return 0;
    else
        return bitSelect(addr, offset_bit_index,
                         offset_bit_index + num_set_bits - 1);
}

Addr
FSGlobalACT::findVictimAcTEntry(int index_set)
{
    std::unordered_map<Addr, PerBlockEntry>::iterator it;
    Addr victimAddr = 0;
    Tick currentTick = curTick();
    // check for all entry within set
    for (it = glb_act_data[index_set].begin();
         it != glb_act_data[index_set].end(); ++it) {
        // FalseSharing: other approach could be to replace first victim found
        if (it->second.getLastAccessTick() < currentTick) {
            victimAddr = it->first;
            currentTick = it->second.getLastAccessTick();
        }
    }
    if (glb_act_data[index_set][victimAddr].isPrivatized) {
        inform("A privatized block %#x is being replaced",
         victimAddr);
    }
    return victimAddr;
}

/**
 * @brief check if block experience TS or not
 *
 * @param addr
 * @return true : TS
 * @return false
 */

bool
FSGlobalACT::checkTrueSharingBit(Addr addr)
{
    int set_index = findSetAct(addr);
    Addr block_num = makeLineAddress(addr);
    bool isTrueSharing = false;
    if (blockEntryExists(addr)) {
        PerBlockEntry currEntry = glb_act_data[set_index][block_num];
        isTrueSharing = currEntry.checkTrueSharing();
    }
    // FalseSharing: if entry does not exist
    // TS bit should be considered as false to enable MD comm
    return isTrueSharing;
}

void
FSGlobalACT::resetAcT()
{
    // FalseSharing: iterate over each set and reset
    // entry that are not private.
    std::unordered_map<Addr, PerBlockEntry>::iterator it;
    for (int i = 0; i < glb_act_data.size(); ++i) {
        for (it = glb_act_data[i].begin(); it != glb_act_data[i].end(); ++it) {
            if (!((it->second).isPrivatized))
                (it->second).resetEntry();
        }
    }
}

Addr
FSGlobalACT::getMaxAddrValue()
{
    return maxAddrValue;
}

PerBlockEntry
FSGlobalACT::getGAEntry(Addr addr)
{
    int set_index = findSetAct(addr);
    Addr block_num = makeLineAddress(addr);
    PerBlockEntry currEntry;
    if (blockEntryExists(addr)) {
        currEntry = glb_act_data[set_index][block_num];
    } else {
        warn("MD entry does not exists for private block.\n");
    }
    return currEntry;
}

WriteMask
FSGlobalACT::generateMaskForMetadata(Addr addr, MachineID machineId,
                                     PerBlockEntry metadataEntry)
{
    int coreID = machineId.getNum();
    WriteMask writeback_mask = WriteMask();

    PerBlockEntry currEntry = metadataEntry;

    std::vector<FSGAEntry> existingOffsets = currEntry.getOffsetEntryList();
    for (FSGAEntry curr : existingOffsets) {
        if (curr.getLastWriterID() == coreID) {
            // FalseSharing: prefdefined Len is provided as 1.
            // With granularity G, each offset correspond to G bytes
            // offset will be effectively offset*G
            // Len will be G
            // TODO: verify the mask
            // old code: writeback_mask.setMask(curr.offset, 1);
            writeback_mask.setMask(curr.offset * tracking_width,
                                   tracking_width);
        }
    }

    return writeback_mask;
}

void
FSGlobalACT::resetMetadataEntryForCore(Addr addr, MachineID machineId)
{
    int set_index = findSetAct(addr);
    Addr block_num = makeLineAddress(addr);
    int coreID = machineId.getNum();
    PerBlockEntry currEntry = glb_act_data[set_index][block_num];
    std::vector<FSGAEntry> existingOffsets = currEntry.getOffsetEntryList();
    std::vector<FSGAEntry>::iterator it;
    currEntry.tick_at_last_access = curTick();

    for (it = existingOffsets.begin(); it != existingOffsets.end(); ++it) {
        // FalseSharing: consider one write per reset
        if (it->getLastWriterID() == coreID) {
            it->setLastWriterID(-1);
        } else {
            // FalseSharing
            if (!opt_readers) {
                it->removeSharer(coreID);
            }
        }
    }
    glb_act_data[set_index][block_num] = currEntry;
}

void
FSGlobalACT::reportFSInst(Addr inst_pc)
{
    if (report_pc)
        inform("Inst %#x participates in false sharing\n", inst_pc);
}

void
FSGlobalACT::reportFSVAdrr(PacketPtr virtPkt)
{
    if (report_pc && virtPkt->req != NULL) {
        inform("Virtual Addr %#x participates in false sharing\n",
               virtPkt->req->getVaddr());
        inform("Physical Addr %#x participates in false sharing\n",
               virtPkt->req->getPaddr());
    }
}


bool
FSGlobalACT::isKernelAddr(Addr inst_pc)
{
    if ( inst_pc >= ULL(18446744071562067968)
        && inst_pc <= ULL(18446744072098938879)) {
            return true;
    } else
        return false;

}
/*
void
FSGlobalACT::startup()
{
    // Before simulation starts, we need to schedule the event
    schedule(event, reset_interval);
}

// FalseSharing: action to perform on event trigger
void
FSGlobalACT::metadataResetEvent()
{
    // FalseSharing: reset access metadata
    if (glb_act_data.size() > 0) {
    // if (timeLeft > 0 && glb_act_data.size()) {
        //timeLeft--;
        // resetAcT();
        inform("Firing the event access metadata event.");
        // reschedule event
        schedule(event, curTick() + reset_interval); // reset_interval
    } else {
        inform("Done firing event\n");
    }
}
*/
FSGlobalACT*
FSGlobalACTParams::create()
{
    return new FSGlobalACT(this);
}

// FalseSharing: add a "m_" to stats as GEM5
// automatically append to variable name
void
FSGlobalACT::regStats()
{
    SimObject::regStats();

    m_metadata_msg.name(name() + ".metadata_msg")
        .desc("Metadata message with own access metadata by cores");

    m_control_metadata_msg.name(name() + ".control_metadata_msg")
        .desc("Control MD message to sync pending MD count");

    m_total_metadata_msg.name(name() + ".total_metadata_msg")
        .desc("All types of metadata messages");

    m_eviction_metadata_msg.name(name() + ".eviction_metadata_msg")
        .desc("Metadata message sent by core on eviction");

    // total_metadata_msg = control_metadata_msg + metadata_msg;
    m_sm_read.name(name() + ".total_sam_read")
        .desc("total read to the shared metadata table");

    m_sm_write.name(name() + ".total_sam_write")
        .desc("total write to the shared metadata table");

    m_prv_line.name(name() + ".prv_line")
        .desc("total number of line reported as falsely shared");

    m_total_term.name(name() + ".total_term")
        .desc("total termination of privatize line");

    m_term_cnft.name(name() + ".term_cnft")
        .desc("total termination of privatize block due to conflict");

    m_term_dir_ev.name(name() + ".term_dir_ev")
        .desc("total termination of privatize block due to directory entry"
         " eviction");

    m_term_sam_ev.name(name() + ".term_sam_ev")
        .desc("total termination of privatization line due to SAM entry"
         " eviction");

    m_imm_prv_term.name(name() + ".immediate_termination")
        .desc("termination immediately after privatizing the line");

    m_sam_eviction.name(name() + ".sam_eviction")
        .desc("total sam entry evictions");

    m_succ_prv_line.name(name() + ".succ_prv_line")
        .desc("lines successfully privatized by repair");

    m_evc_prv_entry.name(name() + ".evc_prv_entry")
        .desc("cache lines in private state evicted by Dir");

    m_prv_ts_bit.name(name() + ".prv_ts_bit")
        .desc("prv instance prevented by TS bit");

    m_prv_hys_cnt.name(name() + ".prv_hys_cnt")
        .desc("prv instance prevented by hysteresis counter");

    m_total_false_sharing_accesses.name(name() +
        ".total_false_sharing_accesses").desc("tracks the total acesses to"
        "block involved in false sharing");
}
