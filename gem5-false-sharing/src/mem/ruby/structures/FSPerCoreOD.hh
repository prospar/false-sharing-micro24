/**
 * FalseSharing:
 * Refer: src/learning_gem5/part2/simple_memobj.hh
 *        src/mem/ruby/structures/CacheMemory.hh
 *
 * Yet to implement: - Replacement policy
 *                   - Tag as BankArray
 *
 *
 * naive implementation: fully associative map
 * Later use BankedArray(src/mem/ruby/structures/BankedArray.cc)
 *
 */

#ifndef __MEM_RUBY_STRUCTURES_FS_PER_CORE_OD_HH__
#define __MEM_RUBY_STRUCTURES_FS_PER_CORE_OD_HH__

#include <unordered_map>

#include "mem/ruby/common/Address.hh"
#include "mem/ruby/structures/FSODEntry.hh"
#include "params/FSPerCoreOD.hh"
#include "sim/sim_object.hh" // src/sim/sim_object.hh

class FSPerCoreOD : public SimObject
{
  public:
    uint16_t od_size;
    // Addr define in python/m5/param.py
    std::unordered_map<Addr, FSODEntry> od_state_data;

    FSPerCoreOD(FSPerCoreODParams* params);
    ~FSPerCoreOD();

    void setFSODEntry(Addr addr);

    bool updateFSODEntry(Addr addr);

    bool unsetFSODEntry(Addr addr);

    void updatePFSBit(Addr, bool);

    void updateCoherenceMissBit(Addr, bool);

    bool blockPresent(Addr addr);

    bool verifyPFSandCM(Addr addr);

    bool isCoherenceMiss(Addr addr);
};

#endif // __MEM_RUBY_STRUCTURES_FS_PER_CORE_OD_HH__
