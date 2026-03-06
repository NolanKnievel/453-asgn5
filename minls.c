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
        printf("usage: minls [ -v ] [ -p part [ -s subpart ] ] imagefile [ path ]\n");
        printf("Options: \n");
        printf("  -p part    --- select partition for filesystem (default: none)\n");
        printf("  -s sub     --- select subpartition for filesystem (default: none)\n");
        printf("  -h         --- print usage information and exit\n");
        printf("  -v         --- enable verbose output (default: off)\n");
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

    // get array of subpartitions (optional)
    // global address of specific partition
    //this address gets us to partition table?
    int partition_addr = partition_entries[config.part - 1].lFirst * BYTES_PER_SECTOR;

    if(config.subpart != -1){
        //redefining partition_entries for subpartition
        if(read_partition_table(fd, partition_entries, partition_addr, &config) == -1){
            fprintf(stderr, "Failed to read subpartition table\n");
            return 1;
        } 
        //resetting partition addr
        partition_addr = partition_entries[config.subpart - 1].lFirst * BYTES_PER_SECTOR;
    }

    // get superblock of target partition table
    //printf("fetching superblock...\n");
    struct superblock superblock_entry;
    off_t sb_offset = (off_t)(partition_addr + SUPERBLOCK_OFFSET);

    //using lseek + read
    if(lseek(fd, sb_offset, SEEK_SET) == -1){
        fprintf(stderr, "lseek\n");
        return 1;
    }
    if(read(fd, &superblock_entry, SUPERBLOCK_SIZE_BYTES) == -1){
        fprintf(stderr, "read");
        return 1;
    }
    
    //debug prints
    printf("superblock magic num: %02x\n", superblock_entry.magic);
    if(superblock_entry.magic != MAGIC_NUM) {
        fprintf(stderr, "magic num\n");
        return 1;
    }

    if(config.verbose){
        printf("Superblock Contents:\n");
        printf("Store Fields:\n");
        printf("ninodes           %u\n",  superblock_entry.ninodes);
        printf("i_blocks          %u\n", superblock_entry.i_blocks);
        printf("z_blocks          %d\n", superblock_entry.z_blocks);
        printf("firstdata         %u\n", superblock_entry.firstdata);
        printf("log_zone_size     %d\n", superblock_entry.log_zone_size);
        printf("max_file          %u\n", superblock_entry.max_file);
        printf("zones             %u\n", superblock_entry.zones);
        printf("magic             %04x\n", superblock_entry.magic);
        printf("blocksize         %u\n", superblock_entry.blocksize);
        printf("subversion        %u\n\n", superblock_entry.subversion);
    }
    
    // close image file
    close(fd);

    printf("hello world!\n");

    return 0;
}