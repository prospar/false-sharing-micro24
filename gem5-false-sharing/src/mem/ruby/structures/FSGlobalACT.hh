/**
 * FalseSharing:  Structure for global access metadata, basically a map from
 * byte address -> GA Entry
 *
 * BlockAddr as a Bank Array
 *
 */

#ifndef __MEM_RUBY_STRUCTURES_FS_GLOBAL_ACT_HH__
#define __MEM_RUBY_STRUCTURES_FS_GLOBAL_ACT_HH__

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "base/logging.hh"
#include "base/statistics.hh"
#include "base/stats/info.hh"
#include "base/types.hh"
#include "debug/GlobalAccess.hh"
#include "debug/PrvDebug.hh"
#include "mem/ruby/common/Address.hh"
#include "mem/ruby/common/MachineID.hh"
#include "mem/ruby/common/WriteMask.hh"

// #include "mem/ruby/structures/FSGAEntry.hh"
#include "mem/ruby/structures/FSPerCoreStateEntry.hh"
#include "mem/ruby/structures/PerBlockEntry.hh"
#include "mem/ruby/structures/PrivatizationStat.hh"
#include "params/FSGlobalACT.hh"
#include "sim/sim_object.hh"

#define BLOCKSIZE 64
// #define HYST_THRESHOLD 4

class FSGlobalACT : public SimObject
{
  private:
    // FalseSharing: event to reset stats every N cycle
    // Do not return any value, no argument
    // void metadataResetEvent();

    // EventFunctionWrapper event;
    // int timeLeft = 10000;
    Addr maxAddrValue = MaxAddr;

  public:
    bool recordStored = false;
    uint32_t global_act_size;
    uint32_t tracking_width;
    // FalseSharing: parallel lookup
    uint32_t assoc_act; // FalseSharing: number of entry per set way;
    // Falsesharing: decide based on size of act table
    uint32_t set_act;     // False Sharing: number of sets:
    int offset_bit_index; // FalseSharing: num of bits
    int num_set_bits;     // FalseSharing: num of bits to address a set
    Tick reset_interval;  // FalseSharing:
    bool opt_readers;     // FalseSharing: enable OPT for reader info
    bool report_pc;       // FalseSharing: enable reporting of inst part of FS
    std::vector<std::unordered_map<Addr, PerBlockEntry>> glb_act_data;

    FSGlobalACT(FSGlobalACTParams* params);
    ~FSGlobalACT();
    // void init();

    void setFSGAEntry(Addr addr, int accessed_byte_num, MachineID machineID,
                      bool readOrWrite);
    Addr updateFSGAEntry(Addr addr, FSPerCoreStateEntry accessMetadata,
                         MachineID machineID);
    void updateGAEntryonRequest(Addr addr, int accessed_byte_num,
                                MachineID machineID, bool readOrWrite);
    PerBlockEntry getGAEntry(Addr addr);
    // FSPerCoreStateEntry getAccessMetadata(Addr addr, MachineID machineID);
    void removeFSGAEntry(Addr addr);
    bool blockEntryExists(Addr addr);
    bool checkPrivatizationStatus(Addr addr, Addr inst_pc);
    WriteMask generateMask(Addr addr, MachineID machineId);
    WriteMask generateMaskForMetadata(Addr addr, MachineID machineId,
                                      PerBlockEntry metadataEntry);
    bool checkAccessConflict(Addr phyAddr, int num_of_bytes,
                             MachineID machineId, bool opType);
    // void updateHysteresisCounter(Addr addr);
    void resetMetadataEntry(Addr addr);
    void print(std::ostream& out) const;
    Addr findVictimAcTEntry(int set_number);
    int findSetAct(Addr Address);
    void resetMetadataonPrivatization(Addr addr);
    bool checkTrueSharingBit(Addr addr);
    void regStats();
    void resetMetadataEntryForCore(Addr, MachineID);
    void resetAcT();
    Addr getMaxAddrValue();
    void reportFSInst(Addr inst_pc);
    void reportFSVAdrr(PacketPtr virtPkt);
    bool isKernelAddr(Addr inst_pc);
    // FalseSharing: refer HelloObject.hh file
    /**
     * Part of a SimObject's initilaization. Startup is called after all
     * SimObjects have been constructed. It is called after the user calls
     * simulate() for the first time.
     */
    // void startup();

    // FalseSharing: stats related implementation
    // add a "m_" before stats name if planning to update a stats from ruby
    // if updating at structure, add any name
  public:
    Stats::Scalar m_metadata_msg; // msg with own access metadata
    Stats::Scalar m_control_metadata_msg;
    Stats::Scalar m_total_metadata_msg;
    Stats::Scalar m_eviction_metadata_msg;
    // FalseSharing: Stats for count total read and write to shared access MD
    // FalseSharing: for energy and area stats
    Stats::Scalar m_sm_read;
    Stats::Scalar m_sm_write;
    Stats::Scalar m_prv_line;
    Stats::Scalar m_total_term;
    Stats::Scalar m_term_cnft;
    Stats::Scalar m_term_dir_ev;
    Stats::Scalar m_term_sam_ev;
    Stats::Scalar m_imm_prv_term;
    Stats::Scalar m_sam_eviction;
    Stats::Scalar m_succ_prv_line;
    Stats::Scalar m_evc_prv_entry;
    Stats::Scalar m_prv_ts_bit;
    Stats::Scalar m_prv_hys_cnt;
    Stats::Scalar m_total_false_sharing_accesses;
    // FalseSharing: define a new stat for
    // Stats::PrvLenInfo* m_record;
};

std::ostream& operator<<(std::ostream& os, const FSGlobalACT& obj);

inline std::ostream&
operator<<(std::ostream& out, const PerBlockEntry& obj)
{
    obj.print(out);
    out << std::flush;
    return out;
}

#endif // __MEM_RUBY_STRUCTURES_FS_GLOBAL_ACT_HH__
