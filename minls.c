#include <stdio.h>
#include "minls.h"
#include "util.h"

int main(int argc, char *argv[]) {
    Config config;
    char * root = "/";
    config.path = root;
    
    if(argc == 1) {
        printf("usage: minls [ -v ] [ -p part [ -s subpart ] ] imagefile [ path ]\n");
        printf("Options: \n");
        printf("  -p part    --- select partition for filesystem (default: none)\n");
        printf("  -s sub     --- select subpartition for filesystem (default: none)\n");
        printf("  -h         --- print usage information and exit\n");
        printf("  -v         --- enable verbose output (default: off)\n");
        return 1;
    }
    parse_args(argc, argv, &config);


    printf("hello world!\n");

    return 0;
}