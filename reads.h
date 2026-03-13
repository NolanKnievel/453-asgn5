#ifndef READS_H
#define READS_H

#include "util.h"

int read_partition_table(int fd, 
    struct partition_table_entry *entries, off_t start, Config *config);

int read_superblock(int fd, 
    struct superblock* superblock_entry, int start);

int read_inode(int fd, struct inode *inode, 
    off_t inode_start, int inode_num);

int read_zone(int fd, struct superblock *sb, 
    uint32_t zone, void *buf, uint32_t fs_start);

int read_block_of_zone(int fd, off_t zone_addr, 
    int blocksize, void* block_data);

#endif