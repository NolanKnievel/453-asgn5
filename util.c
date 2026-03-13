#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include "util.h"
#include "stdio.h"
#include "stdlib.h"

/* ----- UTILITIES ------*/

// parse args and update config struct to match
// ~pn-cs453/Given/Asgn5/Images
int parse_args(int argc, char *argv[], Config *config) {
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
            fprintf(stderr, "Unexpected arguments");
            fprintf(stderr, "please follow the pattern: \n minls [ -v ] ");
            fprintf(stderr, "[ -p part [ -s subpart ] ] \
                imagefile [ path ]\n");
            return -1;
        }
    }

    // validate args
    if (config->imagefile == NULL) {
        fprintf(stderr, "Missing required imagefile\n");
        return -1;
    }

    if ((config->subpart != -1) && (config->part == -1)) {
        fprintf(stderr, "-s cannot be used without -p\n");
        return -1;
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
    if((inode->mode & REGULAR_FILE_MASK) == 0){
        return 0;
    }
    return 1;
}

//calculates datazone offset using zone index and zonesize
int calc_datazone_addr(int data_start, 
    uint16_t firstdata, int zonesize, int zone_idx){
    return data_start + (off_t)(zone_idx - firstdata) * zonesize;
}

//copies path and counts length
int strtok_count(char* path){
    char copy [strlen((const char*)path)];

    memcpy((void*)copy, (const void*)path, strlen((const char*)path));

    char* ret = strtok(copy, "/");
    int count = 0;

    while((ret != NULL)){
        count++;
        ret = strtok(NULL, "/");
    }
    return count;
}

