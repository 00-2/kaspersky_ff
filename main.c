#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#define MAXDIR 255

void printdir(char *dir, char* depth, const char *to_find)
{
    DIR *dp;
    struct dirent *entry;
    struct stat statbuf;
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
            char *tempdepth = malloc(sizeof (char)*MAXDIR);
            strcpy(tempdepth, depth);
            if (strcmp(tempdepth, "/") != 0)
                strcat(tempdepth, "/");
            strcat(tempdepth, entry->d_name);
            printdir(entry->d_name, tempdepth, to_find);
            free(tempdepth);
        }
        else if (strcmp(entry->d_name, to_find) == 0)
        {
            printf("%s/%s\n", depth, entry->d_name);
        }
    }
    chdir("..");
    closedir(dp);
}

int main(int argc, char** argv)
{
    if (argc<2) {fprintf(stderr, "Wrong count of element. Required>=1 : Received: %d\n", argc-1); exit(-1);}
    const char *to_find = argv[1];
    char *filename;
    if (argc>=3) {
        filename = argv[2];
#ifdef DEBUG
        printf("FIND filename_root IN ARGV %s\n", filename);
#endif
    }
    else {
        filename = malloc(MAXDIR);
        // Get the current working directory:
        getcwd( filename, MAXDIR);
#ifdef DEBUG
        printf("filename_root is CWD : %s\n", filename);
#endif
    }
    printdir(filename, filename, to_find);
    exit(0);
}