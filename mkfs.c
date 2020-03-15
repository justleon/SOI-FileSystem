#include "filesystem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

int main(int argc, char **argv){
    if(argc < 2){
        puts("Not enough arguments! Check \"./mkfs --help\" for proper use.");
        return MISSING_ARGS;
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
        printf("./mkfs --help to show this message\n");
        printf("./mkfs your_filesys_name to create file system\n");
    } else{
        char *name = argv[1];
        FILE *fp;
        fp = fopen(name, "r");
        if(fp){
            puts("Such file system already exists!");
            fclose(fp);
            return FSYS_ALREADY_CREATED;
        }

        int blocksno;
        printf("How many user blocks of data would you like to be created? (50000 is max):");
        scanf("%d", &blocksno);
        while(blocksno > 50000 || blocksno < 1){
            puts("Invalid number of user blocks! Please, enter valid number: ");
            scanf("%d", &blocksno);
        }
        printf("The file system will have %dkB in total.\n", 4*blocksno+32);

        int sys_size = BLOCK_SIZE*(blocksno+8);
        fp = fopen(name, "w+b");
        if(!fp){
            puts("Couldn't create file system!");
            return COULDNT_CREATE_FSYS;
        }
        truncate(name, sys_size);

        fsys_SuperBlock fssb;
        fssb.blockSize = BLOCK_SIZE;
        fssb.iNodeSize = 128;
        fssb.iNodesCount = 160;
        fssb.iNodesBlockNum = 5;
        fssb.iNodesOffset = 3;

        fssb.dataBlockNum = blocksno;
        fssb.dataBlockOffset = 8;

        fssb.iNodeBitmapOffset = 1;
        fssb.dataBitmapOffset = 2;

        fwrite(&fssb, BLOCK_SIZE, 1, fp);
        puts("Successfully created Superblock.");

        fsys_iNodesBitmap inodeBitmap;
        for(int i = 0; i < FS_INODES; i++)
            inodeBitmap.bitmap[i] = '\0';
        fwrite(&inodeBitmap, BLOCK_SIZE, 1, fp);

        fsys_dataBitmap dataBitmap;
        for(int i = 0; i < blocksno; ++i)
            dataBitmap.bitmap[i] = '\0';
        fwrite(&dataBitmap, BLOCK_SIZE, 1, fp);
        puts("Created bitmaps correctly.");

        fclose(fp);
        printf("Successfully created %s!\n", name);

        return 0;
    }
}
