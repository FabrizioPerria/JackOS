#ifndef __FILE_SYSTEM_H
#define __FILE_SYSTEM_H

#define FS_FILE         0
#define FS_DIRECTORY    1
#define FS_FILE_INVALID 2

typedef struct _FILE{
	char name[32];
	int flags;
	int length;
	int id;
	int eof;
	int position;
	int currentCluster;
	int deviceID;
	char mode;
}FILE,*FILE_PTR;

typedef struct _FILESYSTEM{
	char name[8];
	FILE (*Directory)(const char *directoryName,FILE* folder);
	void (*mount)();
	void (*read)(FILE_PTR file,unsigned char *buffer,unsigned int length);
	void (*close)(FILE_PTR file);
	FILE (*open)(const char *fileName);
}FILESYSTEM,*FILESYSTEM_PTR;


FILE openFile(char *fileName,char mode);
void readFile(FILE_PTR file,unsigned char *buffer,unsigned int length);
void closeFile(FILE_PTR file);
void registerFS(FILESYSTEM_PTR newFS,int deviceId);
void unregisterFS(unsigned int deviceID);

#endif
