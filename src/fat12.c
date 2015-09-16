#include <system.h>
#include <screen.h>
#include <fat12.h>
#include <string.h>
#include <diskPIO.h>
#include <BPB.h>

FILESYSTEM _fs;
struct MOUNT_INFO _mi[MAX_DEVICES];

static void FAT12NoSpaces(char *name)
{
    int i=strlen(name);
    for(;i>=0;i--){
        if(name[i] <= ' ')  name[i] = 0;
    }
}


void FAT12Init(int drive)
{
	if(drive < 0)
		return;
	/* Initialize and mount the FAT12 partition */
	strcpy(_fs.name,"FAT 12",6);
	_fs.Directory = FAT12Directory;
	_fs.mount = FAT12Mount;
	_fs.read = FAT12Read;
	_fs.close = FAT12Close;
	_fs.open = FAT12Open;
	_fs.remove = FAT12Remove;
	_fs.list = FAT12List;
	_fs.present = 1;
	registerFS(&_fs,0);

	FAT12Mount(drive);
}

FILE FAT12Create(char *fileName)
{
	char *str;
/*
create an empty file

Need a search function to find the first free cluster in the FAT 
	- check the root directory
	- for each element go over the clusters until you find free cluster
	- remember: the clusters don't have to be consecutives
*/
	FILE f = FAT12Directory(fileName[0]-48,fileName+2,-1,NULL);
	if(f.flags != FS_FILE_INVALID)
		/* If the file already exists, return it! */
		return f;

	str = strtok(fileName,'/',1);
	(void)str;
	return f;
}


FILE *FAT12List(FILE folder)
{

	/*f = FAT12Directory(folder,NULL);*/
	static FILE chain[224];
	FILE tmp;
	int j = 0;
	/*int phySector;*/
	/*char buffer[512];*/
	/*struct fat12Entry* directory;*/
	memset((unsigned char *)chain,0,224 * sizeof(FILE));
	if(folder.flags == FS_DIRECTORY){
		/* Provide an array of files contained in the folder */
		for(j = 0;j < (int)(folder.length / sizeof(struct fat12Entry));j++){
			memset((unsigned char*)&tmp,0,sizeof(FILE));
			tmp = FAT12Directory(folder.deviceID,NULL,j,&folder);
			if(strlen(tmp.name) && tmp.flags != FS_FILE_INVALID)
				chain[j] = tmp;
		}
		return chain;
	}else if(folder.flags == FS_FILE){
		/* list of a file; provide only the file itself */
		memcpy((unsigned char *)&chain[0],(unsigned char*)&folder,sizeof(folder));
		return chain;
	}else{
		/* File or folder not found... */
		return NULL;
	}
}

void FAT12Remove(char *filename)
{
	unsigned char buffer[512],fat[512];
	struct fat12Entry *directory;
	char tmpName[32];
	int i=0,FAT_offset=0,FAT_sector=0,nextCluster=0;

	/* Search a file using the name and set 0 on the pointed clusters */
	FILE file = FAT12Directory(filename[0]-48,filename+2,-1,NULL);
	for(i = 0;i<strlen(file.name);i++)
		file.name[i] = charToUpper(file.name[i]);

	if(readLBA28(file.deviceID,file.indexPosition,1,buffer)<1){
		print("Cannot find the directory\r\n");
		return;
	}
	directory = (struct fat12Entry*)buffer;

	/* Scan all the directories in the sector to find the file */
    memset((unsigned char *)tmpName,0,32);
    strcpy(tmpName,directory->name,8);
    FAT12NoSpaces(tmpName);
    if(directory->attribute != 0x10){
	    tmpName[strlen(tmpName)]='.';
    	strcpy(tmpName+strlen(tmpName),directory->extension,3);
    }

	while(strncmp(tmpName,file.name,strlen(file.name))){
		directory++;
	    memset((unsigned char *)tmpName,0,32);
    	strcpy(tmpName,directory->name,8);
    	FAT12NoSpaces(tmpName);
    	if(directory->attribute != 0x10){
        	tmpName[strlen(tmpName)]='.';
        	strcpy(tmpName+strlen(tmpName),directory->extension,3);
    	}
	}
	/* Use the position declared in the directory to free the FAT entries*/
	while(1){
		FAT_offset = file.currentCluster * 1.5;
    	FAT_sector = 1 + (FAT_offset/_mi[file.deviceID].sectorSize);
    	if(readLBA28(file.deviceID,FAT_sector,1,fat) < 1){
			print("Cannot find the FAT table\r\n");
			return;
		}

        nextCluster = fat[FAT_offset % _mi[file.deviceID].sectorSize]+ (fat[(FAT_offset%_mi[file.deviceID].sectorSize)+1]*0x100);
        if(file.currentCluster %2){
        	nextCluster =(nextCluster & 0xFFF0) >> 4;
			fat[(FAT_offset % _mi[file.deviceID].sectorSize)+1] =0x0;
			fat[FAT_offset % _mi[file.deviceID].sectorSize] &= 0xF;
        }else{
        	nextCluster &=0x0FFF;
			fat[FAT_offset % _mi[file.deviceID].sectorSize] =0x0;
			fat[(FAT_offset % _mi[file.deviceID].sectorSize)+1] &=0xF0;
        }

		writeLBA28(file.deviceID,FAT_sector,1,fat);
		writeLBA28(file.deviceID,FAT_sector+_mi[file.deviceID].fatSize,2,fat);

        if((nextCluster == 0) ||(nextCluster >= 0xff8)){
            file.currentCluster = directory->startingCluster;
            break;
        }else{
            file.currentCluster = nextCluster;
        }
	}

	memset((unsigned char*)directory,0,sizeof(struct fat12Entry));

	if(writeLBA28(file.deviceID,file.indexPosition,1,buffer)>0)
		print("\r\nDeleted %s\r\n",filename);
}



void FAT12Write(FILE_PTR file,unsigned char *buffer,unsigned int length)
{
	/* Put the content of the file in the buffer */
	unsigned int i=0;
	int phySector=0;
	int FAT_offset=0,FAT_sector=0;
	int nextCluster=0;
	unsigned char *fat = NULL;

	if(file){
		while(i<length){
			phySector = 32 + (file->currentCluster-1);
			writeLBA28(file->deviceID,phySector,1,buffer+(i*512));
			while(1){
				FAT_offset = file->currentCluster * 1.5;
				FAT_sector = 1 + (FAT_offset/_mi[file->deviceID].sectorSize);
				readLBA28(file->deviceID,FAT_sector,2,fat);
				nextCluster = fat[FAT_offset % _mi[file->deviceID].sectorSize]+(fat[(FAT_offset%_mi[file->deviceID].sectorSize)+1]*0x100);

				if(file->currentCluster %2){
					nextCluster =(nextCluster & 0xFFF0) >> 4;
				}else{
					nextCluster &=0x0FFF;
				}
			}
			if((nextCluster == 0) ||(nextCluster >= 0xff8)){
				file->eof = 1;
				return;
			}
			file->currentCluster = nextCluster;
			i++;
		}
	}
}

int FAT12Read(FILE_PTR file,unsigned char *buffer,unsigned int length)
{
	/* Put the content of the file in the buffer */
	unsigned int i=0;
	int phySector=0;
	int FAT_offset=0,FAT_sector=0;
	int nextCluster=0;
	unsigned char fat[512];

	if(file != NULL && buffer != NULL && length > 0){
		while(!file->eof){
			phySector = 32 + (file->currentCluster-1);
			readLBA28(file->deviceID,phySector,1,buffer+(i*512));
			FAT_offset = file->currentCluster * 1.5;
			FAT_sector = 1 + (FAT_offset/_mi[file->deviceID].sectorSize);
			readLBA28(file->deviceID,FAT_sector,1,fat);
			nextCluster = fat[FAT_offset % _mi[file->deviceID].sectorSize]+
				(fat[(FAT_offset%_mi[file->deviceID].sectorSize)+1]*0x100);

			if(file->currentCluster %2){
				nextCluster =(nextCluster & 0xFFF0) >> 4;
			}else{
				nextCluster &=0x0FFF;
			}
			if((nextCluster == 0) ||(nextCluster >= 0xff8)){
				file->eof = 1;
				return _mi[file->deviceID].sectorSize * length;
			}
			file->currentCluster = nextCluster;
			i++;
		}
	}
	return 0;
}

/*Convert a file name into a 8.3 DOS file format name*/
static void name2DOS(char *fileName, char *dosName)
{
	int i=0,j=0;
	if((fileName == NULL) || (strlen(fileName) > 11) || (dosName == NULL))
		return;

	memset((unsigned char*)dosName,' ',11);
	/* 11 is the maximum length of a name is DOS (with extension)*/
	for(i=0,j=0; (i < strlen(fileName));i++,j++){
		if(fileName[i] == '.'){
			j=7;
			continue;
		}
		dosName[j] = charToUpper(fileName[i]);
	}
	dosName[11]=0;
}

/* Locate file or directory or return the n-th file in the folder(if the name of file is not specified */
FILE FAT12Directory(int drive, char *name, int n, FILE *folder)
{
	FILE f;
	unsigned char buffer[512],fat[512];
	char dosName[12],tmpName[12],bufName[50];
	int i=0,j,k=-1,phySector,FAT_offset,FAT_sector,nextCluster,cnt=1;
	struct fat12Entry *directory;

	if(name[0] == ' ' || name[0] == 0){
		strcpy(f.name,"drive",8);
		f.id = 0;
		f.length = _mi[drive].numRootEntries * sizeof(struct fat12Entry);
		f.eof = 0;
		f.position = _mi[drive].rootPosition;
		f.currentCluster = _mi[drive].rootPosition;
		f.deviceID = drive;
		f.flags = FS_DIRECTORY;
		f.indexPosition = 0;
		return f;
	}

	if(name != NULL && name[0] != 0){
		name2DOS(strtok(name,'/',0),dosName);
	}

	while(1){
		if(folder == NULL || folder->position == _mi[drive].rootPosition){
			/*Root directory*/
			/* 224 entries of 32 Bytes in the root directory with sectors of 512 bytes
			Num_sectors_root = (224*32)/512 = 14 sectors for the root directory */

			if(i < 14){
				phySector = _mi[drive].rootPosition + i;
				i++;
			}else{
				f.flags = FS_FILE_INVALID;
				return f;
			}
		}else{
			/* Go through directories */
			phySector = 32 + (folder->currentCluster-1);
		}
		if(n>=0)
			phySector += ((n * sizeof(struct fat12Entry))/_mi[drive].sectorSize);

		if(readLBA28(drive,phySector,1,buffer) < 1){
			f.flags = FS_FILE_INVALID;
			return f;
		}
		directory = (struct fat12Entry*)buffer;

		if(n >= 0 && name == NULL){
			directory += n;
			if(strlen(directory->name) == 0){
				f.flags = FS_FILE_INVALID;
				return f;
			}
			memset((unsigned char *)f.name,0,FILENAME_LENGTH);
			strcpy(f.name,directory->name,8);
			FAT12NoSpaces(f.name);
			if(directory->attribute != 0x10){
				f.name[strlen(f.name)]='.';
				strcpy(f.name+strlen(f.name),directory->extension,3);
			}
			f.id = 0;
			f.length = directory->fileSize;
			f.eof = 0;
			f.position = directory->startingCluster;
			f.currentCluster = directory->startingCluster;
			f.deviceID = drive;
			f.indexPosition = phySector;
			if(directory->attribute == 0x10)
				f.flags = FS_DIRECTORY;
			else
				f.flags = FS_FILE;

			return f;
		}
		/* Num_root_entries_per_sector = 512 Bytes of sector / 32Bytes of entry = 16 */ 

		for(j=0;j<16;j++){
			memset((unsigned char *)tmpName,0,12);
			strcpy(tmpName,directory->name,8);
			strcpy(tmpName+strlen(tmpName),directory->extension,3);
			if(!strncmp(tmpName,dosName,strlen(tmpName))){
				strcpy(f.name,name,strlen(tmpName));
				f.id = 0;
				f.length = directory->fileSize;
				f.eof = 0;
				f.position = directory->startingCluster;
				f.currentCluster = directory->startingCluster;
				f.deviceID = drive;
				f.indexPosition = phySector;
				if(directory->attribute == 0x10)
					f.flags = FS_DIRECTORY;
				else
					f.flags = FS_FILE;

				k=strFindChar(name,'/');

				if(k > 0){
					strcpy(bufName,name,50);
					if(folder == NULL)
						f=FAT12Directory(drive,substr(bufName,k+1,0),-1,&f);
					else
						f=FAT12Directory(drive,substr(bufName,k,strlen(name)),-1,&f);

				}else if(f.flags == FS_DIRECTORY){
					while(1){
						FAT_offset = f.currentCluster * 1.5;
						FAT_sector = 1 + (FAT_offset/_mi[drive].sectorSize);
						readLBA28(drive,FAT_sector,1,fat);
						nextCluster = fat[FAT_offset % _mi[drive].sectorSize]+
							(fat[(FAT_offset%_mi[drive].sectorSize)+1]*0x100);
						if(f.currentCluster %2){
							nextCluster =(nextCluster & 0xFFF0) >> 4;
						}else{
							nextCluster &=0x0FFF;
						}
						if((nextCluster == 0) ||(nextCluster >= 0xff8)){
							f.length=cnt*512;
							f.currentCluster = directory->startingCluster;
							break;
						}else{
							cnt++;
							f.currentCluster = nextCluster;
						}
					}
				}
				return f;
			}
			directory++;
		}
	}
		/* Impossible to find a file */
	f.flags = FS_FILE_INVALID;
	return f;
}

/* Return a reference of a file or returns a file with FS_FILE_INVALID flag set if not found */
FILE FAT12Open(char *name)
{
	FILE file;
	if(name == NULL){
		file.flags = FS_FILE_INVALID;
		return file;
	}

	file = FAT12Directory(name[0]-48,name+2,-1,NULL);

	if(file.flags != FS_FILE && file.flags != FS_DIRECTORY){
		/*File not found */
		file.flags = FS_FILE_INVALID;
	}

	return file;
}

/* Invalidate a file */
void FAT12Close(FILE_PTR file)
{
	if(file)
		file->flags = FS_FILE_INVALID;
}

/* Mount FAT12 file system */
int FAT12Mount(int drive)
{
	unsigned char bootsectorBuffer[512];
	struct bpb *bootsector;
	if(readLBA28(drive,0,1,bootsectorBuffer) < 1)
		return -1;
	bootsector = (struct bpb*)bootsectorBuffer;

	_mi[drive].numSectors = bootsector->totalSectors;
	_mi[drive].sectorSize = bootsector->bytesPerSector;
	_mi[drive].fatPosition = 1;
	_mi[drive].fatSize = bootsector->sectorsPerFAT;
	_mi[drive].fatEntrySize = 8;
	_mi[drive].numRootEntries = bootsector->maxNumberEntriesRoot;
	_mi[drive].rootPosition = (bootsector->numberFATs * bootsector->sectorsPerFAT)+1;

	_mi[drive].rootSize = (bootsector->maxNumberEntriesRoot *32) / bootsector->bytesPerSector;
	return _mi[drive].sectorSize;
}

