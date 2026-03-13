#ifndef MINLS_H
#define MINLS_H

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "printers.h"
#include "reads.h"
#include "searches.h"

#define USAGE_MESSAGE "usage: minget [ -v ] [ -p part [ -s subpart ] ] imagefile srcpath [ dstpath ]\n \
        Options: \n \
        -p part    --- select partition for filesystem (default: none)\n \
        -s sub     --- select subpartition for filesystem (default: none)\n \
        -h         --- print usage information and exit\n \
        -v         --- enable verbose output (default: off)\n"
#endif