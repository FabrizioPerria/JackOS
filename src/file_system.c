#include <file_system.h>
#include <system.h>
#include <string.h>

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
	if(isNumber(fileName[0])){
		if(fileName != NULL && fs[fileName[0]-48]->present == 1){
			file=fs[fileName[0]-48]->open(fileName);

			if(file.flags == FS_FILE_INVALID && mode == 'w'){
				/*Create a file*/
			}

			file.mode = mode;
			return file;
		}
	}
	file.flags = FS_FILE_INVALID;
	return file;
}

void deleteFile(char *fileName)
{
	int fsIndex=0;
	if(isNumber(fileName[0])){
		fsIndex = fileName[0]-48;
		if(fileName != NULL && fileName[1]=='/'){
			fs[fsIndex]->remove(fileName);
		}
	}
}

int readFile(FILE *file,unsigned char *buffer,unsigned int length)
{
	int lengthBlocks=0;

	if(file!= NULL && file->mode == 'r'){
		if(fs[file->deviceID]){
			lengthBlocks=length/512;
			if(length%512 != 0)
				lengthBlocks++;
			return fs[file->deviceID]->read(file,buffer,lengthBlocks);
		}
	}
	return 0;
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

FILE *listFile(char *folder)
{
	FILE *numElements = NULL;
	FILE folderFile;

	if(folder == NULL)
		return numElements;

	if(strlen(folder) > 2 && folder[strlen(folder)-1] == '/')
		folder[strlen(folder)-1] = 0;

	folderFile = openFile(folder,'r');
	if(folderFile.flags == FS_FILE_INVALID){
		/*print("Cannot find %s\r\n",folder);*/
		return numElements;
	}
	numElements = fs[folderFile.deviceID]->list(folderFile);

	return numElements;
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
		fs[deviceId]->present = 1;
		fs[deviceId]=newFS;
		i++;
	}
}

void unregisterFS(unsigned int deviceID)
{
	if(deviceID < MAX_DEVICES)
		fs[deviceID]=NULL;
}
