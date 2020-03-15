#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include "filesystem.h"

int main(int argc, char **argv){
    if(argc < 3){
        puts("Not enough arguments! Check \"./fscpy --help\" for proper use.");
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
        printf("./fscpy --help to show this message\n");
        printf("./fscpy your_sys_file your_file1, your_file2,... to copy files onto the file system\n");
    } else{
        int argCount = 2;
        const char occ = 0xff;
        while(argCount < argc){
            char *sysName = argv[1], *fileName = argv[argCount];
            FILE *fp, *cpsrc;

            fp = fopen(sysName, "r+b");
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

            for(int i = 0; i < fssb.iNodesCount; ++i){
                if(iNodeBitmap[i] != '\0'){
                    if(strcmp(iNodeTable[i].name, fileName) == 0){
                        printf("File %s already exists on the virtual disk!", fileName);
                        return FILE_ALREADY_EXISTS;
                    }
                }
            }

            cpsrc = fopen(fileName, "r");
            if(!cpsrc){
                printf("The source file doesn't exist\n");
                return FILE_NOT_EXISTING;
            }
            int maxS = BLOCK_SIZE*fssb.dataBlockNum;
            char sourceBuff[maxS];
            fread(sourceBuff, maxS, 1, cpsrc);
            unsigned long fileSize = (unsigned long)strlen(sourceBuff);
            fclose(cpsrc);

            int reqBlocks = 1 + (fileSize / BLOCK_SIZE);
            int i;
            for(i = 0; i < fssb.iNodesCount; ++i){
                if(iNodeBitmap[i] == '\0'){
                    for(int j = 0; j < fssb.dataBlockNum; ++j){
                        if(dataBitmap[j] == '\0'){
                            int notAv = 0;
                            for(int k = 0; k < reqBlocks; ++k){
                                if(dataBitmap[j + k] == '\0')
                                    continue;
                                notAv = 1;
                            }
                            if(notAv)
                                continue;
                            iNodeTable[i].startBlock = fssb.dataBlockOffset+j;
                            iNodeTable[i].accRights = 0x07;
                            iNodeTable[i].size = fileSize;
                            time_t now;
                            time(&now);
                            iNodeTable[i].lastAcc = now;
                            strcpy(iNodeTable[i].name, fileName);

                            fp = fopen(sysName, "r+b");

                            fseek(fp, fssb.iNodeBitmapOffset*BLOCK_SIZE + i*sizeof(char), SEEK_SET);
                            fwrite(&occ, sizeof(char), 1, fp);

                            for(int z = 0; z < reqBlocks; ++z){
                                fseek(fp, fssb.dataBitmapOffset*BLOCK_SIZE + (j+z)*sizeof(char), SEEK_SET);
                                fwrite(&occ, sizeof(char), 1, fp);
                            }

                            fseek(fp, fssb.iNodesOffset*BLOCK_SIZE + i * fssb.iNodeSize, SEEK_SET);
                            fwrite(iNodeTable+i, fssb.iNodeSize, 1, fp);

                            fseek(fp, (fssb.dataBlockOffset+j)*BLOCK_SIZE, SEEK_SET);
                            fwrite(sourceBuff, fileSize, 1, fp);
                            break;
                        }
                    }
                    break;
                }
            }
            if(i == fssb.iNodesCount){
                printf("Not enough iNodes!");
                return NO_INODES;
            }
            argCount++;
        }
        puts("Successfully copied all files!");

        return 0;
    }
    return 0;
}
