#include <system.h>
#include <screen.h>
#include <fat12.h>
#include <string.h>
#include <diskPIO.h>
#include <BPB.h>

FILESYSTEM _fs;
struct MOUNT_INFO _mi;

void FAT12Init(int drive)
{
	/* Initialize and mount the FAT12 partition */
	strcpy(_fs.name,"FAT 12",6);
	_fs.Directory = FAT12Directory;
	_fs.mount = FAT12Mount;
	_fs.read = FAT12Read;
	_fs.close = FAT12Close;
	_fs.open = FAT12Open;
	_fs.remove = FAT12Remove;
	_fs.list = FAT12List;

	registerFS(&_fs,0);

	FAT12Mount(drive);
}

/*FILE FAT12Create(char *fileName)
{

create an empty file

Need a search function to find the first free cluster in the FAT 
	- check the root directory
	- for each element go over the clusters until you find free cluster
	- remember: the clusters don't have to be consecutives

}
*/

FILE *FAT12List(FILE folder)
{

	/*f = FAT12Directory(folder,NULL);*/
	static FILE chain[224];
	int j = 0,phySector;
	char buffer[512];
	struct fat12Entry* directory;
	memset((unsigned char *)chain,0,224 * sizeof(FILE));
	if(folder.flags == FS_DIRECTORY){
		/* Provide an array of files contained in the folder */
		for(j = 0;j < 224;j++){
			chain[j] = FAT12Directory(folder.deviceID,NULL,j,&folder); 
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

void FAT12Remove(const char *filename)
{
	/* Search a file using the name and set 0 on the pointed clusters */
	FILE file = FAT12Directory(filename[0]-48,filename,-1,NULL);

	if(writeLBA28(0,file.position,23,(unsigned char*)"ciao sto sovrascrivendo") != -1)
		print("\r\nDeleted %s\r\n",filename);
}



void FAT12Write(FILE_PTR file,unsigned char *buffer,unsigned int length)
{
	/* Put the content of the file in the buffer */
	unsigned int i=0;
	int phySector=0;
	int FAT_offset=0,FAT_sector=0;
	int nextCluster=0;
	unsigned char *fat;

	if(file){
		while(i<length){
			phySector = 32 + (file->currentCluster-1);
			writeLBA28(file->deviceID,phySector,1,buffer+(i*512));
			while(1){
				FAT_offset = file->currentCluster * 1.5;
				FAT_sector = 1 + (FAT_offset/_mi.sectorSize);
				readLBA28(file->deviceID,FAT_sector,2,fat);
				nextCluster = fat[FAT_offset % _mi.sectorSize]+(fat[(FAT_offset%_mi.sectorSize)+1]*0x100);

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

void FAT12Read(FILE_PTR file,unsigned char *buffer,unsigned int length)
{
	/* Put the content of the file in the buffer */
	unsigned int i=0;
	int phySector=0;
	int FAT_offset=0,FAT_sector=0;
	int nextCluster=0;
	unsigned char fat[1024];

	if(file != NULL && buffer != NULL && length > 0){
		while(!file->eof){
			phySector = 32 + (file->currentCluster-1);
			readLBA28(file->deviceID,phySector,1,buffer+(i*512));
			FAT_offset = file->currentCluster * 1.5;
			FAT_sector = 1 + (FAT_offset/_mi.sectorSize);
			readLBA28(file->deviceID,FAT_sector,2,fat);
			nextCluster = fat[FAT_offset % _mi.sectorSize]+
				(fat[(FAT_offset%_mi.sectorSize)+1]*0x100);

			if(file->currentCluster %2){
				nextCluster =(nextCluster & 0xFFF0) >> 4;
			}else{
				nextCluster &=0x0FFF;
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

/*Convert a file name into a 8.3 DOS file format name*/
static void name2DOS(const char *fileName, char *dosName)
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
FILE FAT12Directory(int drive, const char *name, int n, FILE *folder)
{
	FILE f;
	unsigned char buffer[512],fat[1024];
	char dosName[12],tmpName[12],nameBuf[50];
	int i,j,k=-1,phySector,FAT_offset,FAT_sector,nextCluster,cnt=1;
	struct fat12Entry *directory;

	if(name != NULL){
		name2DOS(strtok(name,'/',0),dosName);
	}
	if(folder == NULL){
		/*Root directory*/
		/* 224 entries of 32 Bytes in the root directory with sectors of 512 bytes
	    Num_sectors_root = (224*32)/512 = 14 sectors for the root directory */
		for(i=0;i<14;i++){
			readLBA28(drive,(_mi.rootPosition +i),1,buffer);
			directory = (struct fat12Entry*)buffer;
			/* Num_root_entries_per_sector = 512 Bytes of sector / 32Bytes of entry = 16 */ 
			if(n >= 0 && name == NULL){
				directory += n;
				memset((unsigned char *)tmpName,0,12);
				strcpy(f.name,directory->name,8);
				strcpy(f.name+strlen(tmpName),directory->extension,3);
				f.id = 0;
				f.length = directory->fileSize;
				f.eof = 0;
				f.position = directory->startingCluster;
				f.currentCluster = directory->startingCluster;
				f.deviceID = drive;
				if(directory->attribute == 0x10)
					f.flags = FS_DIRECTORY;
				else
					f.flags = FS_FILE;
				return f;
			}
			for(j=0;j<16;j++){
				memset((unsigned char *)tmpName,0,12);
				strcpy(tmpName,directory->name,8);
				strcpy(tmpName+strlen(tmpName),directory->extension,3);

				if(!strncmp(tmpName,dosName,strlen(tmpName))){
					strcpy(f.name,name,strlen(name));
					f.id = 0;
					f.length = directory->fileSize;
					f.eof = 0;
					f.position = directory->startingCluster;
					f.currentCluster = directory->startingCluster;
					f.deviceID = drive;
					if(directory->attribute == 0x10)
						f.flags = FS_DIRECTORY;
					else
						f.flags = FS_FILE;

					k=strFindChar(name,'/');
					strcpy(nameBuf,name,strlen(name));
					if(k > 0){
						f=FAT12Directory(drive,substr(nameBuf,k+1,0),-1,&f);
					}else if(f.flags == FS_DIRECTORY){
						while(1){
							FAT_offset = f.currentCluster * 1.5;
							FAT_sector = 1 + (FAT_offset/_mi.sectorSize);
							readLBA28(drive,FAT_sector,2,fat);
							nextCluster = fat[FAT_offset % _mi.sectorSize]+
								(fat[(FAT_offset%_mi.sectorSize)+1]*0x100);

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
	}else{
		while(1){
			/* Go through directories */
			phySector = 32 + (folder->currentCluster-1);
			readLBA28(drive,phySector,1,buffer);

			directory = (struct fat12Entry*)buffer;
			/* Num_root_entries_per_sector = 512 Bytes of sector / 32Bytes of entry = 16 */ 
            if(n >= 0 && name == NULL){
                directory += n;
                memset((unsigned char *)tmpName,0,12);
                strcpy(f.name,directory->name,8);
                strcpy(f.name+strlen(tmpName),directory->extension,3);
                f.id = 0;
                f.length = directory->fileSize;
                f.eof = 0;
                f.position = directory->startingCluster;
                f.currentCluster = directory->startingCluster;
                f.deviceID = drive;
                if(directory->attribute == 0x10)
                    f.flags = FS_DIRECTORY;
                else
                    f.flags = FS_FILE;
                return f;
            }

			for(j=0;j<16;j++){
				memset((unsigned char *)tmpName,0,12);
				strcpy(tmpName,directory->name,8);
				strcpy(tmpName+strlen(tmpName),directory->extension,3);

				if(!strncmp(tmpName,dosName,strlen(tmpName))){
					strcpy(f.name,name,strlen(name));
					f.id = 0;
					f.length = directory->fileSize;
					f.eof = 0;
					f.position = directory->startingCluster;
					f.currentCluster = directory->startingCluster;
					f.deviceID = drive;
					if(directory->attribute == 0x10)
						f.flags = FS_DIRECTORY;
					else
						f.flags = FS_FILE;

					k=strFindChar(name,'/');
					if(k > 0){
						f=FAT12Directory(drive,substr(name,k,strlen(name)),-1,&f);
					}else if(f.flags == FS_DIRECTORY){
						while(1){
							FAT_offset = f.currentCluster * 1.5;
							FAT_sector = 1 + (FAT_offset/_mi.sectorSize);
							readLBA28(drive,FAT_sector,2,fat);
							nextCluster = fat[FAT_offset % _mi.sectorSize]+
								(fat[(FAT_offset%_mi.sectorSize)+1]*0x100);

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

			FAT_offset = folder->currentCluster * 1.5;
			FAT_sector = 1 + (FAT_offset/_mi.sectorSize);
			readLBA28(drive,FAT_sector,2,fat);
			nextCluster = fat[FAT_offset % _mi.sectorSize]+
				(fat[(FAT_offset%_mi.sectorSize)+1]*0x100);

			if(folder->currentCluster %2){
				nextCluster =(nextCluster & 0xFFF0) >> 4;
			}else{
				nextCluster &=0x0FFF;
			}
			if((nextCluster == 0) ||(nextCluster >= 0xff8)){
				f.eof = 1;
				f.flags = FS_FILE_INVALID;
				return f;
			}
			f.currentCluster = nextCluster;
		}
	}
}

/* Return a reference of a file or returns a file with FS_FILE_INVALID flag set if not found */
FILE FAT12Open(const char *name)
{
	FILE file;
	if(name == NULL){
		file.flags = FS_FILE_INVALID;
		return file;
	}
	//TODO: check the first 2 characters of name X/
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
void FAT12Mount(int drive)
{
	unsigned char bootsectorBuffer[512];
	struct bpb *bootsector;
	if(readLBA28(drive,0,1,bootsectorBuffer) < 1)
		return;
	bootsector = (struct bpb*)bootsectorBuffer;

	_mi.numSectors = bootsector->totalSectors;
	_mi.sectorSize = bootsector->bytesPerSector;
	_mi.fatPosition = 1;
	_mi.fatSize = bootsector->sectorsPerFAT;
	_mi.fatEntrySize = 8;
	_mi.numRootEntries = bootsector->maxNumberEntriesRoot;
	_mi.rootPosition = (bootsector->numberFATs * bootsector->sectorsPerFAT)+1;

	_mi.rootSize = (bootsector->maxNumberEntriesRoot *32) / bootsector->bytesPerSector;
}

