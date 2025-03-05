#include <file_system.h>
#include <screen.h>
#include <string.h>
#include <system.h>

static FILESYSTEM* fs[MAX_DEVICES];

/* the filename will be with format

   N/directoryX/flename

   where N is the index in the filesystem lookup table */

static int isNumber (char c)
{
    return ((c >= '0') && (c <= '9'));
}

void openFile (char* fileName, char mode, FILE* dummy)
{
    if (isNumber (fileName[0]))
    {
        if (fileName != NULL && fs[fileName[0] - 48]->present == 1)
        {
            fs[fileName[0] - 48]->open (fileName, mode, dummy);
            return;
        }
    }
    dummy->flags = FS_FILE_INVALID;
}

void deleteFile (char* fileName)
{
    int fsIndex = 0;
    if (isNumber (fileName[0]))
    {
        fsIndex = fileName[0] - 48;
        if (fileName != NULL && fileName[1] == '/')
        {
            fs[fsIndex]->remove (fileName);
        }
    }
}

int readFile (FILE* file, unsigned char* buffer, unsigned int length)
{
    int lengthBlocks = 0;

    if (file != NULL && file->mode == 'r')
    {
        if (fs[file->deviceID]->present)
        {
            lengthBlocks = length / 512; /*TODO */
            if (length % 512 != 0)       /*TODO */
                lengthBlocks++;
            return fs[file->deviceID]->read (file, buffer, lengthBlocks);
        }
    }
    return 0;
}

int writeFile (FILE* file, unsigned char* buffer, unsigned int length)
{
    int lengthBlocks = 0;

    if (file != NULL)
    {
        if (fs[file->deviceID]->present)
        {
            lengthBlocks = length / 512; /*TODO */
            if (length % 512 != 0)       /*TODO */
                lengthBlocks++;
            return fs[file->deviceID]->write (file, buffer, lengthBlocks);
        }
    }
    return 0;
}

void listFile (char* folder, int* numElements, FILE chain[])
{
    FILE folderFile;

    if (folder == NULL)
        return;

    if (strlen (folder) > 2 && folder[strlen (folder) - 1] == '/')
        folder[strlen (folder) - 1] = 0;

    openFile (folder, 'r', &folderFile);
    if (folderFile.flags == FS_FILE_INVALID)
    {
        /*print("Cannot find %s\r\n",folder);*/
        return;
    }
    fs[folderFile.deviceID]->list (&folderFile, chain);
    *numElements = folderFile.length;
}

void closeFile (FILE* file)
{
    if (file != NULL)
    {
        if (fs[file->deviceID])
            fs[file->deviceID]->close (file);
    }
}

void registerFS (FILESYSTEM_PTR newFS, int deviceId)
{
    static int i = 0;
    if (i < MAX_DEVICES && newFS)
    {
        fs[deviceId]->present = 1;
        fs[deviceId] = newFS;
        i++;
    }
}

void unregisterFS (unsigned int deviceID)
{
    if (deviceID < MAX_DEVICES)
        fs[deviceID] = NULL;
}
