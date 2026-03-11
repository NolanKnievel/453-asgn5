#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <stdio.h>

#define DIRECTORY_MASK 0040000
#define OWNER_R 0000400
#define OWNER_W 0000200
#define OWNER_E 0000100
#define REGULAR_FILE_MASK 0100000



#define DIRECT_ZONES 7
#define DIR_STRUCT_SIZE_BYTES 64
#define MAX_DIR_NAME_SIZE_BYTES 60
#define INODE_SIZE_BYTES 40

#define BYTES_PER_SECTOR 512

#define SUPERBLOCK_OFFSET 1024
#define SUPERBLOCK_SIZE_BYTES 31
#define NUM_SUB_PARTITIONS 4
#define MAGIC_NUM 0x4D5A
#define MAGIC_NUM_REV 0x5A4D

#define PARTITION_TABLE_OFFSET 0x1BE
#define PARTITION_TABLE_SIZE 512
#define MBR_SIZE 512
#define NUM_PARTITIONS 4

#define DIRECT_ZONES 7

//still deciding whether we need this or not, same amount of lines
#define SUPERBLOCK_CONTENTS "Superblock Contents:\n \
        Store Fields:\n\
        ninodes           %u\n \
        i_blocks          %u\n \
        z_blocks          %d\n \
        firstdata         %u\n \
        log_zone_size     %d\n \
        max_file          %u\n \
        zones             %u\n \
        magic             %04x\n \
        blocksize         %u\n \
        subversion        %u\n\n"

// struct to store command line arguments
typedef struct {
    int verbose; 
    int part; // defaults to -1 if not provided - no partition
    int subpart; // defaults to -1 if not provided - no subpartition
    char *imagefile; // required
    char *path; // defaults to '/' if not provided. '/' added to paths not including one
} Config;


struct __attribute__((packed)) partition_table_entry {
    uint8_t bootind; // Boot magic number (0x80 if bootable) - IGNORE
    uint8_t start_head; // Start of partition in CHS - IGNORE
    uint8_t start_sec; //  - IGNORE
    uint8_t start_cyl; // - IGNORE
    uint8_t type; // Type of partition (0x81 is minix) - USE THIS
    uint8_t end_head; // End of partition in CHS - IGNORE
    uint8_t end_sec; // - IGNORE
    uint8_t end_cyl; // - IGNORE
    uint32_t lFirst; // First sector (LBA addressing) - USE THIS
    uint32_t size; // size of partition (in sectors) - USE THIS
};


struct __attribute__((packed)) superblock {
    /* on disk. These fields and orientation are non–negotiable */
    uint32_t ninodes; /* number of inodes in this filesystem */
    uint16_t pad1; /* make things line up properly */
    int16_t i_blocks; /* # of blocks used by inode bit map */
    int16_t z_blocks; /* # of blocks used by zone bit map */
    uint16_t firstdata; /* number of first data zone */
    int16_t log_zone_size; /* log2 of blocks per zone */
    int16_t pad2; /* make things line up again */
    uint32_t max_file; /* maximum file size */
    uint32_t zones; /* number of zones on disk */
    int16_t magic; /* magic number */
    int16_t pad3; /* make things line up again */
    uint16_t blocksize; /* block size in bytes */
    uint8_t subversion; /* filesystem sub–version */
};

struct __attribute__((packed)) inode {
    uint16_t mode; /* mode */
    uint16_t links; /* number or links */
    uint16_t uid;
    uint16_t gid;
    uint32_t size;
    int32_t atime;
    int32_t mtime;
    int32_t ctime;
    uint32_t zone[DIRECT_ZONES];
    uint32_t indirect;
    uint32_t two_indirect;
    uint32_t unused;
};

struct __attribute__((packed)) directory {
    uint32_t inode;
    unsigned char name[MAX_DIR_NAME_SIZE_BYTES];
};



int parse_args(int argc, char *argv[], Config *config);

int read_partition_table(int fd, struct partition_table_entry *entries, off_t start, Config *config);

int read_superblock(int fd, struct superblock* superblock_entry, int start, Config* config);

int read_inode(int fd, struct inode *inodes, off_t start, int ninodes, Config *config);

int dir_check(struct inode*  inode);

int calc_datazone_addr(struct superblock* superblock_entry, int inum);

// struct inode* inum_2_inode(int fd, off_t inode_base, int inum);

uint32_t traverse_path(int fd, 
        struct superblock* superblock_entry,
        int inode_data_start,
        unsigned char* target);
        
int print_permissions(struct inode* inode_entry);

int print_macros(int fd, struct superblock* superblock_entry, struct inode* parent, off_t inode_base, int inum);

void read_zone(int fd, struct superblock *sb, uint32_t zone, void *buf, uint32_t fs_start);

uint32_t get_file_zone(int fd, struct superblock *sb, struct inode *node, uint32_t index, uint32_t fs_start);

void copy_file(int fd, FILE *dst, struct superblock *sb, struct inode *node, uint32_t fs_start);
