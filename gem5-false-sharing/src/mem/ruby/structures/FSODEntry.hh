#ifndef __MEM_RUBY_STRUCTURES_FS_OD_ENTRY_HH__
#define __MEM_RUBY_STRUCTURES_FS_OD_ENTRY_HH__

#include <cstdint>

// FalseSharing:  Overlap detection entry
// TODO: remove invalidation count now maintained at Global Access Tracking
class FSODEntry
{
  private:
    bool isCoherenceMiss;
    bool isPFS;
    uint64_t invalidationCount;

  public:
    FSODEntry();
    ~FSODEntry();
    /**
     * Getter Setter for ODEntry member variable
     */
    void setCoherenceMissBit(bool isCoherenceMiss);
    bool getCoherenceMissBit();
    void setPfsBit(bool isPFS);
    bool getPfsBit();
    void setInvalidationCount(int invalidationCount);
    int getInvalidationCount();
};

#endif //__MEM_RUBY_STRUCTURES_FS_OD_ENTRY_HH__
