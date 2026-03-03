#include "minls"
#include <string.h>
#include <unistd.h>

typedef struct {
    int verbose; 
    char *part; // defaults to NULL if not provided - no partition
    char *subpart; // defaults to NULL if not provided
    char *imagefile; 
    char *path; // defaults to '/' if not provided. '/' added to paths not including one
} Config;


int parse_args(int argc, char *argv[], Config *config) {
    char *arg;
    int i = 1;

    // set defaults
    config->verbose = 0;
    config->part = NULL;
    config->subpart = NULL;
    confige->imagefile = NULL;
    config->path = NULL;
    

    while(i < argc) {
        // -v
        if(strcmp(argv[i], "-v") == 0) {
            config->verbose = 1;
            i++;
        }
        // -p
        else if(strcmp(argv[i], "-p" == 0)) {
            if (i+1 >= argc) {
                fprintf(stderr, "-p requires a part number\n");
                return -1;
            }
            config->part = argv[i+1];
            i += 2;

            // -s
            if(strcmp(argv[i], "-s" == 0)) {
                if (i+1 >= argc) {
                    fprintf(stderr, "-v requires a subpart number\n");
                    return -1;
                }
                config->subpart = argv[i+1];
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
            i++
        }
        else {
            fprintf(stderr, "Unexpected argument: %s\n", argv[i]);
            return -1;
        }
    }


    // validate args
    if (cfg->imagefile == NULL) {
        fprintf(stderr, "Missing required imagefile\n");
        return -1;
    }

    if (cfg->has_subpart && !cfg->has_part) {
        fprintf(stderr, "-s cannot be used without -p\n");
        return -1;
    }

    return 0;
        
}
