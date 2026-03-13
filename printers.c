#include "printers.h"

/* ----- PRINTERS ------*/
//print superblock info (verbose)
int print_superblock(struct superblock* sb){
        fprintf(stderr, "Superblock Contents:\n");
        fprintf(stderr, "Store Fields:\n");
        fprintf(stderr, "ninodes           %u\n",  sb->ninodes);
        fprintf(stderr, "i_blocks          %u\n", sb->i_blocks);
        fprintf(stderr, "z_blocks          %d\n", sb->z_blocks);
        fprintf(stderr, "firstdata         %u\n", sb->firstdata);
        fprintf(stderr, "log_zone_size     %d\n", sb->log_zone_size);
        fprintf(stderr, "max_file          %u\n", sb->max_file);
        fprintf(stderr, "zones             %u\n", sb->zones);
        fprintf(stderr, "magic             %04x\n", sb->magic);
        fprintf(stderr, "blocksize         %u\n", sb->blocksize);
        fprintf(stderr, "subversion        %u\n\n", sb->subversion);
        return 1;
}

//prints inode and datazone info (verbose)
//TODO: fix time, numbers don't make sense
int print_inode(struct inode* inode){
    char *wdays[7] = { "Sun","Mon","Tue","Wed","Thu","Fri","Sat" };
    char *months[12] = { "Jan","Feb","Mar","Apr","May","Jun","Jul",
        "Aug","Sep","Oct","Nov","Dec" };

    time_t raw_atime = (time_t)inode->atime;
    time_t raw_mtime = (time_t)inode->mtime;
    time_t raw_ctime = (time_t)inode->ctime;
    struct tm *lt_atime = localtime(&raw_atime);
    struct tm *lt_mtime = localtime(&raw_mtime);
    struct tm *lt_ctime = localtime(&raw_ctime);

    fprintf(stderr, "File inode:\n");
    fprintf(stderr, "  uint16_t mode       %#x\n", inode->mode);
    fprintf(stderr, "  uint16_t links      %u\n", inode->links);
    fprintf(stderr, "  uint16_t uid        %u\n", inode->uid);
    fprintf(stderr, "  uint16_t gid        %u\n", inode->gid);

    fprintf(stderr, "  uint32_t size       %u\n", inode->size);
    //atime
    fprintf(stderr, "  uint32_t atime      %d --- %s %s %d %d:%d:%d %d\n", 
        inode->atime, //raw time
        wdays[lt_atime->tm_wday - 1], 
        months[lt_atime->tm_mon], lt_atime->tm_mday - 1, 
        //wday, month, month day
        lt_atime->tm_hour, lt_atime->tm_min, lt_atime->tm_sec, //hour, min, sec
        lt_atime->tm_year + (int)1900);//year
    //mtime
    fprintf(stderr, "  uint32_t mtime      %d --- %s %s %d %d:%d:%d %d\n", 
        inode->mtime, //raw time
        wdays[lt_mtime->tm_wday ], months[lt_mtime->tm_mon], 
        lt_mtime->tm_mday, 
        //wday, month, month day
        lt_mtime->tm_hour, lt_mtime->tm_min, lt_mtime->tm_sec, //hour, min sec
        lt_mtime->tm_year + (int)1900);//year
    //ctime
    fprintf(stderr, "  uint32_t ctime      %d --- %s %s %d %d:%d:%d %d\n", 
        inode->ctime, //raw time
        wdays[lt_ctime->tm_wday], months[lt_ctime->tm_mon], lt_ctime->tm_mday, 
        //wday, month, month day
        lt_ctime->tm_hour, lt_ctime->tm_min, lt_ctime->tm_sec, //hour, min, sec
        lt_ctime->tm_year + (int)1900);//year

    int i;
    fprintf(stderr, "Direct zones:\n");
    for (i = 0; i < DIRECT_ZONES; i++) {
    fprintf(stderr, "              zone[%d]:            \
        %u\n", i, inode->zone[i]);
    }   
    fprintf(stderr, "  uint32_t   indirect             \
        %u\n", inode->indirect);
    fprintf(stderr, "  uint32_t   double               \
        %u\n", inode->two_indirect);
    return 1;
}

//prints given path (verbose)
int print_path(Config* config){
    fprintf(stderr, "%s:\n", config->path);
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

//iterage through directory contents and print macros
int print_macros_dir(int fd, struct superblock* sb, struct inode* inode, 
    int zonesize, off_t inode_start, uint32_t fs_start){
    //printf("PRINT MACROS\n");
    //locals
    uint32_t dirs_per_zone = zonesize / sizeof(struct directory);
    struct inode* cur_inode = (struct inode*)malloc(sizeof(struct inode));
    struct directory* cur_dir = (struct directory*)
        malloc(sizeof(struct directory));

    //indexes and ocunts
    uint32_t global_idx = 0;
    uint32_t real_idx = 0;
    uint32_t dir_count = 0;

    //parameters
    uint32_t max_num_of_zones = DIRECT_ZONES + (sb->blocksize / 32) 
        + (sb->blocksize / 32)*(sb->blocksize / 32);
    struct directory* dirs = (struct directory*)
        malloc(sizeof(struct directory) * dirs_per_zone);
    //printf("print_macros_dir: max_num_of_zones = %u\n", max_num_of_zones);

    //traverse all of inode's zones to look for target
    while((global_idx < max_num_of_zones)){
        //printf("PRINT MACROS: first while-loop\n");
        real_idx = get_file_zone(fd, sb, inode, global_idx, fs_start);
        if(real_idx == 0){
            global_idx++;
            continue;
        }
        read_zone(fd, sb, real_idx, dirs, fs_start);
        // iterate through zone and print files
        while(dir_count < dirs_per_zone){
           // printf("PRINT MACROS: second while-loop\n");
            read_inode(fd, cur_inode, inode_start, dirs[dir_count].inode);
            //if inode is invalid
            if((cur_inode->mtime == 0) || (dirs[dir_count].inode == 0)){
                dir_count++;
                continue;
            }
            *cur_dir = dirs[dir_count];
            print_macros_file(cur_dir, cur_inode);
            dir_count++;
        }
        global_idx++;
    }
    printf("\n");
    free(cur_dir);
    free(cur_inode);
    free(dirs);
    return 1;
}