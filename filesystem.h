#ifndef FILESYSTEM_H_INCLUDED
#define FILESYSTEM_H_INCLUDED

#include <time.h>

#define FS_DEF_NAME "filesys.fs"
#define FS_INODE_SIZE 128
#define FS_INODES 160
#define BLOCK_SIZE 4096

typedef struct fsys_SuperBlock{
    int blockSize;

    int iNodeSize;
    int iNodesCount;
    int iNodesBlockNum;
    int iNodesOffset;

    int dataBlockNum;
    int dataBlockOffset;

    int iNodeBitmapOffset;
    int dataBitmapOffset;
} fsys_SuperBlock;

typedef struct fsys_iNodeBitmap{
    char bitmap[FS_INODES];
} fsys_iNodesBitmap;

typedef struct fsys_dataBitmap{
    char bitmap[50000];
} fsys_dataBitmap;

typedef struct fsys_iNode{
    int startBlock;
    char accRights;
    int size;
    time_t lastAcc;
    char name[96];
} fsys_iNode;

//errors
#define FILE_ALREADY_EXISTS     -1
#define NO_INODES               -2
#define FSYS_ALREADY_CREATED    -3
#define NO_SUCH_FSYS            -4
#define COULDNT_CREATE_FSYS     -5
#define MISSING_ARGS            -6
#define INCORRECT_ARG           -7
#define FILE_NOT_EXISTING       -8
#define FILE_NOT_IN_FSYS        -9

#endif // FILESYSTEM_H_INCLUDED
