#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include "util.h"
#include "stdio.h"
#include "stdlib.h"



// parse args and update config struct to match
// ~pn-cs453/Given/Asgn5/Images
int parse_args(int argc, char *argv[], Config *config) {
    int i = 1;

    // set defaults
    config->verbose = 0;
    config->part = -1;
    config->subpart = -1;
    config->imagefile = NULL;

    while(i < argc) {
        // -v
        if(strcmp(argv[i], "-v") == 0) {
            config->verbose = 1;
            i++;
        }
        // -p
        else if(strcmp(argv[i], "-p") == 0) {
            if (i+1 >= argc) {
                fprintf(stderr, "-p requires a part number\n");
                return -1;
            }
            config->part = atoi(argv[i+1]);
            i += 2;

            // -s
            if(strcmp(argv[i], "-s") == 0) {
                if (i+1 >= argc) {
                    fprintf(stderr, "-v requires a subpart number\n");
                    return -1;
                }
                config->subpart = atoi(argv[i+1]);
                i += 2;
            }
        }

        // imagefile
        else if(config->imagefile == NULL) {
            config->imagefile = argv[i];
            i++;
        }
        // path
        else if(strcmp(config->path, "/") == 0) {
            config->path = argv[i];
            i++;
        }
        else {
            fprintf(stderr, "Unexpected arguments, please follow the pattern: \n minls [ -v ] [ -p part [ -s subpart ] ] imagefile [ path ]\n");
            return -1;
        }
    }

    // validate args
    if (config->imagefile == NULL) {
        fprintf(stderr, "Missing required imagefile\n");
        return -1;
    }

    if (config->subpart && !config->part) {
        fprintf(stderr, "-s cannot be used without -p\n");
        return -1;
    }

    // verbose - print parsed config
    if(config->verbose) {
        printf("Config:\n");
        printf("  Verbose: %d\n", config->verbose);
        printf("  Part: %i\n", config->part != -1 ? config->part : -1);
        printf("  Subpart: %i\n", config->subpart != -1 ? config->subpart : -1);
        printf("  Imagefile: %s\n", config->imagefile ? config->imagefile : "NULL");
        printf("  Path: %s\n", config->path ? config->path : "NULL");
    }
    return 0;
}


// read partition table into an array of partition_table_entry, return -1 if
// the partition table is invalid
// start marks the start of the disk(or partition if we're reading subpartition entries)
int read_partition_table(int fd, struct partition_table_entry *entries, off_t start, Config *config) {
    //do we need to consider that a 'byte' can be different on different systems?
    //can only read the signature of ~/HardDisk image on CSL
    uint8_t mbr[MBR_SIZE];

    // read MBR
    if (lseek(fd, start, SEEK_SET) == -1) {
        perror("lseek");
        return -1;
    }
    printf("reading at: %zu %i bytes\n", start, MBR_SIZE);
    if (read(fd, mbr, MBR_SIZE) != MBR_SIZE) {
        perror("read");
        return -1;
    }
    // debug prints
    if (config->verbose) {
        printf("sig: %02x %02x\n", mbr[510], mbr[511]);
        printf("\n");
    }
    // validate boot signature
    //printf("validating signature...\n");
    if ((mbr[510] != 0x55) || (mbr[511] != 0xAA)) {
        // no partitions
        return -1;
    }
    // copy partition entries
    //printf("copying to pointer...\n");
    memcpy(entries, mbr + PARTITION_TABLE_OFFSET,
           NUM_PARTITIONS * sizeof(struct partition_table_entry));
    
    return 0;
}

int read_superblock(int fd, struct superblock* superblock_entry, int start, Config* config){
    off_t sb_offset = (off_t)(start + SUPERBLOCK_OFFSET);

    //using lseek + read
    if(lseek(fd, sb_offset, SEEK_SET) == -1){
        fprintf(stderr, "lseek\n");
        return 1;
    }
    if(read(fd, superblock_entry, SUPERBLOCK_SIZE_BYTES) == -1){
        fprintf(stderr, "read");
        return 1;
    }
    
    //debug prints
    printf("superblock magic num: %02x\n", superblock_entry->magic);
    if(superblock_entry->magic != MAGIC_NUM) {
        fprintf(stderr, "magic num\n");
        return 1;
    }

    if(config->verbose){
        printf("Superblock Contents:\n");
        printf("Store Fields:\n");
        printf("ninodes           %u\n",  superblock_entry->ninodes);
        printf("i_blocks          %u\n", superblock_entry->i_blocks);
        printf("z_blocks          %d\n", superblock_entry->z_blocks);
        printf("firstdata         %u\n", superblock_entry->firstdata);
        printf("log_zone_size     %d\n", superblock_entry->log_zone_size);
        printf("max_file          %u\n", superblock_entry->max_file);
        printf("zones             %u\n", superblock_entry->zones);
        printf("magic             %04x\n", superblock_entry->magic);
        printf("blocksize         %u\n", superblock_entry->blocksize);
        printf("subversion        %u\n\n", superblock_entry->subversion);
    }
    

    return 1;
}

// read inode into struct
// inodes table starting at start
//  return -1 on error
int read_inode(int fd, struct inode *inode, off_t start, int inode_num, Config *config) {
    int i;
    // seek and read
    if (lseek(fd, start + (inode_num - 1) * sizeof(struct inode), SEEK_SET) == -1) {
        fprintf(stderr, "lseek\n");
        return -1;
    }

    ssize_t bytes = read(fd, inode, sizeof(struct inode));
    if (bytes != sizeof(struct inode)) {
        perror("read");
        return -1;
    }

    // verbose - print inode info
    if(config->verbose) {
        printf("read inode %i\n", inode_num);
        printf("  links: %u\n", inode->links);
        printf("  atime: %u\n", inode->atime);
        printf("  ctime: %u\n", inode->ctime);
        printf("  mtime: %u\n", inode->mtime);
        printf("  size: %u\n", inode->size);
        printf("  mode: %u\n", inode->mode);
        printf("  uid: %u\n", inode->uid);
        printf("  gid: %u\n", inode->gid);

        for (i = 0; i < 7; i++) {
            printf("  zone[%d]: %u\n", i, inode->zone[i]);
        }   
    }
    return 0;
}
