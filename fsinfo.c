#include <stdio.h>
#include <getopt.h>
#include "filesystem.h"

int main(int argc, char **argv){
    if(argc < 2){
        puts("Not enough arguments! Check \"./fsinfo --help\" for proper use.");
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
        printf("./fsinfo --help to show this message\n");
        printf("./fsinfo your_filesys_name to map the file system\n");
    } else{
        char *name = argv[1];
        FILE *fp;

        fp = fopen(name, "r+b");
        if(!fp){
            puts("No such file system exists!");
            return FILE_NOT_EXISTING;
        }

        fsys_SuperBlock fssb;
        fread(&fssb, sizeof(fsys_SuperBlock), 1, fp);

        char iNodeBitmap[fssb.iNodesCount];
        char dataBitmap[fssb.dataBlockNum];
        fsys_iNode iNodeTable[fssb.iNodesCount];

        fseek(fp, fssb.iNodeBitmapOffset*fssb.blockSize, SEEK_SET);
        fread(iNodeBitmap, sizeof(char), fssb.iNodesCount, fp);
        fseek(fp, fssb.iNodesOffset*fssb.blockSize, SEEK_SET);
        fread(iNodeTable, fssb.iNodeSize, fssb.iNodesCount, fp);

        fseek(fp, fssb.dataBitmapOffset*fssb.blockSize, SEEK_SET);
        fread(dataBitmap, sizeof(char), fssb.dataBlockNum, fp);
        fclose(fp);

        int iNodeCount = 0, rlSpace = 0;
        for(int i = 0; i < fssb.iNodesCount; ++i){
            if(iNodeBitmap[i] != 0x00){
                iNodeCount++;
                rlSpace += iNodeTable[i].size;
            }
        }

        printf("iNode table: 0x%x, it's occupied in %.2f%%\n",
            fssb.iNodesOffset*BLOCK_SIZE, 100*(float)iNodeCount/(float)fssb.iNodesCount);
        printf("File sys stores %d file(s)\n", iNodeCount);
        printf("iNode bitmap:\n");
        for(int i = 0; i < fssb.iNodesCount; ++i)
            printf("|%c", iNodeBitmap[i] == 0x00 ? 'O' : 'X');
        printf("|\n");

        int dataBlockCount = 0;
        for(int i = 0; i < fssb.iNodesCount; ++i){
            if(dataBitmap[i] != 0x00) dataBlockCount++;
        }

        printf("dataBlock table: 0x%x, it's occupied in %.2f%%\n",
            fssb.dataBitmapOffset*BLOCK_SIZE, 100*(float)dataBlockCount/(float)fssb.dataBlockNum);
        printf("Real data space usage = %.2f%%, %d bytes\n", 100*(float)rlSpace/((float)fssb.dataBlockNum*fssb.blockSize), rlSpace);
        printf("Data bitmap:\n");
        for(int i = 0; i < fssb.dataBlockNum; ++i)
            printf("|%c", dataBitmap[i] == 0x00 ? 'O' : 'X');
        printf("|\n");
    }
}
