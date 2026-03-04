#include <stdint.h>


#define PARTITION_TABLE_OFFSET 0x1BE
#define PARTITION_TABLE_SIZE 512
#define MBR_SIZE 512
#define NUM_PARTITIONS 4

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

int parse_args(int argc, char *argv[], Config *config);

int read_partition_table(int fd, struct partition_table_entry *entries, size_t start, Config *config);
