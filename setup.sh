#!/bin/bash

if ! [ -x "$(command -v gcc)" ]; then
  echo 'Error: gcc is required, but not installed.'
  echo 'Try execute command: sudo apt-get -y install build-essential'
  exit 1
fi

if ! [ -x "$(command -v cmake)" ]; then
  echo 'Error: cmake is required, but not installed.'
  echo 'Try execute command: sudo apt-get -y install cmake'
  exit 1
fi

# create code src directory
if ! [ -d "/usr/src/ff" ]; then
  if ! mkdir "/usr/src/ff"; then
    echo "Try use sudo"
    exit 1
  fi
fi
# shellcheck disable=SC2028
echo '''#include <unistd.h>
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
''' > /usr/src/ff/main.c


BIN_NAME="ff_bin"
# shellcheck disable=SC2016
echo '
cmake_minimum_required(VERSION 3.23)
project(kaspersky_ff C)

set(CMAKE_C_STANDARD 11)

add_executable(kaspersky_ff main.c)
set_target_properties(kaspersky_ff PROPERTIES OUTPUT_NAME "'$BIN_NAME'")
' > /usr/src/ff/CMakeLists.txt


# create code lib directory
if ! [ -d "/usr/lib/ff" ]; then
  if ! mkdir "/usr/lib/ff"; then
      exit 1
  fi
fi
# CMake create build cache
if ! sudo cmake -S /usr/src/ff/ -B /usr/lib/ff/ | true; then
  echo "Error: cmake did not build"
  echo "Try use sudo"
  exit 1
fi
# CMake build binary program
sudo cmake --build /usr/lib/ff/ > /dev/null; # remove /dev/null -> output result of build
sudo chmod +x /usr/lib/ff/$BIN_NAME


# create script ff file
# shellcheck disable=SC2016
echo '
#!/bin/bash
if [[ $# -eq 0 ]] ; then
    echo "Wrong count of element. Required>=1 : Received: "$#
    exit 0
fi
to_find=$1
filename=$PWD
if [[ $# -ge 2 ]] ; then
    filename=$2
fi
echo "$(sudo /usr/lib/ff/ff_bin $to_find $filename)"
if ! [ "$res" = "" ]; then
  echo "$res"
fi
' > /usr/bin/ff_bin


# set permission to execute ff
sudo chmod a+x /usr/bin/ff_bin

# need to start sudo and not sudo

if ! [ -f "~/.bashrc" ]; then
  RED='\033[0;31m'
  NC='\033[0m' # No Color
  printf "${RED}NO HOME DIR FOUND${NC} insert: alias ff='/usr/bin/ff_bin'\n"
  exit 0
fi
if ! grep -q 'alias ff="/usr/bin/ff_bin"' ~/.bashrc ; then
 sudo echo 'alias ff="/usr/bin/ff_bin"' >> ~/.bashrc && source ~/.bashrc
fi

echo "RELOAD CONSOLE"
# do tests
