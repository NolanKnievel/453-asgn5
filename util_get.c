#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include "util_get.h"
#include "stdio.h"
#include "stdlib.h"

/* ----- UTILITIES ------*/

// parse args and update config struct to match
// ~pn-cs453/Given/Asgn5/Images
int parse_ls_args_get(int argc, char *argv[], Config *config) {
    int i = 1;

    // set defaults
    config->verbose = 0;
    config->part = -1;
    config->subpart = -1;
    config->imagefile = NULL;
    config->path = "/";

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

    // not needed for verbose (debugging)
    // if(config->verbose) {
    //     printf("Config:\n");
    //     printf("  Verbose: %d\n", config->verbose);
    //     printf("  Part: %i\n", config->part != -1 ? config->part : -1);
    //     printf("  Subpart: %i\n", config->subpart != -1 ? config->subpart : -1);
    //     printf("  Imagefile: %s\n", config->imagefile ? config->imagefile : "NULL");
    //     printf("  Path: %s\n", config->path ? config->path : "NULL");
    // }
    return 0;
}


// parse args and update config struct to match
int parse_get_args_get(int argc, char *argv[], Config *config) {
    int i = 1;

    // set defaults
    config->verbose = 0;
    config->part = -1;
    config->subpart = -1;
    config->imagefile = NULL;
    config->path = NULL;
    config->copy_path = NULL;

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
        else if(config->path == NULL) {
            config->path = argv[i];
            i++;
        }
        // for minget: copy_path if arg exists
        else if(config->copy_path == NULL) {
            config->copy_path = argv[i];
            i++;
        }
        else {
            fprintf(stderr, "Unexpected arguments, please follow the pattern: \n minget [ -v ] [ -p part [ -s subpart ] ] imagefile srcpath [ dstpath ]\n");
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

    if (config->path == NULL) {
        fprintf(stderr, "Missing required path\n");
        return -1;
    }

    // print config
    if (config->verbose) {
        printf("Config:\n");
        printf("  Verbose: %d\n", config->verbose);
        printf("  Part: %i\n", config->part != -1 ? config->part : -1);
        printf("  Subpart: %i\n", config->subpart != -1 ? config->subpart : -1);
        printf("  Imagefile: %s\n", config->imagefile ? config->imagefile : "NULL");
        printf("  Path: %s\n", config->path ? config->path : "NULL");
        printf("  Copy Path: %s\n", config->copy_path ? config->copy_path : "NULL");
    }

    return 0;
}

// return 1 if inode is directory, 0 otherwise
int dir_check_get(struct inode* inode){
    if((inode->mode & DIRECTORY_MASK) == 0){
        return 0;
    }
    return 1;
}
// return 1 if inode is file, 0 otherwise
int regFile_check_get(struct inode* inode){
    printf("AND operation: %u\n", (inode->mode & REGULAR_FILE_MASK));
    if((inode->mode & REGULAR_FILE_MASK) == 0){
        return 0;
    }
    return 1;
}

//calculates datazone offset using zone index and zonesize
int calc_datazone_addr_get(int data_start, uint16_t firstdata, int zonesize, int zone_idx){
    return data_start + (off_t)(zone_idx - firstdata) * zonesize;
}

//copies path and counts length
int strtok_count_get(char* path){
    char copy [strlen((const char*)path)];
    memcpy((void*)copy, (const void*)path, strlen((const char*)path));
    char* ret = strtok(path, "/");
    int count = 0;
    while((ret != NULL)){
        count++;
        ret = strtok(NULL, "/");
    }
    return count;
}


// helper to read a zone
void read_zone2_get(int fd, struct superblock *sb, uint32_t zone, void *buf, uint32_t fs_start, Config *config) {
    uint32_t zone_size = sb->blocksize << sb->log_zone_size;

    if(config->verbose) {
        printf("Reading zone %d\n", zone);
    }

    if (zone == 0) {
        memset(buf, 0, zone_size);
        return;
    }

    uint32_t offset = fs_start + (zone * sb->blocksize << sb->log_zone_size);

    lseek(fd, offset, SEEK_SET);
    read(fd, buf, zone_size);
}


// converts a file zone index to actual zone number
// 0 through DIRECT_ZONES + INDIRECT + DOUBLE_INDIRECT
// actual: the actual zone number
uint32_t get_file_zone_get(int fd, struct superblock *sb, struct inode *node, uint32_t index, uint32_t fs_start, Config *config) {
    uint32_t zone_size = sb->blocksize << sb->log_zone_size;
    uint32_t per_block = zone_size / sizeof(uint32_t);

    uint32_t buf[per_block];

    // Direct
    if (index < DIRECT_ZONES){
        if (config->verbose) {
            printf("Reading direct zone %d\n", index);
        }

        return node->zone[index];
    }
    index -= DIRECT_ZONES;

    // Indirect
    if (index < per_block) {

        if (node->indirect == 0)
            return 0;

        if (config->verbose) {
            printf("Reading indirect zone %d\n", index);
        }

        read_zone2_get(fd, sb, node->indirect, buf, fs_start, config);

        return buf[index];
    }

    index -= per_block;

    // Double indirect
    if (node->two_indirect == 0)
        return 0;

    if (config->verbose)
        printf("Reading two indirect zone %d\n", index);

    read_zone2_get(fd, sb, node->two_indirect, buf, fs_start, config);

    uint32_t first_index = index / per_block;
    uint32_t second_index = index % per_block;

    uint32_t indirect_zone = buf[first_index];

    if (indirect_zone == 0)
        return 0;

    read_zone2_get(fd, sb, indirect_zone, buf, fs_start, config);

    return buf[second_index];
}

// writes the contents of a file to a destination
void copy_file_get(int fd, FILE *dst, struct superblock *sb, struct inode *node, uint32_t fs_start, Config *config) {
    uint32_t zone_size = sb->blocksize << sb->log_zone_size;
    uint32_t remaining = node->size;

    char *buffer = malloc(zone_size);

    uint32_t zone_index = 0;

    while (remaining > 0) {

        uint32_t zone = get_file_zone_get(fd, sb, node, zone_index, fs_start, config);

        if (zone == 0) {
            memset(buffer, 0, zone_size);
        } else {
            read_zone2_get(fd, sb, zone, buffer, fs_start, config);
        }

        uint32_t write_size = remaining < zone_size ? remaining : zone_size;

        fwrite(buffer, write_size, 1, dst);

        remaining -= write_size;
        zone_index++;
    }

    free(buffer);
}