#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "minls.h"
#include "util.h"
#include <stdlib.h>

int main(int argc, char *argv[]) {
    Config config;
    char * root = "/";
    config.path = root;
    
    if(argc == 1) {
        printf(USAGE_MESSAGE);
        return 1;
    }
    parse_args(argc, argv, &config);

    // open image file
    int fd = open(config.imagefile, O_RDONLY);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    // get array of partition entry structs
    if(config.verbose) {
        printf("finding primary partitions...\n");
    }
    struct partition_table_entry partition_entries[NUM_PARTITIONS];
    if (read_partition_table(fd, partition_entries, 0, &config) == -1) {
        // no partition table
        if(config.part != -1) {
            fprintf(stderr, "Partition specified but no partition table found\n");
            return 1;
        }
    }

    //this address gets us to partition table
    int partition_addr = partition_entries[config.part - 1].lFirst * BYTES_PER_SECTOR;

    // get array of subpartitions (optional)
    if(config.subpart != -1){
        //redefining partition_entries for subpartition
        if(read_partition_table(fd, partition_entries, partition_addr, &config) == -1){
            fprintf(stderr, "Failed to read subpartition table\n");
            return 1;
        } 
        //resetting partition addr for subpartition
        partition_addr = partition_entries[config.subpart - 1].lFirst * BYTES_PER_SECTOR;
    }

    // get superblock of specific partition table
    //printf("fetching superblock...\n");
    struct superblock superblock_entry;
    if(read_superblock(fd, &superblock_entry, partition_addr, &config) == -1){
        fprintf(stderr, "superblock read error\n");
        return 1;
    }

    // read inodes
    struct inode *inode_1 = malloc(sizeof(struct inode));
    if (inode_1 == NULL) {
        perror("malloc");
        return 1;
    }

    off_t inode_start =
    partition_addr +
    (2 + superblock_entry.i_blocks + superblock_entry.z_blocks)
    * superblock_entry.blocksize;

    // read inode 1 as a test
    if (read_inode(fd, inode_1, inode_start, 1, &config) == -1) {
        fprintf(stderr, "Failed to read inode 1\n");
        free(inode_1);
        return 1;
    }


    // cleanup
    close(fd);
    free(inode_1);

    printf("hello world!\n");

    return 0;
}

/*
PLAN: 




*/