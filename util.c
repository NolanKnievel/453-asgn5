#include <string.h>
#include <unistd.h>
#include "util.h"
#include "stdio.h"
#include "stdlib.h"


// parse args and update config struct to match
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
