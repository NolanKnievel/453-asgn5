#ifndef SEARCHES_H
#define SEARCHES_H

#include "reads.h"

#define POINTER_SIZE_BYTES 4

uint32_t get_file_zone(int fd, struct superblock *sb, 
    struct inode *node, uint32_t index, uint32_t fs_start);

int search_file_zone(unsigned char* target, 
    struct directory* dirs, uint32_t zone_size, 
    struct directory* final_dir);

int search_zones(int fd, struct superblock* sb,
     struct inode* inode, struct directory* final_dir, 
     unsigned char *target, int fs_start);

int search_all(int fd, struct superblock *sb, 
    struct inode *node, uint32_t fs_start, uint32_t inode_start,
     char* path, struct directory* final_dir, struct inode* final_inode);

#endif