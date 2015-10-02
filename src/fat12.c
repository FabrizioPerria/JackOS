#include <RTC.h>
#include <system.h>
#include <screen.h>
#include <fat12.h>
#include <string.h>
#include <diskPIO.h>
#include <BPB.h>

FILESYSTEM _fs;
struct MOUNT_INFO _mi[MAX_DEVICES];


/* Delete the spaces from a string */
static void removeSpaces(char *name)
{
	int i=strlen(name);
	for(;i>=0;i--){
		if(name[i] <= ' ')
			name[i] = 0;
	}
}

/* Return the drive given a path */
static int getDriveFromPath(char *path)
{
	if (path != NULL){
		return path[0]-48;
	}
	return -1;
}

/*Convert a file name into a 8.3 DOS file format name returned through dosName*/
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

/*get the next Cluster index of a file in the FAT
* drive: drive which contains the file
* index: position of the entry in the table
* fat: array with the fat
*/
static int getNextClusterFromFAT(int drive, int index, unsigned char *fat)
{
	int FAT_offset = 0,next = 0;
	FAT_offset = index * 1.5;
	next = fat[FAT_offset % _mi[drive].sectorSize] + (fat[FAT_offset % _mi[drive].sectorSize+1]*0x100);
	if (index % 2)
		return (next & 0xFFF0) >> 4;
	else
		return next & 0x0FFF;
}

/* Write a value in the FAT table
* drive: drive containing the fat
* sector: sector containing the fat
* offset: position of the entry to be written in the table
* index: position of the file
* value to be written
* fat: table
*/
static void writeOnFAT(int drive,int sector,int offset,int index,int value,unsigned char *fat)
{
	if(index % 2){
		fat[(offset % _mi[drive].sectorSize) + 1] = (value & 0xFF0) >> 4;
		fat[offset % _mi[drive].sectorSize] &=  0xF;
		fat[offset % _mi[drive].sectorSize] += (value & 0xF) << 4;
	} else {
		fat[offset % _mi[drive].sectorSize] = (value & 0xFF);
		fat[(offset % _mi[drive].sectorSize) + 1] &= 0xF0;
		fat[(offset % _mi[drive].sectorSize) + 1] += (value >> 8);
	}

	writeLBA28(drive,sector,1,fat);
	writeLBA28(drive,sector + _mi[drive].fatSize,1,fat);
}

/* Calculate and read one sector of the FAT and calculate in which byte is represented the index-th file */ 
static void getFAT(int drive,int index,int *FAT_offset,int *FAT_sector,unsigned char *fat)
{
	int j=0,off=0;
	off = index * 1.5;
	j = 1 + (off/_mi[drive].sectorSize);
	if(FAT_sector == NULL)
		readLBA28(drive,j,1,fat);
	else if ((FAT_sector !=NULL) && (j != *FAT_sector)){
		*FAT_sector = j;
		readLBA28(drive,*FAT_sector,1,fat);
	}

	if(FAT_offset!= NULL)
		*FAT_offset = off;
}

/* Initialize and mount a file system */
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

/*Create a file in the path specified */
FILE FAT12Create(char *fileName)
{
	FILE folder,file;
	FILE_PTR list;
	int i,j=1,tmp =0, len=0, nextEntry = 0, nextCluster = 0,pos=0,phySector=0, drive=-1;
	unsigned char fat[512],buffer[512];
	unsigned char *time;
	char *tmpName,name[12];
	struct fat12Entry *directory;
/*
create an empty file

Need a search function to find the first free cluster in the FAT 
	- check the root directory
	- for each element go over the clusters until you find free cluster
	- remember: the clusters don't have to be consecutives
*/

	drive = getDriveFromPath(fileName);
	if (drive < 0){
		folder.flags = FS_FILE_INVALID;
		return folder;
	}

	/* If the file already exists, return it */
	file = FAT12Directory(drive,fileName+2,NULL);
	if(file.flags != FS_FILE_INVALID)
		return file;

	for (i = 0;; i++){
		tmp = strlen(strtok(fileName,'/',i));
		len += tmp;
		if(fileName[len] == '/'){
			len++;
		} else {
			len -= (tmp + 1);
			if (len == 1)
				/* Root folder */
				folder = FAT12Directory(drive," ",NULL);
			else
				folder = FAT12Directory(drive,substr(fileName,2,tmp),NULL);

			if (folder.flags == FS_FILE_INVALID)
				return folder;
			break;
		}
	}
	/* folder points to the folder location */

	list = FAT12List(folder);

	while(strlen(list->name) != 0 && list->flags != FS_FILE_INVALID){
		list++;
		pos++;
	}

	if(pos > 0 && (pos % 16 == 0)){
		/*TODO: expand the folder in the fat */
	}

	/* Find free cluster in the FAT */
	for(i = 0; i< 9 ; i++){
		readLBA28(drive,i + _mi[drive].fatPosition ,1,fat);

		/*The first 3 Entries at the beginning of the table are standard*/
		if(i==0)
			nextEntry = 4;

		while(nextEntry < ((_mi[drive].sectorSize * 8) / 12)){
			nextCluster = getNextClusterFromFAT(drive,nextEntry,fat);
			if(nextCluster == 0){
				/* Found an available entry */
				writeOnFAT(drive,_mi[drive].fatPosition + i, nextEntry*1.5,nextEntry,0xFFF,fat);
				break;
			} else {
				if(j%2)
					nextEntry+=2;
				else
					nextEntry++;
			}
			j++;
		}
		if(nextEntry < ((_mi[drive].sectorSize*8) /12)){
			break;
		}
	}
	/* nextEntry is the position in the FAT */
	if(len > 1){
		phySector = 32 + (folder.currentCluster-1);
	}else {
		phySector = _mi[drive].rootPosition;
	}

	phySector += ((pos / sizeof(struct fat12Entry))/_mi[drive].sectorSize);

	readLBA28(drive,phySector,1,buffer);

	directory = (struct fat12Entry*)buffer;

	directory += (pos%16);

	tmpName = substr(fileName,len+1,strlen(fileName));
	name2DOS(tmpName,name);
	strcpy(directory->name,strtok(name,'.',0),8);
	strcpy(directory->extension,substr(name,strlen(directory->name),3),3);
	directory->attribute = FS_FILE;
	directory->startingCluster = nextEntry;
	directory->fileSize = _mi[drive].sectorSize;

	time = getTimeDate();
	/* time is an array of bytes with the folloing values:
		    6        5      4    3   2      1      0
		|highYear|lowYear|month|day|hour|minute|seconds|
	*/
	directory->time = ((time[2] & 0x1F)<<11)||((time[1] & 0x3F) << 5)||(time[0] & 0x1F);
	directory->date = (((time[5]-80) & 0x5F) << 9) || ((time[4] & 0xF) << 5) || (time[3] & 0x1F);
	/* Write the record */
	writeLBA28(drive,phySector,1,buffer);

	/* return the file created */
	return FAT12Directory(drive,fileName+2,&folder);
}


/*Get the list of files contained in a folder */
FILE *FAT12List(FILE folder)
{
	static FILE chain[224];
	FILE tmp;
	int i=0, j = 0, phySector=0,cnt=0,nextCluster=0;
	unsigned char buffer[512],fat[512];
	struct fat12Entry *directory;
/*****************************************************************************/
	static int l=0;
	if(l==0){
		l++;
		FAT12Create("0/folder/crea.txt");
	}
/*****************************************************************************/

	memset((unsigned char *)chain,0,224 * sizeof(FILE));
	if(folder.flags == FS_DIRECTORY){
		for (j=0; j < (int)folder.length; j++){
			if(folder.position == _mi[folder.deviceID].rootPosition){
				/*Root directory*/
				i = _mi[folder.deviceID].rootPosition;

			}else{
				/* Go through directories */
				i = 32 + (folder.currentCluster-1);
			}

			i += ((j * sizeof(struct fat12Entry))/_mi[folder.deviceID].sectorSize);

			if(phySector != i){
				phySector = i;
				readLBA28(folder.deviceID,phySector,1,buffer);
			}

			directory = (struct fat12Entry*)buffer;

			memset((unsigned char*)&tmp,0,sizeof(FILE));

			directory += (j % (_mi[folder.deviceID].sectorSize / sizeof(struct fat12Entry)));
			if(strlen(directory->name) == 0){
				continue;
			}
			memset((unsigned char *)tmp.name,0,FILENAME_LENGTH);
			strcpy(tmp.name,directory->name,8);
			removeSpaces(tmp.name);
			if(directory->attribute != 0x10){
				tmp.name[strlen(tmp.name)]='.';
				strcpy(tmp.name+strlen(tmp.name),directory->extension,3);
			}
			tmp.id = 0;
			tmp.eof = 0;
			tmp.position = directory->startingCluster;
			tmp.currentCluster = directory->startingCluster;
			tmp.deviceID = folder.deviceID;
			tmp.indexPosition = phySector;
			if(directory->attribute == 0x10){
				tmp.flags = FS_DIRECTORY;
				cnt = 0;
				while(1){
					getFAT(folder.deviceID,tmp.currentCluster,NULL,NULL,fat);
					nextCluster = getNextClusterFromFAT(folder.deviceID,tmp.currentCluster,fat);
					if((nextCluster == 0) ||(nextCluster >= 0xff8)){
						tmp.length=(cnt*512) / sizeof(struct fat12Entry);
						tmp.currentCluster = directory->startingCluster;
						break;
					}else{
						cnt++;
						tmp.currentCluster = nextCluster;
					}
				}
			}else{
				tmp.flags = FS_FILE;
				tmp.length = directory->fileSize;
			}

			if(strlen(tmp.name) && tmp.flags != FS_FILE_INVALID){
				chain[j] = tmp;
			}
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

/* Delete a file */
void FAT12Remove(char *filename)
{
	unsigned char buffer[512],fat[512];
	struct fat12Entry *directory;
	char tmpName[32];
	int i=0,FAT_offset=0,FAT_sector=0,nextCluster=0,drive = -1;
	FILE file;

	if (filename == NULL)
		return;

	drive = getDriveFromPath(filename);
	if (drive < 0)
		return;

	/* Search a file using the name and set 0 on the pointed clusters */
	file = FAT12Directory(drive,filename+2,NULL);
	if(file.flags == FS_FILE_INVALID)
		return;
	for(i = 0;i<strlen(file.name);i++)
		file.name[i] = charToUpper(file.name[i]);

	readLBA28(file.deviceID,file.indexPosition,1,buffer);
	directory = (struct fat12Entry*)buffer;

	/* Scan all the directories in the sector to find the file */
	memset((unsigned char *)tmpName,0,32);
	strcpy(tmpName,directory->name,8);
	removeSpaces(tmpName);
	if(directory->attribute != 0x10){
		tmpName[strlen(tmpName)]='.';
		strcpy(tmpName+strlen(tmpName),directory->extension,3);
	}

	while(strncmp(tmpName,file.name,strlen(file.name))){
		directory++;
		memset((unsigned char *)tmpName,0,32);
		strcpy(tmpName,directory->name,8);
		removeSpaces(tmpName);
		if(directory->attribute != 0x10){
			tmpName[strlen(tmpName)]='.';
			strcpy(tmpName+strlen(tmpName),directory->extension,3);
		}
	}
	/* Use the position declared in the directory to free the FAT entries*/
	while(1){
		getFAT(drive,file.currentCluster,&FAT_offset,&FAT_sector,fat);

		nextCluster = getNextClusterFromFAT(file.deviceID,file.currentCluster,fat);

		writeOnFAT(drive,FAT_sector,FAT_offset,file.currentCluster, 0 ,fat);

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

/* Write a file in the disk */
void FAT12Write(FILE_PTR file,unsigned char *buffer,unsigned int length)
{
	/* Put the content of the file in the buffer */
	unsigned int i=0;
	int phySector=0;
	int nextCluster=0;
	unsigned char *fat = NULL;

	if(file != NULL && buffer != NULL && length > 0){
		/* TODO: length % 512 tells you how much sectors you will need to store the data */
		while(i<length){
			phySector = 32 + (file->currentCluster-1);
			writeLBA28(file->deviceID,phySector,1,buffer+(i*512));

			getFAT(file->deviceID,file->currentCluster,NULL,NULL,fat);
			nextCluster = getNextClusterFromFAT(file->deviceID,file->currentCluster,fat);
			if((nextCluster == 0) ||(nextCluster >= 0xff8)){
				/* TODO: handle the nextCluster value, maybe you need to increase.....check length to see how much data you 
					have in the buffer */
				file->eof = 1;
				return;
			}
			file->currentCluster = nextCluster;
			i++;
		}
	}
}

/* Read a file from the disk */
int FAT12Read(FILE_PTR file,unsigned char *buffer,unsigned int length)
{
	/* Put the content of the file in the buffer */
	unsigned int i=0;
	int phySector=0,j=0;
	int nextCluster=0;
	unsigned char fat[512];

	if(file != NULL && buffer != NULL && length > 0){
		while(!file->eof && i < length){
			j = 32 + (file->currentCluster-1);
			if(j != phySector){
				phySector = j;
				readLBA28(file->deviceID,phySector,1,buffer+(i*512));
			}

			getFAT(file->deviceID,file->currentCluster,NULL,NULL,fat);

			nextCluster = getNextClusterFromFAT(file->deviceID,file->currentCluster,fat);
			if((nextCluster == 0) ||(nextCluster >= 0xff8)){
				file->eof = 1;
				break;
			}
			file->currentCluster = nextCluster;
			i++;
		}
		return _mi[file->deviceID].sectorSize * length;
	}
	return 0;
}

/* Locate file or directory or return the n-th file in the folder(if the name of file is not specified */
FILE FAT12Directory(int drive, char *name, FILE *folder)
{
	FILE f;
	unsigned char buffer[512],fat[512];
	char dosName[12],tmpName[12],bufName[50];
	int i=0,j,k=-1,phySector,FAT_offset,FAT_sector,nextCluster,cnt=1;
	struct fat12Entry *directory;

	if(name != NULL && (name[0] == ' ' || name[0] == 0)){
		/* Root folder */
		strcpy(f.name,"drive",8);
		f.id = 0;
		f.length = _mi[drive].numRootEntries;
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

		readLBA28(drive,phySector,1,buffer);

		directory = (struct fat12Entry*)buffer;

		for(j=0;j < (int)(_mi[drive].sectorSize / sizeof(struct fat12Entry)); j++){
			memset((unsigned char *)tmpName,0,12);
			strcpy(tmpName,directory->name,8);
			strcpy(tmpName+strlen(tmpName),directory->extension,3);
			if(!strncmp(tmpName,dosName,strlen(tmpName))){
				strcpy(f.name,name,strlen(tmpName));
				f.id = 0;
				f.eof = 0;
				f.position = directory->startingCluster;
				f.currentCluster = directory->startingCluster;
				f.deviceID = drive;
				f.indexPosition = phySector;
				if(directory->attribute == 0x10){
					f.flags = FS_DIRECTORY;
				}else{
					f.flags = FS_FILE;
					f.length = directory->fileSize;
				}
				k=strFindChar(name,'/');

				if(k > 0){
					strcpy(bufName,name,50);
					if(folder == NULL)
						f=FAT12Directory(drive,substr(bufName,k+1,0),&f);
					else
						f=FAT12Directory(drive,substr(bufName,k,strlen(name)),&f);

				}else if(f.flags == FS_DIRECTORY){
					while(1){
						getFAT(drive,f.currentCluster,&FAT_sector,&FAT_offset,fat);

						nextCluster = getNextClusterFromFAT(drive,f.currentCluster,fat);
						if((nextCluster == 0) ||(nextCluster >= 0xff8)){
							f.length=(cnt*512) / sizeof(struct fat12Entry);
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
		if(folder != NULL){
			nextCluster = getNextClusterFromFAT(drive,folder->currentCluster,fat);
			if((nextCluster == 0) ||(nextCluster >= 0xff8)){
        		break;
        	}else{
        		folder->currentCluster = nextCluster;
			}
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

	file = FAT12Directory(name[0]-48,name+2,NULL);

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

/*  Mount FAT12 file system: the mount operation consists in saving in the mount info structure the informations about the 
    file system*/
int FAT12Mount(int drive)
{
	unsigned char bootsectorBuffer[512];
	struct bpb *bootsector;

	readLBA28(drive,0,1,bootsectorBuffer);
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
