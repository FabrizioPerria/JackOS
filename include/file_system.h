#ifndef __FILE_SYSTEM_H
#define __FILE_SYSTEM_H

#define MAX_DEVICES 8

#define FS_FILE_INVALID 0
#define FS_FILE 1
#define FS_DIRECTORY 2

#define FILENAME_LENGTH 32

typedef struct _FILE
{
    char name[FILENAME_LENGTH];
    int flags;
    int length;
    int id;
    int eof;
    int indexPosition;
    int position;
    int currentCluster;
    int deviceID;
    char mode;
} FILE, *FILE_PTR;

typedef struct _FILESYSTEM
{
    char name[8];
    char present;
    void (*Directory) (int drive, char* directoryName, FILE* folder, FILE* f);
    int (*mount) (int drive);
    void (*remove) (char* fileName);
    void (*list) (FILE* folder, FILE chain[]);
    int (*read) (FILE_PTR file, unsigned char* buffer, unsigned int length);
    int (*write) (FILE_PTR file, unsigned char* buffer, unsigned int length);
    void (*close) (FILE_PTR file);
    void (*open) (char* fileName, char mode, FILE_PTR file);
} FILESYSTEM, *FILESYSTEM_PTR;

void openFile (char* fileName, char mode, FILE_PTR file);
int readFile (FILE_PTR file, unsigned char* buffer, unsigned int length);
int writeFile (FILE_PTR file, unsigned char* buffer, unsigned int length);
void closeFile (FILE_PTR file);
void registerFS (FILESYSTEM_PTR newFS, int deviceId);
void unregisterFS (unsigned int deviceID);
void deleteFile (char* fileName);
void listFile (char* folder, int* numElements, FILE chain[]);

#endif
