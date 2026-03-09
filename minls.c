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
    int partition_addr = 0;
    if (read_partition_table(fd, partition_entries, 0, &config) == -1) {
        // no partition table
        if(config.part != -1) {
            fprintf(stderr, "Partition specified but no partition table found\n");
            return 1;
        }
    }
    else{
        //this address gets us to partition table (if it exists)
        partition_addr = partition_entries[config.part - 1].lFirst * BYTES_PER_SECTOR;
    }

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
    struct inode *root_inode = malloc(sizeof(struct inode));
    if (root_inode == NULL) {
        perror("malloc");
        return 1;
    }

    off_t inode_start =
        partition_addr +
        (2 + superblock_entry.i_blocks + superblock_entry.z_blocks)
        * superblock_entry.blocksize;

    // read inode 1 (root)
    if (read_inode(fd, root_inode, inode_start, 1, &config) == -1) {
        fprintf(stderr, "Failed to read inode 1\n");
        free(root_inode);
        return 1;
    }
    //make sure root is a directory
    if(dir_check(root_inode) == 0){
        fprintf(stderr, "Root is not directory\n");
        return 1;
    }
    //if path not given
    uint32_t last_inum = 1;
    if(strcmp(config.path, " ") == 0){
        //print root directory contents
        printf("PRINTING ROOT DIR\n");
        print_macros(fd, &superblock_entry, root_inode, inode_start, 1);
        return 0;
    }
    //else, path is given, traverse path
    else{
        //locals
        int data_start = 0;
        char* temp;
        unsigned char* cur_file;
        char* delim = "/";

        //confirming elements exist
        while((temp = strtok(config.path, delim)) != NULL){
            //calculate first data zone address for the latest inode we've searched
            cur_file = (unsigned char*) temp;
            data_start = calc_datazone_addr(&superblock_entry, last_inum);//
            if((last_inum = traverse_path(fd, &superblock_entry, data_start, cur_file)) == 0){
                fprintf(stderr, "File %s does not exist!\n", cur_file);
                return 1;
            }
        }
        printf("PATH CONFIRMED\n");
        //file exists and fd is reset to reference superblock
    }
    //need to turn last_inode into struct inode to get permissions
    //off_t inode_addr = (off_t)(inode_start + (last_inode * INODE_SIZE_BYTES));
    
    //need to get directory to read inode number and name
    
    /*METHOD 1, pread() or lseek()???
    - move file descriptor to directories (inode's data zone)
    - read and save inode number and name
    - move file descriptor back to its inode struct
    - read and print permissions
    - print inode number and name
    */
    print_macros(fd, &superblock_entry, root_inode, inode_start, last_inum);

    // cleanup
    close(fd);
    free(root_inode);

    printf("hello world!\n");

    return 0;
}

/*
PLAN: 




*/