#include <BPB.h>
#include <RTC.h>
#include <diskPIO.h>
#include <fat12.h>
#include <screen.h>
#include <string.h>
#include <system.h>

FILESYSTEM _fs;
struct MOUNT_INFO _mi[MAX_DEVICES];

/* Delete the spaces from a string */
static void removeSpaces (char* name)
{
    int i = strlen (name);
    for (; i >= 0; i--)
    {
        if (name[i] <= ' ')
            name[i] = 0;
    }
}

/* Return the drive given a path */
static int getDriveFromPath (char* path)
{
    if (path != NULL)
    {
        return path[0] - 48;
    }
    return -1;
}

/*Convert a file name into a 8.3 DOS file format name returned through dosName*/
static void name2DOS (char* fileName, char* dosName)
{
    int i = 0, j = 0;
    if ((fileName == NULL) || (strlen (fileName) > 11) || (dosName == NULL))
        return;

    memset ((unsigned char*) dosName, ' ', 11);
    /* 11 is the maximum length of a name is DOS (with extension)*/
    for (i = 0, j = 0; (i < strlen (fileName)); i++, j++)
    {
        if (fileName[i] == '.')
        {
            j = 7;
            continue;
        }
        dosName[j] = charToUpper (fileName[i]);
    }
    dosName[11] = 0;
}

/*get the next Cluster index of a file in the FAT
 * drive: drive which contains the file
 * index: position of the entry in the table
 */
static int getNextClusterFromFAT (int drive, int index)
{
    /* int FAT_offset = 0, next = 0; */
    /* FAT_offset = index * 1.5; */
    int FAT_offset = (index * 3) / 2; // This is equivalent to index * 1.5. FAT12 is 12bits per entry, so 3 bytes for every 2 entries.
    int next = _mi[drive].fat[FAT_offset] + (_mi[drive].fat[FAT_offset + 1] * 0x100);
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
 */
static void writeOnFAT (int drive, int index, int value)
{
    int offset = index * 1.5;
    if (index % 2)
    {
        _mi[drive].fat[offset + 1] = (value & 0xFF0) >> 4;
        _mi[drive].fat[offset] &= 0xF;
        _mi[drive].fat[offset] += (value & 0xF) << 4;
    }
    else
    {
        _mi[drive].fat[offset] = (value & 0xFF);
        _mi[drive].fat[offset + 1] &= 0xF0;
        _mi[drive].fat[offset + 1] += (value >> 8);
    }

    writeLBA28 (drive, _mi[drive].fatPosition, 9, _mi[drive].fat);
    writeLBA28 (drive, _mi[drive].fatPosition + _mi[drive].fatSize, 9, _mi[drive].fat);
}

static int findFreeCluster (int drive)
{
    int nextEntry = 0, nextCluster = 0, j = 0;

    /*The first 3 Entries at the beginning of the table are standard*/
    nextEntry = 4;

    while (nextEntry < ((_mi[drive].sectorSize * _mi[drive].fatSize * 8) / 12))
    {
        nextCluster = getNextClusterFromFAT (drive, nextEntry);
        if (nextCluster == 0)
        {
            /*Found an available entry*/
            return nextEntry;
        }
        else
        {
            if (j % 2)
                nextEntry += 2;
            else
                nextEntry++;
        }
        j++;
    }
    return -1;
}

/*Goes through the chain starting from an offset*/
static int findLastCluster (int drive, int offset)
{
    int j = 0, nextEntry = 0, nextCluster = 0;

    nextEntry = offset;

    while (nextEntry < ((_mi[drive].sectorSize * _mi[drive].fatSize * 8) / 12))
    {
        nextCluster = getNextClusterFromFAT (drive, nextEntry);
        if (nextCluster == 0xFFF)
        {
            /*Found the last entry of the chain*/
            return nextEntry;
        }
        else
        {
            if (j % 2)
                nextEntry += 2;
            else
                nextEntry++;
        }
        j++;
    }
    return -1;
}

/* Initialize and mount a file system */
void FAT12Init (int drive)
{
    if (drive < 0)
        return;
    /* Initialize and mount the FAT12 partition */
    strcpy (_fs.name, "FAT 12", 6);
    _fs.Directory = FAT12Directory;
    _fs.mount = FAT12Mount;
    _fs.read = FAT12Read;
    _fs.write = FAT12Write;
    _fs.close = FAT12Close;
    _fs.open = FAT12Open;
    _fs.remove = FAT12Remove;
    _fs.list = FAT12List;
    _fs.present = 1;
    registerFS (&_fs, 0);

    FAT12Mount (drive);
}

/*Create an empty file in the path specified */
void FAT12Create (char* fileName, FILE* folder, FILE* file)
{
    FILE list[224];
    int i, j = 1, tmp = 0, len = 0, lastCluster, freeCluster, nextEntry = 0, nextCluster = 0, pos = 0, phySector = 0, drive = -1;
    unsigned char buffer[512];
    unsigned char* time;
    char tmpName[50], name[12], bufName[50];
    struct fat12Entry* directory;

    drive = getDriveFromPath (fileName);
    if (drive < 0)
    {
        folder->flags = FS_FILE_INVALID;
        return;
    }

    /* If the file already exists, return it */
    FAT12Directory (drive, fileName + 2, NULL, file);
    if (file->flags != FS_FILE_INVALID)
        return;

    for (i = 0;; i++)
    {
        tmp = strlen (strtok (fileName, '/', i));
        len += tmp;
        if (fileName[len] == '/')
        {
            len++;
        }
        else
        {
            len -= (tmp + 1);
            if (len == 1)
            {
                /* Root folder */
                FAT12Directory (drive, " ", NULL, folder);
            }
            else if (substr (fileName, 2, len, bufName) > 0)
            {
                FAT12Directory (drive, bufName, NULL, folder);
            }
            else
            {
                folder->flags = FS_FILE_INVALID;
            }
            if (folder->flags == FS_FILE_INVALID)
            {
                folder->deviceID = drive;
                return;
            }
            break;
        }
    }
    /* folder points to the folder location */

    FAT12List (folder, list);
    int indexList = 0;

    while (strlen (list[indexList].name) != 0 && list[indexList].flags != FS_FILE_INVALID)
    {
        indexList++;
        pos++;
    }

    /* TODO: test it */
    if (pos > 0 && (pos % 16 == 0))
    {
        /*expand the folder in the fat */
        lastCluster = findLastCluster (drive, (folder->currentCluster * 3) / 2);
        freeCluster = findFreeCluster (drive);
        writeOnFAT (drive, lastCluster, freeCluster);
        writeOnFAT (drive, freeCluster, 0xFFF);
    }

    /* Find free cluster in the FAT */

    /*The first 3 Entries at the beginning of the table are standard*/
    nextEntry = 4;
    while (nextEntry < ((_mi[drive].sectorSize * _mi[drive].fatSize * 8) / 12))
    {
        nextCluster = getNextClusterFromFAT (drive, nextEntry);
        if (nextCluster == 0)
        {
            /* Found an available entry */
            writeOnFAT (drive, nextEntry, 0xFFF);
            break;
        }
        else
        {
            if (j % 2)
                nextEntry += 2;
            else
                nextEntry++;
        }
        j++;
    }
    if (nextEntry == ((_mi[drive].sectorSize * _mi[drive].fatSize * 8) / 12))
    {
        file->deviceID = drive;
        file->flags = FS_FILE_INVALID;
        return;
    }

    /* nextEntry is the position in the FAT */
    if (len > 1)
    {
        phySector = 32 + (folder->currentCluster - 1);
    }
    else
    {
        phySector = _mi[drive].rootPosition;
    }

    phySector += ((pos / sizeof (struct fat12Entry)) / _mi[drive].sectorSize);

    readLBA28 (drive, phySector, 1, buffer);

    directory = (struct fat12Entry*) buffer;

    directory += pos;

    substr (fileName, len + 1, strlen (fileName), tmpName);
    name2DOS (tmpName, name);
    strcpy (directory->name, strtok (name, '.', 0), 8);
    substr (name, strlen (directory->name), 3, bufName);
    strcpy (directory->extension, bufName, 3);
    directory->attribute = FS_FILE;
    directory->startingCluster = nextEntry;
    directory->fileSize = _mi[drive].sectorSize;

    /*TODO: test it */
    time = getTimeDate();
    /* time is an array of bytes with the folloing values:
              6        5      4    3   2      1      0
          |highYear|lowYear|month|day|hour|minute|seconds|
  */
    directory->time = ((time[2] & 0x1F) << 11) | ((time[1] & 0x3F) << 5) | (time[0] & 0x1F);
    directory->date = (((time[5] - 80) & 0x5F) << 9) | ((time[4] & 0xF) << 5) | (time[3] & 0x1F);
    /* Write the record */
    writeLBA28 (drive, phySector, 1, buffer);

    /* return the file created */
    FAT12Directory (drive, tmpName, folder, file);
}

/*Get the list of files contained in a folder */
void FAT12List (FILE* folder, FILE chain[])
{
    FILE tmp;
    int i = 0, j = 0, phySector = 0, cnt = 0, nextCluster = 0;
    unsigned char buffer[512];
    struct fat12Entry* directory;

    memset ((unsigned char*) chain, 0, 4 * sizeof (FILE));
    if (folder->flags == FS_DIRECTORY)
    {
        for (j = 0; j < (int) folder->length; j++)
        {
            if (folder->position == _mi[folder->deviceID].rootPosition)
            {
                /*Root directory*/
                i = _mi[folder->deviceID].rootPosition;
            }
            else
            {
                /* Go through directories */
                i = 32 + (folder->currentCluster - 1);
            }

            i += ((j * sizeof (struct fat12Entry)) / _mi[folder->deviceID].sectorSize);

            if (phySector != i)
            {
                phySector = i;
                readLBA28 (folder->deviceID, phySector, 1, buffer);
            }

            directory = (struct fat12Entry*) buffer;

            memset ((unsigned char*) &tmp, 0, sizeof (FILE));

            directory += (j % (_mi[folder->deviceID].sectorSize / sizeof (struct fat12Entry)));
            if (strlen (directory->name) == 0)
            {
                continue;
            }
            memset ((unsigned char*) tmp.name, 0, FILENAME_LENGTH);
            strcpy (tmp.name, directory->name, 8);
            removeSpaces (tmp.name);
            if (directory->attribute != 0x10)
            {
                tmp.name[strlen (tmp.name)] = '.';
                strcpy (tmp.name + strlen (tmp.name), directory->extension, 3);
            }
            tmp.id = 0;
            tmp.eof = 0;
            tmp.position = directory->startingCluster;
            tmp.currentCluster = directory->startingCluster;
            tmp.deviceID = folder->deviceID;
            tmp.indexPosition = phySector;
            if (directory->attribute == 0x10)
            {
                tmp.flags = FS_DIRECTORY;
                cnt = 0;
                while (1)
                {
                    nextCluster = getNextClusterFromFAT (folder->deviceID, tmp.currentCluster);
                    if ((nextCluster == 0) || (nextCluster >= 0xff8))
                    {
                        tmp.length = (cnt * 512) / sizeof (struct fat12Entry);
                        tmp.currentCluster = directory->startingCluster;
                        break;
                    }
                    else
                    {
                        cnt++;
                        tmp.currentCluster = nextCluster;
                    }
                }
            }
            else
            {
                tmp.flags = FS_FILE;
                tmp.length = directory->fileSize;
            }

            if (strlen (tmp.name) && tmp.flags != FS_FILE_INVALID)
            {
                memcpy ((unsigned char*) &chain[j], (unsigned char*) &tmp, sizeof (FILE));
            }
        }
    }
    else if (folder->flags == FS_FILE)
    {
        /* list of a file; provide only the file itself */
        memcpy ((unsigned char*) &chain[0], (unsigned char*) &folder, sizeof (*folder));
    }
}

/* Delete a file */
void FAT12Remove (char* filename)
{
    unsigned char buffer[512];
    struct fat12Entry* directory;
    char tmpName[32];
    int i = 0, nextCluster = 0, drive = -1;
    FILE file;

    if (filename == NULL)
        return;

    drive = getDriveFromPath (filename);
    if (drive < 0)
        return;

    /* Search a file using the name and set 0 on the pointed clusters */
    FAT12Directory (drive, filename + 2, NULL, &file);
    if (file.flags == FS_FILE_INVALID)
        return;
    for (i = 0; i < strlen (file.name); i++)
        file.name[i] = charToUpper (file.name[i]);

    readLBA28 (file.deviceID, file.indexPosition, 1, buffer);
    directory = (struct fat12Entry*) buffer;

    /* Scan all the directories in the sector to find the file */
    memset ((unsigned char*) tmpName, 0, 32);
    strcpy (tmpName, directory->name, 8);
    removeSpaces (tmpName);
    if (directory->attribute != 0x10)
    {
        tmpName[strlen (tmpName)] = '.';
        strcpy (tmpName + strlen (tmpName), directory->extension, 3);
    }

    while (strncmp (tmpName, file.name, strlen (file.name)))
    {
        directory++;
        memset ((unsigned char*) tmpName, 0, 32);
        strcpy (tmpName, directory->name, 8);
        removeSpaces (tmpName);
        if (directory->attribute != 0x10)
        {
            tmpName[strlen (tmpName)] = '.';
            strcpy (tmpName + strlen (tmpName), directory->extension, 3);
        }
    }
    /* Use the position declared in the directory to free the FAT entries*/
    while (1)
    {
        nextCluster = getNextClusterFromFAT (file.deviceID, file.currentCluster);

        writeOnFAT (drive, file.currentCluster, 0);

        if ((nextCluster == 0) || (nextCluster >= 0xff8))
        {
            file.currentCluster = directory->startingCluster;
            break;
        }
        else
        {
            file.currentCluster = nextCluster;
        }
    }

    memset ((unsigned char*) directory, 0, sizeof (struct fat12Entry));

    if (writeLBA28 (file.deviceID, file.indexPosition, 1, buffer) > 0)
        print ("\r\nDeleted %s\r\n", filename);
}

/* Write a file in the disk */
int FAT12Write (FILE_PTR file, unsigned char* buffer, unsigned int length)
{
    /* Put the content of the file in the buffer */
    unsigned int i = 0;
    int phySector = 0;
    int nextCluster = 0, freeCluster = 0;

    if (file != NULL && buffer != NULL && length > 0)
    {
        while (i < length)
        {
            phySector = 32 + (file->currentCluster - 1);
            writeLBA28 (file->deviceID, phySector, 1, buffer + (i * 512));

            nextCluster = getNextClusterFromFAT (file->deviceID, file->currentCluster);
            if ((i + 1 < length) && ((nextCluster == 0) || (nextCluster >= 0xff8)))
            {
                freeCluster = findFreeCluster (file->deviceID);
                writeOnFAT (file->deviceID, file->currentCluster, freeCluster);
                writeOnFAT (file->deviceID, freeCluster, 0xFFF);
                return _mi[file->deviceID].sectorSize * length;
            }
            file->currentCluster = nextCluster;
            i++;
        }
    }
    return 0;
}

/* Read a file from the disk */
int FAT12Read (FILE_PTR file, unsigned char* buffer, unsigned int length)
{
    /* Put the content of the file in the buffer */
    unsigned int i = 0;
    int phySector = 0, j = 0;
    int nextCluster = 0;

    if (file != NULL && buffer != NULL && length > 0)
    {
        while (! file->eof && i < length)
        {
            j = 32 + (file->currentCluster - 1);
            if (j != phySector)
            {
                phySector = j;
                readLBA28 (file->deviceID, phySector, 1, buffer + (i * 512));
            }

            nextCluster = getNextClusterFromFAT (file->deviceID, file->currentCluster);
            if ((nextCluster == 0) || (nextCluster >= 0xff8))
            {
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

/* Locate file or directory or return the n-th file in the folder(if the name of
 * file is not specified */
void FAT12Directory (int drive, char* name, FILE* folder, FILE* f)
{
    unsigned char buffer[512];
    char dosName[12], tmpName[12], bufName[50], bufName2[50];
    int i = 0, j, k = -1, phySector, nextCluster, cnt = 1;
    struct fat12Entry* directory;

    if (name != NULL && (name[0] == ' ' || name[0] == 0))
    {
        /* Root folder */
        strcpy (f->name, "drive", 8);
        f->id = 0;
        f->length = _mi[drive].numRootEntries;
        f->eof = 0;
        f->position = _mi[drive].rootPosition;
        f->currentCluster = _mi[drive].rootPosition;
        f->deviceID = drive;
        f->flags = FS_DIRECTORY;
        f->indexPosition = 0;
        return;
    }

    if (name != NULL && name[0] != 0)
    {
        name2DOS (strtok (name, '/', 0), dosName);
    }

    while (1)
    {
        if (folder == NULL || folder->position == _mi[drive].rootPosition)
        {
            /*Root directory*/
            /* 224 entries of 32 Bytes in the root directory with sectors of 512 bytes
      Num_sectors_root = (224*32)/512 = 14 sectors for the root directory */

            if (i < 14)
            {
                phySector = _mi[drive].rootPosition + i;
                i++;
            }
            else
            {
                f->flags = FS_FILE_INVALID;
                return;
            }
        }
        else
        {
            /* Go through directories */
            phySector = 32 + (folder->currentCluster - 1);
        }

        readLBA28 (drive, phySector, 1, buffer);

        directory = (struct fat12Entry*) buffer;

        for (j = 0; j < (int) (_mi[drive].sectorSize / sizeof (struct fat12Entry)); j++)
        {
            memset ((unsigned char*) tmpName, 0, 12);
            strcpy (tmpName, directory->name, 8);
            strcpy (tmpName + strlen (tmpName), directory->extension, 3);
            if (! strncmp (tmpName, dosName, strlen (tmpName)))
            {
                strcpy (f->name, name, strlen (tmpName));
                f->id = 0;
                f->eof = 0;
                f->position = directory->startingCluster;
                f->currentCluster = directory->startingCluster;
                f->deviceID = drive;
                f->indexPosition = phySector;
                if (directory->attribute == 0x10)
                {
                    f->flags = FS_DIRECTORY;
                }
                else
                {
                    f->flags = FS_FILE;
                    f->length = directory->fileSize;
                }
                k = strFindChar (name, '/');

                if (k > 0)
                {
                    strcpy (bufName, name, 50);
                    if (folder == NULL)
                    {
                        substr (bufName, k + 1, 0, bufName2);
                        FAT12Directory (drive, bufName2, f, f);
                    }
                    else
                    {
                        substr (bufName, k, strlen (name), bufName2);
                        FAT12Directory (drive, bufName2, f, f);
                    }
                }
                else if (f->flags == FS_DIRECTORY)
                {
                    while (1)
                    {
                        nextCluster = getNextClusterFromFAT (drive, f->currentCluster);
                        if ((nextCluster == 0) || (nextCluster >= 0xff8))
                        {
                            f->length = (cnt * 512) / sizeof (struct fat12Entry);
                            f->currentCluster = directory->startingCluster;
                            break;
                        }
                        else
                        {
                            cnt++;
                            f->currentCluster = nextCluster;
                        }
                    }
                }
                return;
            }
            directory++;
        }
        if (folder != NULL)
        {
            nextCluster = getNextClusterFromFAT (drive, folder->currentCluster);
            if ((nextCluster == 0) || (nextCluster >= 0xff8))
            {
                break;
            }
            else
            {
                folder->currentCluster = nextCluster;
            }
        }
    }
    /* Impossible to find a file */
    f->flags = FS_FILE_INVALID;
}

/* Return a reference of a file or returns a file with FS_FILE_INVALID flag set
 * if not found */
void FAT12Open (char* name, char mode, FILE_PTR file)
{
    int drive;

    if (name == NULL)
    {
        file->flags = FS_FILE_INVALID;
        return;
    }

    drive = getDriveFromPath (name);
    if (drive < 0)
    {
        file->flags = FS_FILE_INVALID;
        return;
    }

    FAT12Directory (drive, name + 2, NULL, file);

    if (file->flags != FS_FILE && file->flags != FS_DIRECTORY && mode == 'w')
    {
        FAT12Create (name, NULL, file);
    }

    if (file->flags == FS_FILE)
    {
        file->lastCluster = findLastCluster (drive, file->currentCluster);
    }
    file->mode = mode;
}

/* Invalidate a file */
void FAT12Close (FILE_PTR file)
{
    if (file)
        file->flags = FS_FILE_INVALID;
}

/*  Mount FAT12 file system: the mount operation consists in saving in the mount
   info structure the informations about the file system*/
int FAT12Mount (int drive)
{
    unsigned char bootsectorBuffer[512];
    struct bpb* bootsector;

    readLBA28 (drive, 0, 1, bootsectorBuffer);
    bootsector = (struct bpb*) bootsectorBuffer;

    _mi[drive].numSectors = bootsector->totalSectors;
    _mi[drive].sectorSize = bootsector->bytesPerSector;
    _mi[drive].fatPosition = 1;
    _mi[drive].fatSize = bootsector->sectorsPerFAT;
    _mi[drive].fatEntrySize = 8;
    _mi[drive].numRootEntries = bootsector->maxNumberEntriesRoot;
    _mi[drive].rootPosition = (bootsector->numberFATs * bootsector->sectorsPerFAT) + 1;

    _mi[drive].rootSize = (bootsector->maxNumberEntriesRoot * 32) / bootsector->bytesPerSector;

    readLBA28 (drive, _mi[drive].fatPosition, _mi[drive].fatSize, _mi[drive].fat);

    return _mi[drive].sectorSize;
}
