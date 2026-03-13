#include "reads.h"

/* ----- READER FUNCTIONS ----- */
// read partition table into an array of partition_table_entry, 
// return -1 if the partition table is invalid
// start marks the start of the disk(or partition if we're reading subpartition entries)
int read_partition_table_get(int fd, struct partition_table_entry *entries, off_t start, Config *config) {
    //do we need to consider that a 'byte' can be different on different systems?
    //can only read the signature of ~/HardDisk image on CSL
    uint8_t mbr[MBR_SIZE];

    // read MBR
    if (lseek(fd, start, SEEK_SET) == -1) {
        perror("lseek");
        return -1;
    }
    //printf("reading at: %zu %i bytes\n", start, MBR_SIZE);
    if (read(fd, mbr, MBR_SIZE) != MBR_SIZE) {
        perror("read");
        return -1;
    }
    // debug prints
    if (config->verbose) {
        printf("sig: %02x %02x\n", mbr[510], mbr[511]);
        printf("\n");
    }
    // validate boot signature
    //printf("validating signature...\n");
    if ((mbr[510] != 0x55) || (mbr[511] != 0xAA)) {
        // no partitions
        return -1;
    }
    // copy partition entries
    //printf("copying to pointer...\n");
    memcpy(entries, mbr + PARTITION_TABLE_OFFSET,
           NUM_PARTITIONS * sizeof(struct partition_table_entry));
    
    return 0;
}

int read_superblock_get(int fd, struct superblock* superblock_entry, int start){
    off_t sb_offset = (off_t)(start + SUPERBLOCK_OFFSET);

    //using lseek + read
    if(lseek(fd, sb_offset, SEEK_SET) == -1){
        fprintf(stderr, "superblock lseek\n");
        return -1;
    }
    if(read(fd, superblock_entry, SUPERBLOCK_SIZE_BYTES) == -1){
        fprintf(stderr, "superblock read");
        return -1;
    }
    
    //debug prints
    //printf("superblock magic num: %02x\n", superblock_entry->magic);
    if(superblock_entry->magic != MAGIC_NUM) {
        fprintf(stderr, "magic num\n");
        return -1;
    }
    return 1;
} 

// read inode into struct
// inodes table starting at start
//  return -1 on error
int read_inode_get(int fd, struct inode *inode, off_t inode_start, int inode_num) {
    //seek
    if (lseek(fd, inode_start + (inode_num - 1) * sizeof(struct inode), SEEK_SET) == -1) {
        fprintf(stderr, "lseek\n");
        return -1;
    }
    //reading inode struct
    size_t bytes = read(fd, inode, sizeof(struct inode));

    //error check
    if (bytes != sizeof(struct inode)) {
        fprintf(stderr, "inode read error\n");
        perror("read error in read_inode");
        return -1;
    }
    return 0;
}

//read zone into pointer
//TODO: use for indirect and double-indirect searches?
int read_zone_get(int fd, int zone_addr, int zonesize, void* zone){
    //locals
    int ret = 0;
    //seek
    if(lseek(fd, (off_t)zone_addr, SEEK_SET) == -1){
        fprintf(stderr, "read_zone lseek error\n");
        return -1;
    }
    //read
    if((ret = read(fd, zone, zonesize)) == -1){
        fprintf(stderr, "read zone error\n");
        return -1;
    }
    return 1;
}