#ifndef PRINTERS_H
#define PRINTERS_H

#include "reads.h"
#include "searches.h"
#include <time.h>

int print_permissions(struct inode* inode_entry);

int print_name(unsigned char* name);

int print_inum(uint32_t inum);

int print_inode(struct inode* inode);

int print_path(Config* config);

int print_superblock(struct superblock* sb);

int print_macros_dir(int fd, struct superblock* sb, struct inode* inode, 
    int zonesize, off_t inode_start, uint32_t fs_start);

int print_macros_file(struct directory* dir, struct inode* parent);

#endif