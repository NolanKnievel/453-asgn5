typedef struct {
    int verbose; 
    char *part; // defaults to NULL if not provided - no partition
    char *subpart; // defaults to NULL if not provided
    char *imagefile; // required
    char *path; // defaults to '/' if not provided. '/' added to paths not including one
} Config;


int parse_args(int argc, char *argv[], Config *config);
