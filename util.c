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
int read_partition_table(int fd, struct partition_table_entry *entries, size_t start) {
    uint8_t mbr[MBR_SIZE];
    int i;

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
    printf("sig: %02x %02x\n", mbr[510], mbr[511]);
    for (i = 0; i < 16; i++) {
        printf("%02x ", mbr[i]);
    }
printf("\n");

    // validate boot signature
    if (mbr[510] != 0x55 || mbr[511] != 0xAA) {
        fprintf(stderr, "Invalid partition table\n");
        return -1;
    }

    // copy partition entries
    memcpy(entries, mbr + PARTITION_TABLE_OFFSET,
           NUM_PARTITIONS * sizeof(struct partition_table_entry));

    return 0;
}