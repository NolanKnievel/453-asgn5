#include "minget.h"


int main(int argc, char *argv[]) {
    Config config;
    char * root = "/";
    config.path = root;
    
    if(argc == 1) {
        printf(USAGE_MESSAGE);
        return 1;
    }
    if(parse_get_args(argc, argv, &config) == -1) {
        return 1;
    }

    // open image file
    int fd = open(config.imagefile, O_RDONLY);
    if (fd == -1) {
        perror("open");
        return 1;
    }
    /* -----FIND PARTITIONS-----*/
    // get array of partition entry structs

    struct partition_table_entry partition_entries[NUM_PARTITIONS];
    int partition_addr = 0;
    int ret1 = 0;
    if ((ret1 = read_partition_table(fd, partition_entries, 0, &config)) == -1) {
        //printf("no partition found\n");
        // no partition table
        if(config.part != -1) {
            fprintf(stderr, "Partition specified but no partition table found\n");
            return 1;
        }
    }
    else if(ret1 == -1){
        fprintf(stderr, "error reading partition table\n");
        return 1;
    }
    else{
        //this address gets us to partition table (if it exists)
        //printf("finding partition addr...\n");
        if(config.part != -1)
        partition_addr = partition_entries[config.part - 1].lFirst * BYTES_PER_SECTOR;
    }

    /* -----FIND SUBPARTITIONS-----*/
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

    /* -----FIND SUPERBLOCK-----*/
    // get superblock of specific partition table
    struct superblock superblock_entry;
    if(read_superblock(fd, &superblock_entry, partition_addr) == -1){
        fprintf(stderr, "superblock read error\n");
        return 1;
    }

    /* -----FIND ROOT INODE STRUCT-----*/
    // read inodes
    struct inode *root_inode = malloc(sizeof(struct inode));
    if (root_inode == NULL) {
        perror("malloc");
        return 1;
    }

    int inode_start =
        partition_addr +
        (2 + superblock_entry.i_blocks + superblock_entry.z_blocks)
        * superblock_entry.blocksize;

    // read inode 1 (root)
    if (read_inode(fd, root_inode, inode_start, 1) == -1) {
        fprintf(stderr, "Failed to read inode 1\n");
        free(root_inode);
        return 1;
    }
    //make sure root is a directory
    if(dir_check(root_inode) == 0){
        fprintf(stderr, "Root is not directory\n");
        return 1;
    }
    /* ----- CALCULATE OFFSETS ----- */
    int zonesize = superblock_entry.blocksize << superblock_entry.log_zone_size;
    off_t data_start = partition_addr + (off_t)(superblock_entry.firstdata * zonesize);
    // data_start ++;

    /* -----PATH NOT GIVEN, ERROR-----*/
    //if path not given
    if(strcmp(config.path, "/") == 0){
        fprintf(stderr, "File is a directory, cannot minget\n");
        return 0;
    }
    /* ----- PATH GIVEN, SEARCH FOR FILE ------*/
    //pointers to hold data of final file in path
    struct directory final_dir = {0};
    struct inode final_inode = {0};
    //want wrapper
    printf("copying: \n");
    int ret = search_all(fd, &config, superblock_entry.blocksize, root_inode, inode_start, zonesize, &final_dir, &final_inode);
    //file not found
    if(ret == 0){
        fprintf(stderr, "search_all: File not found!\n");
        return 1;
    }
    //error
    if(ret == -1){
        fprintf(stderr, "search_all: Error!\n");
        return 1;
    }
    /*----- COPY CONTENTS -----*/
    if(config.verbose){
        printf("Writing contents of %s to %s\n", config.path, config.copy_path ? config.copy_path : "stdout");
    }

    if(regFile_check(&final_inode)){    // check if regular file
        FILE *f = config.copy_path ? fopen(config.copy_path, "w") : stdout;
        copy_file(fd, f, &superblock_entry, root_inode, (uint32_t)partition_addr, &config);
        config.copy_path ? fclose(f) : 0;
    }
    else if(dir_check(&final_inode)){     // file is directory
        fprintf(stderr, "File is a directory, cannot minget\n");
    }
    else {
        fprintf(stderr, "File is not regular file or directory, cannot minget\n");
        return 1;
    }

    // cleanup
    close(fd);
    free(root_inode);

    printf("hello world!\n");

    return 0;
}