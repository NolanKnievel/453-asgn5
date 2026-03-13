#ifndef SEARCHES_H
#define SEARCHES_H

#include "reads.h"

int file_search_datazone(int zonesize,
        struct directory* zone_data,
        unsigned char* target,
        struct directory* dir
        );


int search_all(int fd,
    Config* config, 
    uint16_t blocksize,
    struct inode* inode,
    int inode_start, 
    int zonesize, 
    struct directory* final_dir,
    struct inode* final_inode
    );

int search_direct(int fd, 
    struct inode* parent, 
    Config* config, 
    int zonesize,
    int inode_start,
    struct directory* final_dir
    );

// int search_indirect(int fd, 
//     struct inode* inode,
//     Config* config, 
//     uint16_t blocksize,
//     int zonesize, 
//     struct directory* final_dir
//     );

// int search_2x_indirect(int fd, 
//     struct inode* inode,
//     Config* config, 
//     uint16_t blocksize,
//     int zonesize, 
//     struct directory* final_dir
//     );
#endif