#ifndef READS_GET_H
#define READS_GET_H

#include "util_get.h"

int read_partition_table_get(int fd, struct partition_table_entry *entries, off_t start, Config *config);

int read_superblock_get(int fd, struct superblock* superblock_entry, int start);
int read_inode_get(int fd, struct inode *inode, off_t inode_start, int inode_num);

int read_zone_get(int fd, int zone_addr, int zonesize, void* zone);
#endif