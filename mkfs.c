#include "filesystem.h"
#include "util.c"
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>

int main(int argc, char *argv[]) {
    if(argc < 2){
        puts("Not enough arguments! Check \"./mkfs --help\" for proper use.");
        return MISSING_ARGS;
    }

    static struct option long_opt[] = {
        {"help",    no_argument,    0,  'h'},
        {0,         0,              0,  0}
    };

    char ch;
    while((ch = getopt_long(argc, argv, "", long_opt, NULL)) != -1) {
        switch(ch){
            case 'h':
                printf("Usage:\n");
                printf("./mkfs --help to show this message\n");
                printf("./mkfs your_filesys_name to create file system\n");
                return 0;
            case '?':
                break;
        }
    }

    char *name = argv[1];
    char *null = '\0';
    FILE *fp;
    int blocksno;
    size_t sys_size;
    fsys_SuperBlock fssb;

    if(fp = fopen(name, "r")){
            puts("Such file system already exists!");
            fclose(fp);
            return FSYS_ALREADY_CREATED;
    }
    fclose(fp);

    printf("How many user blocks of data would you like to create? (50000 is max):");
    blocksno = safeInput();
    while(blocksno > 50000 || blocksno < 1) {
        puts("Invalid number of user blocks! Please, enter valid number: ");
        scanf("%d", &blocksno);
    }
    printf("The file system will have %dkB in total.\n", 4*blocksno+32);

    sys_size = BLOCK_SIZE*(blocksno+8);
    if(!(fp = fopen(name, "a+b"))) {
        puts("Couldn't create file system!");
        return COULDNT_CREATE_FSYS;
    }
    truncate(name, sys_size);
    fwrite(&null, sizeof(char), sys_size, fp);

    fflush(fp);
    fclose(fp);

    fssb.blockSize = BLOCK_SIZE;
    fssb.iNodeSize = FS_INODE_SIZE;
    fssb.iNodesCount = FS_INODES;
    fssb.iNodesBlockNum = 5;
    fssb.iNodesOffset = 3;

    fssb.dataBlockNum = blocksno;
    fssb.dataBlockOffset = 7;

    fssb.iNodeBitmapOffset = 1;
    fssb.dataBitmapOffset = 2;

    if(!(fp = fopen(name, "r+b"))){
        puts("Couldn't create superblock!");
        return COULDNT_CREATE_FSYS;
    }
    rewind(fp);
    fwrite(&fssb, BLOCK_SIZE, 1, fp);

    fflush(fp);
    fclose(fp);

    printf("Successfully created %s!\n", name);

    return 0;
}
