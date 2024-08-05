#ifndef __MEM_RUBY_STRUCTURES_FS_GA_ENTRY_HH__
#define __MEM_RUBY_STRUCTURES_FS_GA_ENTRY_HH__

#include <cstdint>
#include <iostream>

#include "mem/ruby/common/IntVec.hh"

// FalseSharing:  Global access tracking entry
class FSGAEntry
{
  private:
    int lastWriterID;
    IntVec sharerIDs;
    // FalseSharing: used if 'opt_readers' option is passed from cmd
    int lastReaderID; // track the reader
    bool multipleReadersStatus; // If more than one reader exists for the byte

  public:
    int offset;
    FSGAEntry();
    ~FSGAEntry();
    int getLastWriterID();
    void setLastWriterID(int lastWriterID);
    IntVec getSharerIDs();
    void setSharerIDs(IntVec sharerIDs);
    void updateSharer(int sharer);
    void resetSharerIDs();
    void removeSharer(int sharer);
    void print(std::ostream& out) const;
    int getLastReaderID();
    void setLastReaderID(int readerID);
    bool getMultipleReadersStatus();
    void setMultipleReadersStatus(bool status);
    void resetReadersInfo();
};
std::ostream& operator<<(std::ostream& os, const FSGAEntry& obj);
#endif // __MEM_RUBY_STRUCTURES_FS_GA_ENTRY_HH__
