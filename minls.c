#include <stdio.h>
#include "minls.h"
#include "util.h"

int main(int argc, char *argv[]) {
    Config config;

    printf("hello world!\n");
    parse_args(argc, argv, &config);


    return 0;
}