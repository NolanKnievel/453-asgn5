#include "printers.h"

/* ----- PRINTERS ------*/
//print superblock info
int print_superblock(struct superblock* sb){
        printf("Superblock Contents:\n");
        printf("Store Fields:\n");
        printf("ninodes           %u\n",  sb->ninodes);
        printf("i_blocks          %u\n", sb->i_blocks);
        printf("z_blocks          %d\n", sb->z_blocks);
        printf("firstdata         %u\n", sb->firstdata);
        printf("log_zone_size     %d\n", sb->log_zone_size);
        printf("max_file          %u\n", sb->max_file);
        printf("zones             %u\n", sb->zones);
        printf("magic             %04x\n", sb->magic);
        printf("blocksize         %u\n", sb->blocksize);
        printf("subversion        %u\n\n", sb->subversion);
        return 1;
}

//prints inode and datazone info
//TODO: fix time, numbers don't make sense
int print_inode(struct inode* inode){
    time_t raw_atime = (time_t)inode->atime;
    time_t raw_mtime = (time_t)inode->mtime;
    time_t raw_ctime = (time_t)inode->ctime;
    struct tm *utc_atime = gmtime(&raw_atime);
    struct tm *utc_mtime = gmtime(&raw_mtime);
    struct tm *utc_ctime = gmtime(&raw_ctime);
    
    printf("File inode:\n");
    printf("  uint16_t mode       %#x\n", inode->mode);
    printf("  uint16_t links      %u\n", inode->links);
    printf("  uint16_t uid        %u\n", inode->uid);
    printf("  uint16_t gid        %u\n", inode->gid);

    printf("  uint32_t size       %u\n", inode->size);
    printf("  uint32_t atime      %d --- %d %d %d %d:%d:%d %d\n", 
        inode->atime, 
        utc_atime->tm_wday, utc_atime->tm_mon, utc_atime->tm_mday, 
        utc_atime->tm_hour, utc_atime->tm_min, utc_atime->tm_sec, 
        utc_atime->tm_year);
    printf("  uint32_t mtime      %d --- %d %d %d %d:%d:%d %d\n", 
        inode->mtime,
        utc_mtime->tm_wday, utc_mtime->tm_mon, utc_mtime->tm_mday, 
        utc_mtime->tm_hour, utc_mtime->tm_min, utc_mtime->tm_sec, 
        utc_mtime->tm_year);
    printf("  uint32_t ctime      %d --- %d %d %d %d:%d:%d %d\n", 
        inode->ctime, 
        utc_ctime->tm_wday, utc_ctime->tm_mon, utc_ctime->tm_mday, 
        utc_ctime->tm_hour, utc_ctime->tm_min, utc_ctime->tm_sec, 
        utc_ctime->tm_year);

    int i;
    printf("Direct zones:\n");
    for (i = 0; i < DIRECT_ZONES; i++) {
        printf("              zone[%d]:            %u\n", i, inode->zone[i]);
    }   
    printf("  uint32_t   indirect             %u\n", inode->indirect);
    printf("  uint32_t   double               %u\n", inode->two_indirect);
    return 1;
}

//prints given path
int print_path(Config* config){
    printf("%s:\n", config->path);
    return 1;
}

//prints permissions
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

//parses directory name
int print_name(unsigned char* name){
    char dest[MAX_DIR_NAME_SIZE_BYTES + 1];
    size_t len = strnlen((const char*)name, MAX_DIR_NAME_SIZE_BYTES);
    memcpy(dest, name, len);
    dest[len] = '\0';
    printf(" %s\n", dest);
    return 0;
}

//formats and prints inode number
int print_inum(uint32_t inum){
    printf("%10u", inum);
    return 1;
} 

//iterage through directory contents and print macros
//TODO: skip irregular files, do we need to?
int print_macros_dir(int fd, uint16_t firstdata, struct inode* inode, int zonesize, off_t inode_start, off_t data_start){
    //locals
    int dir_per_zone = zonesize / sizeof(struct directory);
    off_t offset = 0;
    int zone_list_idx = 0;
    int dir_idx = 0;
    struct inode* cur_inode = (struct inode*)malloc(sizeof(struct inode));
    struct directory* cur_dir = (struct directory*)malloc(sizeof(struct directory));
    struct directory* zone_data = (struct directory*)malloc(sizeof(struct directory) * dir_per_zone);

    //iterate through inode's zones
    for(zone_list_idx = 0; zone_list_idx < DIRECT_ZONES; zone_list_idx++){
        //skip if zone index is zero
        if(inode->zone[zone_list_idx] == 0){
            continue;
        }
        //calculate offset 
        offset = calc_datazone_addr(data_start, firstdata, zonesize, inode->zone[zone_list_idx]);

        //read each zone
        if(read_zone(fd, offset, zonesize, zone_data) == -1){
            fprintf(stderr, "read zone error in print_dir\n");
            return -1;
        }

        //read data zone and print stats
        for(dir_idx = 0; dir_idx < dir_per_zone; dir_idx++){
            //get directory
            *cur_dir = zone_data[dir_idx];
            //get inode
            if(read_inode(fd, cur_inode, inode_start, cur_dir->inode) == -1){
                fprintf(stderr, "print_macros read dir error\n");
                return -1;
            }
            //skip deleted files
            if(cur_dir->inode == 0){
                continue;
            }
            //TODO: skip irregular files
            //---
            print_macros_file(cur_dir, cur_inode);
        }
    }
    printf("\n");
    free(cur_inode);
    free(cur_dir);
    free(zone_data);
    return 1;

}


//print single file macros
int print_macros_file(struct directory* dir, struct inode* parent){
    if(print_permissions(parent) == -1){
        fprintf(stderr, "print_file perm error\n");
        return -1;
    }
    if(print_inum(dir->inode) == -1){
        fprintf(stderr, "print_file inum error\n");
        return -1;
    }
    if(print_name(dir->name) == -1){
        fprintf(stderr, "print_file name error\n");
        return -1;
    }
    return 1;
}