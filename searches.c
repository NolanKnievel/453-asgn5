#include "searches.h"
/* ----- SEARCH FUNCTIONS ----- */
/*
Return values: 
    = 1: file found
    = 0: file not found
    =-1: error
Description:
    These functions search direct, indirect, and double-indirect zones.
    Use by passing in (struct directory*) and other params, if file found, 
    pointer is given value and function returns 1. On file not found, function
    returns 0 without modifying pointer. On error, -1 is returned without
    modifying pointer.
*/
// converts a file zone index to actual zone number
// 0 through DIRECT_ZONES + INDIRECT + DOUBLE_INDIRECT
// actual: the actual zone number

uint32_t get_file_zone(int fd, struct superblock *sb, struct inode *node, 
    uint32_t index, uint32_t fs_start)
{
    //printf("GET FILE ZONE\n");
    uint32_t zone_size = sb->blocksize << sb->log_zone_size;
    uint32_t ptrs_per_block = zone_size / sizeof(uint32_t);

    //changed to zone_size from ptrs_per_block for debugging memory read error
    uint32_t buf[zone_size];

    // Direct
    if (index < DIRECT_ZONES)
        return node->zone[index];

    index -= DIRECT_ZONES;

    // Indirect
    if (index < ptrs_per_block) {

        if (node->indirect == 0)
            return 0;

        read_zone(fd, sb, node->indirect, buf, fs_start);

        return buf[index];
    }

    index -= ptrs_per_block;

    // Double indirect
    if (node->two_indirect == 0)
        return 0;

    // Read indirect table 
    read_zone(fd, sb, node->two_indirect, buf, fs_start);

    uint32_t first_index = index / ptrs_per_block;
    uint32_t second_index = index % ptrs_per_block;

    uint32_t indirect_zone = buf[first_index];

    if (indirect_zone == 0)
        return 0;

    read_zone(fd, sb, indirect_zone, buf, fs_start);

    return buf[second_index];
}

//searches a given zone_data buffer for target
int search_file_zone(unsigned char* target, struct directory* dirs, 
    uint32_t zone_size, 
    struct directory* final_dir)
{
    //printf("SEARCH FILE ZONE\n");
    //locals
    //cast target to 60 character buffer for good comparison
    //printf("search_file_zone: target before while loop = %s\n", target);

    unsigned char local_target [MAX_DIR_NAME_SIZE_BYTES + 1];
    local_target[MAX_DIR_NAME_SIZE_BYTES] = '\0';
    memcpy(local_target, target, MAX_DIR_NAME_SIZE_BYTES+1);

    int dirs_per_zone = zone_size / sizeof(struct directory);
    struct directory cur_dir;
    int i = 0;
    
    //printf("search_file_zone: size of target")
    //iterate through buff to find target
    while(i < dirs_per_zone){
        cur_dir = dirs[i];
        // printf("search_file_zone: cur_dir.name = %s\n", cur_dir.name);
        // printf("search_file_zone: target = %s\n", target);
        if(strcmp((const char*)local_target, (const char*)cur_dir.name) == 0){
            //printf("seach_file_zone: file found!\n");
            *final_dir = cur_dir;
            return 1;
        }
        i++;
    }
    //printf("search_file_zone: file not found!\n");
    return 0;
}


int search_zones(int fd, struct superblock* sb, struct inode* inode, 
    struct directory* final_dir, unsigned char *target, int fs_start){
    uint32_t global_idx = 0;
    int real_idx = 0;

    //calculate max_global_zone_idx
    uint32_t zone_size = sb->blocksize << sb->log_zone_size;
    uint32_t max_num_of_zones = (inode->size + zone_size - 1) / zone_size;

    int dirs_per_zone = zone_size / sizeof(struct directory);

    //pointers
    struct directory* zone_data = (struct directory*)
        malloc(sizeof(struct directory) * dirs_per_zone);

    //reset counters
    max_num_of_zones = (inode->size + zone_size - 1) / zone_size;
    global_idx = 0;
    int ret = 0;

    //traverse all of inode's zones to look for target
    while((global_idx < max_num_of_zones)){
        //printf("search_all: while-loop for global and dir index\n");
        real_idx = get_file_zone(fd, sb, inode, global_idx, fs_start);

        //printf("search_all: read_idx = %d\n", real_idx);
        if(real_idx == 0){
            global_idx++;
            continue;
        }
        read_zone(fd, sb, real_idx, zone_data, fs_start);

        //if we find target
        if((ret = search_file_zone(target, 
            zone_data, zone_size, final_dir)) == 1){
            break;
        }
        global_idx++;
    }
    free(zone_data);
    return ret;
}

//want to call this in main
int search_all(int fd, struct superblock *sb, struct inode *node, 
    uint32_t fs_start, uint32_t inode_start, char* path, 
    struct directory* final_dir, struct inode* final_inode){
    //printf("SEARCH ALL\n");

    //parsing path
    //printf("search_all: path depth = %u\n", path_depth);

    unsigned char* target = 
        (unsigned char*)malloc((MAX_DIR_NAME_SIZE_BYTES + 1));
    target[MAX_DIR_NAME_SIZE_BYTES] = '\0';

    //indexes and counters
    int ret = 0;

    //initial strtok
    if((target = (unsigned char*)strtok(path, "/")) == NULL){
        printf("search_all: initial target strtok\n");
        // free(target);
        return -1;
    }
    if((ret = search_zones(fd, sb, node, final_dir, target, fs_start)) == -1){
        printf("search_all: search_zones call error\n");
        free(target);
        return -1;
    }
    if(ret == 0){
        printf("search_all: file not found\n");
        free(target);
        return -1;
    }
    read_inode(fd, final_inode, inode_start, final_dir->inode);
    
    //traverse rest of path
    while((target = (unsigned char*)strtok(NULL, "/")) != NULL){
        if((ret = 
                search_zones(fd, sb, final_inode, 
                    final_dir, target, fs_start)) == -1){
            printf("search_all: search_zones call error\n");
            free(target);
            return -1;
        }
        if(ret == 0){
            printf("search_all: file not found\n");
            free(target);
            return -1;
        }
    }
    read_inode(fd, final_inode, inode_start, final_dir->inode);

    free(target);
    //we traversed the whole path and didn't error or not find a file
    return 1;
}
