#include <kTerm.h>
#include <system.h>
#include <screen.h>
#include <string.h>
#include <keyboard.h>
#include <cpu.h>
#include <physicalMemoryManager.h>
#include <diskPIO.h>
#include <file_system.h>
#include <fat12.h>

#define NUM_FUNCTIONS 16
#define CMD_LENGTH 70
#define CURSOR_START_XPOS 3

static char command[CMD_LENGTH];
static int exit=0;

static void kTermEcho(char *str);
static void kTermClear();
static void kTermExit();
static void kTermHelp();
static void kTermLayout();
static void kTermGetCpuVendor();
static void kTermGetCpuFeatures();
static void kTermFullscreen();
static void kTermTitleScreen();
static void kTermChangeBackground();
static void kTermChangeForeground();
static void kTermGetSector(char* sector);
static void kTermReadFile(char* fileName);
static void kTermMoreFile(char *filename);
static void kTermDeleteFile(char *fileName);
static void kTermList(char *folder);

struct functionPointer{
	void (*funcPTR)();
	char *name;
};

static struct functionPointer fPtr[NUM_FUNCTIONS]={
	{ kTermGetCpuVendor		, "cpuVendor"	},
	{ kTermGetCpuFeatures	, "cpuFeatures"	},
	{ kTermFullscreen		, "fullscreen"	},
	{ kTermTitleScreen		, "halfscreen"	},
	{ kTermChangeBackground	, "background"	},
	{ kTermChangeForeground	, "foreground"	},
	{ kTermEcho				, "echo"		},
	{ kTermClear			, "clear"		},
	{ kTermExit				, "exit"		},
	{ kTermLayout			, "set_layout"	},
	{ kTermHelp				, "help"		},
	{ kTermGetSector		, "hexdump"		},
	{ kTermReadFile			, "read"		},
	{ kTermMoreFile			, "more"		},
	{ kTermDeleteFile		, "rm"			},
	{ kTermList				, "ls"			}
};

static void getCommand(char *cmd)
{
	unsigned char c=0;
	int i=0;
	memset((unsigned char*)cmd,0,CMD_LENGTH);
	while(1){
		c=getLastKeyPressed();
		switch(c){
			case 0x0:
				continue;
			case 0x8:{
				/*delete only the command characters */
				if(getXCursor() > CURSOR_START_XPOS){
					putChar(0x8);
					//resetKey();
					cmd[--i]=0;
				}
				break;
			}
			case 0x11:{
			/*
				if(getXCursor() > CURSOR_START_XPOS){
					putChar(0x11);
					i--;
				}*/
				break;
			}
			case 0x12:{
            /*
				if(getXCursor() < (strlen(cmd)+CURSOR_START_XPOS)){
					putChar(0x12);
					i++;
				}*/
				break;
			}
			case 0x13:{
            /* Mapped as up arrow: create history command */
				continue;
			}
			case 0x14:{
            /* Mapped as down arrow:create history command */
				continue;
			}
			case '\r':
			case '\n':
				return;
			default:{
				if(i<CMD_LENGTH){
					cmd[i++]=c;
					asm("cli");
					putChar(c);
					//resetKey();
					asm("sti");
				}
			}
		}
	}
}

static void kTermEcho(char *str)
{
	print("\r\n%s\r\n",str);
}

static void kTermExit()
{
	exit=1;
}

static void kTermClear()
{
	clearScreen();
}

static void kTermHelp()
{
	int i;
	print("\r\n");
	for(i=0;i<NUM_FUNCTIONS;i++){
		print("%s\t",fPtr[i].name);
	}
	print("\r\n");
}

static void kTermLayout()
{
	int key=0;
	print("\r\nPress 0 to use US layout\r\n");
	print("Press 1 to use IT layout\r\n");
	memset((unsigned char *)command,0,CMD_LENGTH);
	getCommand(command);

	if(command[0]>=48)
		key=command[0]-48;
	if(command[1]>=48){
		key*=10;
		key+=command[1]-48;
	}
	setLayout(key);
}

static void kTermFullscreen()
{
	fullScreen(1);
}

static void kTermTitleScreen()
{
	fullScreen(0);
}

static void kTermChangeBackground()
{
	int key=0;
	print("\r\n0:Black 1:Blue 2:Green 3:Cyan 4:Red 5:Magenta\r\n");
	print("6:Brown 7:Light Grey 8:Dark Grey 9:Light Blue\r\n");
	print("10:Light Green 11:Light Cyan 12:Light Red \r\n");
	print("13:Light Magenta 14:Light Brown 15:White\r\n");
	memset((unsigned char *)command,0,CMD_LENGTH);
	getCommand(command);

	if(command[0]>=48)
		key=command[0]-48;
	if(command[1]>=48){
		key*=10;
		key+=command[1]-48;
	}
	if(key>15)
		key%=16;
	setBackground(key);
	clearScreen();
}

static void kTermChangeForeground()
{
	int key=0;
	print("\r\n0:Black 1:Blue 2:Green 3:Cyan 4:Red 5:Magenta\r\n");
	print("6:Brown 7:Light Grey 8:Dark Grey 9:Light Blue\r\n");
	print("10:Light Green 11:Light Cyan 12:Light Red \r\n");
	print("13:Light Magenta 14:Light Brown 15:White\r\n");
	memset((unsigned char *)command,0,CMD_LENGTH);
	getCommand(command);

	if(command[0]>=48)
		key=command[0]-48;
	if(command[1]>=48){
		key*=10;
		key+=command[1]-48;
	}
	if(key>15)
		key%=16;
	setForeground(key);
	clearScreen();
}

static void kTermGetCpuVendor()
{
	print("\r\n");
	getCPUVendor();
}

static void kTermGetSector(char *sector)
{
	static unsigned char buffer[512];
	int i=0,sec=0,j=0;
	sector=strtok(sector,' ',0);

	for(i=0;sector[i]!=0;++i){
		sec = sec*10 + sector[i] - 48;
	}
	memset(buffer,0,512);

	readLBA28(0,sec,1,buffer);
	scrollDisable();
	print("\r\n");
	for(i=0;i<512;i++){
		if(j % 16 == 0)
			print("\r\n");
		j++;
		print("%2x ",buffer[i]);
	}
	scrollEnable();
}

static void kTermReadFile(char *filename)
{
	unsigned char *buffer;
	struct fat12Entry *dir;
	int i=0;
	FILE f;
	scrollEnable();
	f=openFile(filename,'r');
	if(f.flags == FS_FILE_INVALID){
		print("\r\nFile not found...");
		closeFile(&f);
		return;
	}
	buffer=phy_manager_alloc_blocks(f.length * 512);

	readFile(&f,buffer,f.length);
	if(f.flags == FS_FILE){
		print("\r\n%s",buffer);
	}else if(f.flags == FS_DIRECTORY){
		dir = (struct fat12Entry*)buffer;
		for(i=0;i<31;i++){
			if(dir->name[0] != 0 && dir->name[0] != '.'){
				/* file's name */
				print("\r\n%s.%s ",strtok(dir->name,' ',0),substr(dir->name,8,11));
			}
			dir++;
		}
	}
	phy_manager_dealloc_blocks(buffer,f.length *512);
	closeFile(&f);
}

void kTermMoreFile(char *filename)
{
	unsigned char buffer[15872]; 
	struct fat12Entry *dir;
	int i=0;
	FILE f;

	scrollDisable();
	f=openFile(filename,'r');
	if(f.flags == FS_FILE_INVALID){
		print("\r\nFile not found...");
		closeFile(&f);
		return;
	}

	readFile(&f,buffer,f.length);
	if(f.flags == FS_FILE){
		print("\r\n%s",buffer);
	}else if(f.flags == FS_DIRECTORY){
		dir = (struct fat12Entry*)buffer;
		for(i=0;i<31;i++){
			if(dir->name[0] != 0 && dir->name[0] != '.'){
				/* file's name */
				print("\r\n%s.%s ",strtok(dir->name,' ',0),substr(dir->name,8,11));
			}
			dir++;
		}
	}
	closeFile(&f);
	scrollEnable();
}



static void kTermGetCpuFeatures()
{
	print("\r\n");
	scrollDisable();
	getCPUFeatures();
	scrollEnable();
}

static void kTermDeleteFile(char *fileName)
{
	if(fileName != NULL)
		deleteFile(fileName);
}

static void kTermList(char *folder)
{
	if(folder != NULL)
		listFile(folder);
}

static int run(char *cmd)
{
	char *command=strtok(cmd,' ',0);
	int i=0;

	for(i=0;i<NUM_FUNCTIONS;i++){
		if(!strncmp(command,fPtr[i].name,strlen(fPtr[i].name))){
			fPtr[i].funcPTR(substr(cmd,strlen(command)+1,0));
			return 0;
		}
	}

	print("\r\nUnknown command\r\n");

	return -1;
}

void kTerm()
{
	while(!exit){
		print("\r\n>> ");
		memset((unsigned char*)command,0,CMD_LENGTH);
		getCommand(command);
		run(command);
	}
}
