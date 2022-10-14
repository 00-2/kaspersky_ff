#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <regex.h>
#include <sys/stat.h>
#include <dirent.h>

regex_t regex;
int reti;
long path_max;
size_t _MAX_PATH_SIZE, size;
char *buf;
char *ptr;

static inline void *mallocOrDie(size_t MemSize)
{
    void *AllocMem = malloc(MemSize);
    /* Some implementations return null on a 0 length alloc,
     * we may as well allow this as it increases compatibility
     * with very few side effects */
    if(!AllocMem && MemSize)
    {
        //TODO handle error
        fprintf(stderr, "Cannot allocate memory\n");
        exit(-1);
    }
    return AllocMem;
}

void printdir(char *dir, char* depth, const char *to_find)
{
    DIR *dp;
    struct dirent *entry;
    struct stat statbuf;
    size_t count_of_block_depth;
    size_t count_of_block_entry;
    if ((dp=opendir(dir)) == NULL) {
        fprintf(stderr, "cannot open directory: %s\n", dir);
        return;
    }
    chdir(dir);
    while ((entry = readdir(dp)) != NULL) {
        lstat(entry->d_name,&statbuf);
        if(S_ISDIR(statbuf.st_mode)) {
            /* found a directory, but ignore . and .. */
            if (strcmp(".", entry->d_name) == 0 ||
                strcmp("..", entry->d_name) == 0)
                continue;
            /* recurse at a new indent level */
            count_of_block_depth = strlen(depth) / _MAX_PATH_SIZE + 1;
            count_of_block_entry = strlen(entry->d_name) / _MAX_PATH_SIZE + 1;
            char *tempdepth = mallocOrDie((sizeof(char)*_MAX_PATH_SIZE) * (count_of_block_depth+count_of_block_entry) + 1);
            strcpy(tempdepth, depth);
            if (strcmp(tempdepth, "/") != 0)
                strcat(tempdepth, "/");
            strcat(tempdepth, entry->d_name);
            printdir(entry->d_name, tempdepth, to_find);
            free(tempdepth);
        }
        else if (! regexec(&regex, entry->d_name, 0, NULL, 0))
        {
            printf("%s/%s\n", depth, entry->d_name);
        }
    }
    chdir("..");
    closedir(dp);
}

int main(int argc, char** argv) {
    char *to_regex;
    const char *to_find;
    char *root_dir;
    size_t count_of_block_depth_init;
    size_t count_of_block_root_dir = 0;
    path_max = pathconf(".", _PC_PATH_MAX);
    if (path_max == -1)
        _MAX_PATH_SIZE = 1024;
    else if (path_max > 10240)
        _MAX_PATH_SIZE = 10240;
    else
        _MAX_PATH_SIZE = path_max;

    if (argc<2) {fprintf(stderr, "Wrong count of element. Required>=1 : Received: %d\n", argc-1); exit(-1);}
    to_find = argv[1];
    count_of_block_depth_init = strlen(to_find) / _MAX_PATH_SIZE + 1;
    to_regex = calloc((_MAX_PATH_SIZE*7) * count_of_block_depth_init + 1, sizeof(char)); // because of replacement "*" with "[*\n ]*"
    strcpy(to_regex, to_find);

    if (argc>=3) {
        // check if end with "/"
        // check strlen != 0 in case "/"
        count_of_block_root_dir = strlen(argv[2]) / _MAX_PATH_SIZE + 1;
        root_dir = mallocOrDie((sizeof(char)*_MAX_PATH_SIZE*7) * count_of_block_root_dir + 1);
        if ((strlen(argv[2])-1)!=0 && argv[2][strlen(argv[2])-1] == '/') { // ends with "/"
            strncpy(root_dir, argv[2], strlen(argv[2])-1);
        }
        else {
            strcpy(root_dir, argv[2]);
        }
    #ifdef DEBUG
        printf("root_dir from ARGV %s\n", root_dir);
    #endif
    }
    else {
        size = _MAX_PATH_SIZE;
        for (root_dir = ptr = NULL; ptr == NULL; size *= 2) {
            if ((root_dir = realloc(root_dir, size)) == NULL) {
                //TODO handle error
                fprintf(stderr, "Cannot reallocate memory\n");
                exit(-1);
            }

            ptr = getcwd(root_dir, size);
            if (ptr == NULL && errno != ERANGE) {
                //TODO handle error
                fprintf(stderr, "Error with getcwd\n");
                exit(-1);
            }
        }
    #ifdef DEBUG
        printf("root_dir is CWD : %s\n", root_dir);
    #endif
    }

    // build regex for match special regex symbols, exclude "?*"
    regex_t regex_non_command_symbol;
    reti = regcomp(&regex_non_command_symbol, "[!@#$%^&(),.\":{}|?<>]", REG_EXTENDED);
    if (reti) {
        //TODO handle error
        fprintf(stderr, "Could not compile regex\n");
        exit(1);
    }
    char temp_buffer_regex_non_command_symbol[2] = "\0";    // regexec requires string buffer
    int i;
    int j = 1; // to_regex counter
    to_regex[0] = '^';
    for(i = 0; i<strlen(to_find); ++i){
        temp_buffer_regex_non_command_symbol[0] = to_find[i];
        if (to_find[i] == '?'){ // "?" -> "." because of POSIX
            to_regex[j]='.'; ++j;
            continue;
        }
        if (to_find[i] == '*'){
            to_regex[j]='['; ++j;
            to_regex[j]='^'; ++j;
            to_regex[j]='\\'; ++j;
            to_regex[j]='n'; ++j;
            to_regex[j]=' '; ++j;
            to_regex[j]=']'; ++j;
            to_regex[j]='*'; ++j;
            continue;
        }
        reti = regexec(&regex_non_command_symbol, temp_buffer_regex_non_command_symbol, 0, NULL, 0);
        if (!reti) { // match
            to_regex[j]='\\'; ++j;
            to_regex[j]=to_find[i]; ++j;
        }
        else {
            to_regex[j]=to_find[i]; ++j;
        }
    }
    strcat(to_regex, "$");
    #ifdef DEBUG
        printf("to_regex value %s\n", to_regex);
    #endif
    // main regex pattern
    // validate in printdir
    reti = regcomp(&regex, to_regex, REG_EXTENDED);
    if (reti) {
        fprintf(stderr, "Could not compile regex\n");
        exit(1);
    }
    printdir(root_dir, root_dir, to_find);
    free(to_regex);
    free(root_dir);
    regfree(&regex);
    regfree(&regex_non_command_symbol);
    exit(0);
}