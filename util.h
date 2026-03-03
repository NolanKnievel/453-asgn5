typedef struct {
    int verbose; 
    int part; // defaults to -1 if not provided - no partition
    int subpart; // defaults to -1 if not provided - no subpartition
    char *imagefile; // required
    char *path; // defaults to '/' if not provided. '/' added to paths not including one
} Config;


int parse_args(int argc, char *argv[], Config *config);
