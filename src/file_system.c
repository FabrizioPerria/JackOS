#include <file_system.h>
#include <system.h>

#define MAX_DEVICES 8

static FILESYSTEM *fs[MAX_DEVICES];

/* the filename will be with format 
   N/directoryX/flename 
   where N is the index in the filesystem lookup table */
static int isNumber(char c)
{
	return ((c >= '0') && (c <= '9'));
}

FILE openFile(char *fileName,char mode)
{
	FILE file;
	int fsIndex;
	if(isNumber(fileName[0])){
		fsIndex = fileName[0]-48;
		if(fileName != NULL && fileName[1]=='/'){
			file=fs[fsIndex]->open(fileName+2);

			if(file.flags == FS_FILE_INVALID && mode == 'w'){
				/*Create a file*/
			}

			file.deviceID=fsIndex;
			file.mode = mode;
			return file;
		}
	}
	file.flags = FS_FILE_INVALID;
	return file;
}

void deleteFile(const char *fileName)
{
	int fsIndex=0;
	if(isNumber(fileName[0])){
		fsIndex = fileName[0]-48;
		if(fileName != NULL && fileName[1]=='/'){
			fs[fsIndex]->remove(fileName+2);
		}
	}
}

void readFile(FILE *file,unsigned char *buffer,unsigned int length)
{
	int lengthBlocks=0;

	if(file!= NULL && file->mode == 'r'){
		if(fs[file->deviceID]){
			lengthBlocks=length/512;
			if(length%512 != 0)
				lengthBlocks++;
			fs[file->deviceID]->read(file,buffer,lengthBlocks);
		}
	}
}

void writeFile(FILE *file, unsigned char *buffer,unsigned int length)
{
	(void)file;
	(void)buffer;
	(void)length;
/*
	if(file != NULL){
		if(file->mode == 'a'){
			Append data to the existing file; the file must exist
		} else if(file->mode == 'w'){
			Overwrite the existing content
		}
	}*/
}

void listFile(const char *folder)
{
	int numElements = 0;
	FILE folderFile = openFile(folder,'r');
	if(folderFile.flags == FS_FILE_INVALID){
		print("Cannot find %s\r\n",folder);
		return;
	}
	numElements = fs[folderFile.deviceID]->list(folderFile);
	while(numElements > 0){
		print("QUI\r\n");
	}
}

void closeFile(FILE *file)
{
	if(file!= NULL){
		if(fs[file->deviceID])
			fs[file->deviceID]->close(file);
	}
}

void registerFS(FILESYSTEM_PTR newFS,int deviceId)
{
	static int i = 0;
	if(i < MAX_DEVICES && newFS){
		fs[deviceId]=newFS;
		i++;
	}
}

void unregisterFS(unsigned int deviceID)
{
	if(deviceID < MAX_DEVICES)
		fs[deviceID]=NULL;
}
