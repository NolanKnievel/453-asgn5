#ifndef PRINTERS_GET_H
#define PRINTERS_GET_H

#include "reads_get.h"
#include <time.h>

int print_permissions_get(struct inode* inode_entry);

int print_name_get(unsigned char* name);

int print_inum_get(uint32_t inum);

int print_inode_get(struct inode* inode);

int print_path_get(Config* config);

int print_superblock_get(struct superblock* sb);

int print_macros_dir_get(int fd, uint16_t firstdata, struct inode* inode, int zonesize, off_t inode_start, off_t data_start);

int print_macros_file_get(struct directory* dir, struct inode* parent);

#endif