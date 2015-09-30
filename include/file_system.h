#ifndef __FILE_SYSTEM_H
#define __FILE_SYSTEM_H

#define MAX_DEVICES 8

#define FS_FILE_INVALID 0
#define FS_FILE         1
#define FS_DIRECTORY    2

#define FILENAME_LENGTH 32
typedef struct _FILE{
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
}FILE,*FILE_PTR;

typedef struct _FILESYSTEM{
	char name[8];
	char present;
	FILE (*Directory)(int drive,char *directoryName,int n,FILE* folder);
	int  (*mount)(int drive);
	void (*remove)(char *fileName);
	FILE_PTR (*list)(FILE folder);
	int (*read)(FILE_PTR file,unsigned char *buffer,unsigned int length);
	void (*close)(FILE_PTR file);
	FILE (*open)(char *fileName);
}FILESYSTEM,*FILESYSTEM_PTR;

FILE openFile(char *fileName,char mode);
int readFile(FILE_PTR file,unsigned char *buffer,unsigned int length);
void closeFile(FILE_PTR file);
void registerFS(FILESYSTEM_PTR newFS,int deviceId);
void unregisterFS(unsigned int deviceID);
void deleteFile(char *fileName);
FILE* listFile(char *folder);

#endif
