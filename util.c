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
    config->path = " ";

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
        fprintf(stderr, "superblock lseek\n");
        return -1;
    }
    if(read(fd, superblock_entry, SUPERBLOCK_SIZE_BYTES) == -1){
        fprintf(stderr, "superblock read");
        return -1;
    }
    
    //debug prints
    printf("superblock magic num: %02x\n", superblock_entry->magic);
    if(superblock_entry->magic != MAGIC_NUM) {
        fprintf(stderr, "magic num\n");
        return -1;
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

        for (i = 0; i < DIRECT_ZONES; i++) {
            printf("  zone[%d]: %u\n", i, inode->zone[i]);
        }   
    }
    return 0;
}

int dir_check(struct inode* inode){
    if((inode->mode & DIRECTORY_MASK) == 0){
        return 0;
    }
    return 1;
}

int regFile_check(struct inode* inode){
    printf("AND operation: %u\n", (inode->mode & REGULAR_FILE_MASK));
    if((inode->mode & REGULAR_FILE_MASK) == 0){
        return 0;
    }
    return 1;
}

int calc_datazone_addr(struct superblock* superblock_entry, int inum){
    int zone_size = superblock_entry->blocksize << superblock_entry->log_zone_size;
    int inode_data_start = zone_size * superblock_entry->firstdata
                            + ((inum - 1) * DIRECT_ZONES); //gets us to first data zone
    return inode_data_start;
}

//confirm given file exists
uint32_t traverse_path(int fd, 
        struct superblock* superblock_entry,
        int inode_data_start,
        unsigned char* target)
        {
    //locals
    int bytes_read = 0;
    struct directory dir;

    //offset to inode's data
    if(lseek(fd, inode_data_start, SEEK_SET) == -1){
        fprintf(stderr, "lseek error\n");
        return 0;
    }
    //read entire data zone
    int zone_size = superblock_entry->blocksize << superblock_entry->log_zone_size;
    while((bytes_read += read(fd, &dir, sizeof(struct directory))) <= (zone_size * DIRECT_ZONES)){
        //traversing path
        if(strcmp((const char*)dir.name, (const char*)target) == 0){
            //file found
            //resetting fd back to superblock
            if(lseek(fd, -1 * (bytes_read + inode_data_start), SEEK_SET) == -1){
                fprintf(stderr, "lseek error\n");
                return 0;
            }
            //return inode number
            return dir.inode;
        }
    }
    if(bytes_read == -1){
        fprintf(stderr, "Error reading directory\n");
        return 0;
    }
    //resetting fd back to superblock
    if(lseek(fd, -1 * (bytes_read + inode_data_start), SEEK_SET) == -1){
        fprintf(stderr, "lseek error\n");
        return 0;
    }
    
    //print directory (debugging)
    // if(config->verbose){
    //     printf("Directory Entry:\n");
    //     printf("    inode:%u\n", dir.inode);
    //     printf("    name:%s\n", dir.name);
    // }

    //file not found
    return 0;
}

unsigned char* parse_name(unsigned char* name){
    char* ret = strtok((char*)name, "\0");
    return (unsigned char*)ret;
}

//helper to print_content()
int print_permissions(struct inode* inode_entry){
    int i;
    if(dir_check(inode_entry)){
        printf("d");
    }
    else{
        printf("-");
    }
    for(i = 0; i < 3; i++){
        if((inode_entry->mode & (OWNER_R >> i)) != 0){
            printf("r");
        }
        else{
            printf("-");
        }
        if((inode_entry->mode & (OWNER_W >> i)) != 0){
            printf("w");
        }
        else{
            printf("-");
        }
        if((inode_entry->mode & (OWNER_E >> i)) != 0){
            printf("e");
        }
        else{
            printf("-");
        }
    }
    return 1;
}

struct inode* inum_2_inode(int fd, off_t inode_base, int cur_offset){
    //TODO: OFFSET IS WRONG, WE NEED TO GO BACKWARDS FROM DATA ZONES TO INODE BASE
    struct inode* ret = malloc(sizeof(struct inode));
    ssize_t temp = 0;
    //need to go backwards from data done, to base inode, plus inode offset
    if(lseek(fd , offset, SEEK_SET) == -1){//go backwards to inode base
        fprintf(stderr, "lseek for inum_2_inode failed\n");
        return NULL;
    }
    if((temp = read(fd, ret, sizeof(struct inode))) == -1){
        fprintf(stderr, "error reading inode, inum_2_inode\n");
        return NULL;
    }
    return ret;
}

int print_macros(int fd, struct superblock* superblock_entry, struct inode* parent, off_t inode_base, int inum){
    int inode_read = 0;
    struct directory dir;
    struct inode cur_inode;

    //if parent is directory, iterate through contents
    if(dir_check(parent)){
        //locals
        uint32_t dir_read = 0;
        //offset to target inode struct and read it
        off_t offset = calc_datazone_addr(superblock_entry, inum);//where we're at right now
        if(lseek(fd, offset, SEEK_SET) == -1){//offset to datazone
            fprintf(stderr, "lseek error\n");
            return 0;
        }
        //read all files referenced in data zone
        //start reading at fd, which has been offset to the datazone of the given inode
        while((dir_read += read(fd, &dir, sizeof(struct directory))) <= parent->size){
            //skip deleted files
            if(dir.inode == 0){
                continue;
            }
            //get inode struct and print permissions
            struct inode* cur_node = inum_2_inode(fd, inode_base, offset);
            if(cur_node == NULL){
                fprintf(stderr, "error while getting inode\n");
                return -1;
            }
            print_permissions(cur_node);

            //print inode in 10-digit buffer, filling right to left
            printf("%10u", dir.inode);
            //parse name and print it
            size_t name_len = strlen((const char*)dir.name);
            if(name_len > 60){
                name_len = 60;
            }
            char temp_buff[name_len];
            memcpy((void *)temp_buff, (const void*)dir.name, name_len);
            printf(" %s\n", temp_buff);

            //debug prints
            // printf("dir inode: %d\n", dir.inode);
            // printf("dir name: %-10s\n", dir.name);

        }
    }
    else{
        //file not directory, read its inode and print contents
        printf("FILE NOT DIRECTORY\n");
        inode_read = read(fd, &dir, sizeof(struct directory));
        if(inode_read == -1){
            fprintf(stderr, "error when reading dir\n");
            return -1;
        }
        print_permissions(&cur_inode);
        printf("%10d", dir.inode);
        printf("'%s'\n", dir.name);
    }
    return 1;

}