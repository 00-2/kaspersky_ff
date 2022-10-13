#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <regex.h>
#define MAXDIR 255
regex_t regex;
int reti;


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
        else if (! regexec(&regex, entry->d_name, 0, NULL, 0))
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
    char *to_regex = malloc(sizeof(char)*MAXDIR*7+1); // because of replacement "*" with "[*\n ]*"
    strcpy(to_regex, to_find);
    char *dir_to_find = malloc(sizeof(char)* MAXDIR);
    if (argc>=3) {
        // check if end with "/"
        // check strlen != 0 in case "/"
        if ((strlen(argv[2])-1)!=0 && argv[2][strlen(argv[2])-1] == '/'){ // ends with "/"
            strncpy(dir_to_find, argv[2], strlen(argv[2])-1);
        }
        else{ // strcpy
            strcpy(dir_to_find, argv[2]);
        }
    #ifdef DEBUG
            printf("FIND filename_root IN ARGV %s\n", dir_to_find);
    #endif
    }
    else {
        // Get the current working directory:
        getcwd( dir_to_find, MAXDIR);
    #ifdef DEBUG
            printf("filename_root is CWD : %s\n", dir_to_find);
    #endif
    }
    // build regex for match special regex symbols, exclude "?*"
    regex_t regex_non_command_symbol;
    reti = regcomp(&regex_non_command_symbol, "[!@#$%^&(),.\":{}|<>]", REG_EXTENDED);
    if (reti) {
        fprintf(stderr, "Could not compile regex\n");
        exit(1);
    }
    char temp_buffer_regex_non_command_symbol[2] = "\0";    // regexec requires string buffer
    size_t to_find_size = strlen(to_find);

    int j = 0; // to_regex pos counter

    for(int i = 0; i<to_find_size; ++i){
        temp_buffer_regex_non_command_symbol[0] = to_find[i];
        if (to_find[i] == '?'){
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
    printdir(dir_to_find, dir_to_find, to_find);
    free(dir_to_find);
    free(to_regex);
    regfree(&regex);
    regfree(&regex_non_command_symbol);
    exit(0);
}