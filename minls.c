#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
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

    // open image file
    int fd = open(config.imagefile, O_RDONLY);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    // get array of partition entry structs
    struct partition_table_entry partition_entries[NUM_PARTITIONS];
    if (read_partition_table(config.imagefile, partition_entries, 0) == -1) {
        fprintf(stderr, "Failed to read partition table\n");
        return 1;
    }

    // close image file
    close(fd);

    printf("hello world!\n");

    return 0;
}