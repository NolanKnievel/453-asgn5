#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "minls.h"
#include "util.h"

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
    printf("finding primary parititions...\n");
    struct partition_table_entry partition_entries[NUM_PARTITIONS];
    if (read_partition_table(fd, partition_entries, 0, &config) == -1) {
        fprintf(stderr, "Failed to read partition table\n");
        return 1;
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
   
    
    // close image file
    close(fd);

    printf("hello world!\n");

    return 0;
}