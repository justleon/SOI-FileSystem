#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include "filesystem.h"

int main(int argc, char **argv){
    if(argc != 2){
        puts("Not valid no of arguments! Check \"./rmfs --help\" for proper use.");
        return INCORRECT_ARG;
    }

    int flag = 0;
    static struct option long_opt[] = {
        {"help",    no_argument,    0,  'h'},
        {0,         0,              0,  0}
    };

    char ch;
    while((ch = getopt_long(argc, argv, "", long_opt, NULL)) != -1){
        switch(ch){
            case 'h':
                flag = 1;
                break;
            case '?':
                break;
        }
    }

    if(flag == 1){
        printf("Usage:\n");
        printf("./rmfs --help to show this message\n");
        printf("./rmfs your_filesys_name to remove file system\n");
    } else{
        FILE *fp;
        char *name = argv[1];
        fp = fopen(name, "r");
        if(fp){
            fclose(fp);
            unlink(name);
            printf("Successfully removed %s!\n", name);
            return 0;
        }
        puts("There's no such file system!");
        return FILE_NOT_EXISTING;
    }
}
