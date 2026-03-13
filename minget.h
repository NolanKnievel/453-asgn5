#ifndef MINGET_H
#define MINGET_H

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "printers_get.h"
#include "reads_get.h"
#include "searches_get.h"

#define USAGE_MESSAGE "usage: minget [ -v ] [ -p part [ -s subpart ] ] \
        imagefile srcpath [ dstpath ]\n \
        Options: \n \
        -p part    --- select partition for filesystem (default: none)\n \
        -s sub     --- select subpartition for filesystem (default: none)\n \
        -h         --- print usage information and exit\n \
        -v         --- enable verbose output (default: off)\n"
#endif