#include "ext2.h"
#include "../../utils.h"
#include "../../memory/kheap.h"
#include "../../disk/disk.h"
#include "../../disk/streamer.h"
#include "../../memory/status.h"

#define MAX_DIR_NAME 128

//optionals
#define PREALLOC_FOR_DIR 0x0001 
#define ASF_SERVER 0x0002 
#define FS_HAS_JOURNAL 0x0004 
#define INODES_HAVE_EXTENDED_ATTRS 0x0008 
#define FS_CAN_RESIZE_ITSELF 0x0010 
#define DIRS_USE_HASH_INDEX 0x0020 

//required
#define COMPRESSION_IS_USED 0x0001 
#define DIR_ENTRIES_HAVE_TYPE_FIELD 0x0002 
#define FS_NEEDS_TO_REPLAY_JOURNAL 0x0004 
#define FS_USES_JOURNAL 0x0008 

//read-only features
#define SPARSE 0x0001 
#define FS_USES_64_FILE_SIZE 0x0002 
#define DIRS_CONTENTS_STORED_IN_BINARY_TREE 0x0004 

//inode types
#define TYPE_FIFO 0x1000
#define TYPE_CHARACTER_DEVICE 0x2000
#define TYPE_DIRECTORY 0x4000
#define TYPE_BLOCK_DEVICE 0x6000
#define TYPE_REGULAR_FILE 0x8000
#define TYPE_SYMBOLIC_LINK 0xA000
#define TYPE_UNIX_SOCKET 0xC000

//inode premissions
#define OTHER_EXEC 0x001
#define OTHER_WRITE 0x002
#define OTHER_READ 0x004
#define GROUP_EXEC 0x008
#define GROUP_WRITE 0x010
#define GROUP_READ 0x020
#define USER_EXEC 0x040
#define USER_WRITE 0x080
#define USER_READ 0x100
#define STICKY_BIT 0x200
#define SET_GROUP_ID 0x400
#define SET_USER_ID 0x800

//inode flags
#define SECUTE_DELETION 0x00000001
#define KEEP_COPY_OF_DATA 0x00000002
#define FILE_COMPRESSED 0x00000004
#define SYNCHRONOUS_UPDATES 0x00000008
#define IMMUTABLE_FILE 0x00000010
#define APPEND_ONLY 0x00000020
#define FILE_EXCLUDED_FROM_DUMP 0x00000040
#define HASH_INDEXED_DIR 0x00010000
#define AFS_DIRECTORY 0x00020000
#define JOURNAL_FILE_DATA 0x00040000

//dir entry types
#define DIR_UNKNOWN 0
#define DIR_REGULAR_FILE 1
#define DIR_DIRECTORY 2
#define DIR_CHAR_DEVICE 3
#define DIR_BLOCK_DEVICE 4
#define DIR_FIFO 5
#define DIR_SOCKER 6
#define DIR_SYMBOLIC_LINK 7


struct ext2_superblock {
  uint32_t inodes;
  uint32_t blockNum;
  uint32_t superBlockReserved;
  uint32_t unallocBlocks;
  uint32_t unallocInodes;
  uint32_t superBlockNum;
  uint32_t blockSize;
  uint32_t fragSize;
  uint32_t blocksInGroup;
  uint32_t fragsInGroup;
  uint32_t inodesInGroup;
  uint32_t lastMountTime;
  uint32_t lastWrittenTime;
  uint16_t mountedTimesAfterCheck;
  uint16_t mountedTimesBeforeCheck;
  uint16_t sig; //must be 0xef53. If not - it's not ext2
  uint16_t fsState; // 1 - clean, 2 - errors
  uint16_t error; //1 - ignore error, 2 - remount as read-only, 3 - FUCKING KERNEL PANIC WO
  uint16_t minorVer;
  uint32_t lastCheck;
  uint32_t checkInterval;
  uint32_t OSID; //OS id which created this fs on this volume; 1 - linux, 2- GNU HURD, 3 - MASIX, 4 - FreeBSD, 5 - else
  uint32_t majorVer;
  uint16_t userID;
  uint16_t groupID;
  
  //Those are only if major version >= 1
  uint32_t firstFreeInode; // 11 if version < 1
  uint16_t sizeOfInode; // 128 if version < 1
  uint16_t superBlockGroup;
  uint32_t optionalFeatures;
  uint32_t requiredFeatures;
  char FSID[16];
  char volumeName[16];
  char mountedPath[64];
  uint32_t compressAlg;
  uint8_t blockNumToPreallocForFiles;
  uint8_t blockNumToPreallocForDirs;
  uint16_t unused; //Really, just free space;
  char journalID[16];
  uint32_t journalInode;
  uint32_t journalDevice;
  uint32_t headOfOrphanInodeList;
}__attribute__((packed));

struct ext2_block_group_descriptor{
  uint32_t blockUsageBitmap;
  uint32_t inodeUsageBitmap;
  uint32_t startingBlockAddr;
  uint16_t unallocBlocksInGroup;
  uint16_t unallocInodesInGroup;
  uint16_t dirsInGroup;
  uint8_t unused[14];
}__attribute__((packed));

struct ext2_inode_data{
  uint16_t typeAndPrems;
  uint16_t userID;
  uint32_t lowerSize;
  uint32_t lastAccess;
  uint32_t lastCreationTime;
  uint32_t lastModTime;
  uint32_t delTime;
  uint16_t groupID;
  uint16_t hardLinksCount;
  uint32_t diskSectorsCount;
  uint32_t flags;
  uint32_t OSSpecificValue;
  uint32_t dirBlockPointer[12];
  uint32_t slightlyIndirectBlockPointer;
  uint32_t doublyIndirectBlockPointer;
  uint32_t triplyIndirectBlockPointer;
  uint32_t generationNumber;
  uint32_t fileACL;
  union{
    uint32_t dirACL;
    uint32_t upperSize;
  };
  uint32_t blockAddrOfFrag;
}__attribute__((packed));


struct ext2_dir_entry{
  struct ext2_inode_data *inode;
  uint16_t totalSize;
  uint8_t nameLength;
  uint8_t typeIndicator;
  uint8_t name[MAX_DIR_NAME];
};


struct filesystem ext2_fs = {
        .resolve = 0,
        .open = 0,
        .read = 0,
        .seek = 0,
        .stat = 0,
        .close = 0 
};

struct filesystem *ext2_init(){
  strcpy(ext2_fs.name, "EXT2");
  return &ext2_fs;
}




struct ext2_private{
   struct ext2_superblock sb;
   struct ext2_dir_entry root_dir;
   //struct disk_stream *cluster_read_stream;
   struct disk_stream *read_stream;
   //struct disk_stream *directory_stream;
};

static inline uint32_t ext2_get_inode_group(uint32_t inode_num, struct ext2_private *private){
  return (inode_num - 1) / private->sb.inodesInGroup;
}

static inline uint32_t ext2_get_inode_pos_in_group(uint32_t inode_num, struct ext2_private *private){
  return (inode_num - 1) % private->sb.inodesInGroup;
}

void ext2_init_private(struct disk *disk, struct ext2_private *private){
  disk->fs_private = private;
  disk->fs = &ext2_fs;
}

int ext2_resolve(struct disk *disk){
  int res = 0;
  struct ext2_private *ext_private = kzalloc(sizeof(struct ext2_private));
  if (!ext_private){
    res = -ENOMEM;
    goto out;
  }
  ext2_init_private(disk, ext_private);
  struct disk_stream *stream = disk_streamer_new(disk->id);
  disk_stream_seek(stream, 1024);
  if (!stream){
    res = -ENOMEM;
    goto out;
  }
  if (disk_stream_read(stream, &ext_private->sb, sizeof(struct ext2_superblock))){
    res = -EIO;
    goto out;
  }
  if (ext_private->sb.sig != 0xef53){
    res = -EFSNOTUS;
    goto out;
  }


out:
  
  if (res && ext_private){
    disk->fs_private = 0;
    disk->fs = 0; 
    kfree(ext_private);
  }
  disk_stream_close(stream);
  return res;
}

int ext2_open(struct disk *disk, struct path_part *file, uint32_t mode){
  struct ext2_private *private = disk->fs_private;
  struct ext2_block_group_descriptor bgd;
}
int ext2_read(struct disk *disk, void *private_data, uint32_t size, uint32_t nmemb, char *out){
  struct ext2_private *private = private_data;
  return 0;
}
