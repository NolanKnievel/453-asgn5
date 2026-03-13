#include "reads.h"

/* ----- READER FUNCTIONS ----- */
// read partition table into an array of partition_table_entry, 
// return -1 if the partition table is invalid
// start marks the start of the disk(or partition 
//    if we're reading subpartition entries)

// helper to read a zone
int read_zone(int fd, struct superblock *sb, uint32_t zone, 
    void *buf, uint32_t fs_start)
{
    //printf("READ ZONE\n");
    uint32_t zone_size = sb->blocksize << sb->log_zone_size;
    int ret = 0;
    // if (zone == 0) {
    //     memset(buf, 0, zone_size);
    //     return 1;
    // }

    uint32_t offset =
     fs_start + (zone * (sb->blocksize << sb->log_zone_size));

    lseek(fd, offset, SEEK_SET);
    if((ret = read(fd, buf, zone_size) == -1)){
        printf("read_zone: read error\n");
        return -1;
    }
    // if((uint32_t)ret != zone_size){
    //     printf("read_zone: bytes read != zone_size\n");
    //     return -1;
    // }
    return 1;
}

int read_partition_table(int fd, 
    struct partition_table_entry *entries, off_t start, Config *config) {
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

int read_superblock(int fd, struct superblock* superblock_entry, int start){
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
int read_inode(int fd, struct inode *inode, off_t inode_start, int inode_num){
    //seek
    if (lseek(fd, inode_start + (inode_num - 1) 
        * sizeof(struct inode), SEEK_SET) == -1) {
        fprintf(stderr, "lseek\n");
        return -1;
    }
    //reading inode struct
    ssize_t bytes = read(fd, inode, sizeof(struct inode));
    if(bytes == -1){
        perror("read_inode: read call");
        return -1;
    }
    if(bytes == 0){
        printf("read_inode: zero bytes read of inode struct\n");
        return -1;
    }
    //error check
    if ((unsigned long)bytes != sizeof(struct inode)) {
        fprintf(stderr, "inode read error\n");
        perror("read error in read_inode");
        return -1;
    }
    return 0;
}
