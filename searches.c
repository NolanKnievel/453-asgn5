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

//confirm given file exists in given data zone
int file_search_datazone(int zonesize,
        struct directory* zone_data,
        unsigned char* target,
        struct directory* dir)
        {
    //locals
    struct directory local_dir;
    int idx = 0;
    int dir_per_zone = zonesize / sizeof(struct directory);
    //read zone and compare target to dir.name
    for(idx = 0; idx < dir_per_zone; idx++){
        local_dir = zone_data[idx];
        if(strcmp((const char*)local_dir.name, (const char*)target) == 0){
            //file found
            *dir = local_dir;
            return 1;
        }
    }
    //file not found
    return 0;
}

int search_all(int fd,
    Config* config, 
    uint16_t blocksize,
    struct inode* inode,
    int inode_start, 
    int zonesize, 
    struct directory* final_dir,
    struct inode* final_inode
    ){
    //call all search functions
    int ret = search_direct(fd, inode, config, inode_start, zonesize, final_dir);
    //delete, used to avoid 'unused variable' error
    blocksize++;
    //error
    if(ret == -1){
        fprintf(stderr, "search direct error\n");
        return -1;
    }
    // //if ret == 0, file wasn't found, continue to indirect
    // if(ret == 0){
    //     ret = search_indirect(fd, );
    // }
    // //indirect error
    // if(ret == -1){
    //     fprintf(stderr, "search indirect error\n");
    //     return 1;
    // }
    // //indirect didn't find file, use double indirect
    // if(ret == 0){
    //     ret = search_2x_indirect();
    // }
    // //indirect error
    // if(ret == -1){
    //     fprintf(stderr, "search double indirect error\n");
    //     return 1;
    // }
    //file not found in double indirect, file DNE
    if(ret == 0){
        fprintf(stderr, "File not found!");
        return -1;
    }
    //file found
    if(ret == 1){
        //save final inode using final_dir->inode
        if(read_inode(fd, final_inode, inode_start, final_dir->inode) == -1){
            fprintf(stderr, "error reading inode\n");
            return 1;
        }
    }
    //file not found
    return 0;
}

int search_direct(int fd, 
    struct inode* parent, 
    Config* config, 
    int zonesize,
    int inode_start,
    struct directory* final_dir
    ){

    struct directory* cur_dir = NULL;
    struct inode* cur_inode = parent;
    int path_depth = strtok_count(config->path);
    int local_count = 0;
    int zone_off = 0;
    char* target = NULL;
    char* delim = "/";
    struct directory zone_data [zonesize];

    //iterating through file path
    while((target = strtok(config->path, delim)) != NULL){
        //for loop counter
        int zone_list_idx;

        //iterate thorugh inode's direct data zones
        for(zone_list_idx = 0; zone_list_idx < DIRECT_ZONES; zone_list_idx++){
            //calculate the start of the specific data zone
            zone_off = zonesize * cur_inode->zone[zone_list_idx];
            //read the data zones
            if(read_zone(fd, zone_off, zonesize, zone_data) == -1){
                fprintf(stderr, "read_zone error\n");
                return 1;
            }
            //want to search for target of current inode
            int ret = 0;
            if((ret = file_search_datazone(zonesize, zone_data, (unsigned char*)target, cur_dir)) == 1){
                local_count++;
                //last file found, save directory, return success
                //final_inode will be updated in search_all()
                if(local_count == path_depth){
                    *final_dir = *cur_dir;
                    return 1;
                }
            }
            //if file_search_datazone() errors
            if(ret == -1){
                fprintf(stderr, "search_direct: file_search_datazone error\n");
                return -1;
            }
            //update cur_dir for following iteration
            if((read_inode(fd, cur_inode, inode_start, cur_dir->inode)) == -1){
                fprintf(stderr, "search_direct: read inode error\n");
                return -1;
            }
        }
    }
    //file not found
    return 0;
}

// int search_indirect(int fd, 
//     struct inode* inode,
//     Config* config, 
//     uint16_t blocksize,
//     int zonesize, 
//     struct directory* final_dir
//     ){
//     //need inode->indirect
//     //read a block's worth of inode->indirect
//     //search that for config->target
//     uint32_t indirect_ptr = inode->indirect;
//     return 0;
// }

// int search_2x_indirect(int fd, 
//     struct inode* inode,
//     Config* config, 
//     uint16_t blocksize,
//     int zonesize, 
//     struct directory* final_dir
//     ){
//     //need inode->two_indirect
//     int two_indirect_ptr = inode->two_indirect;
//     //read a block's worth of this zone, this will contain more uint32_t pointers
//     //iterate through pointers
//     return 0;
// }
