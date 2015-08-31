#ifndef __SYSTEM_H
#define __SYSTEM_H

#define NULL (void*)0

unsigned char *memcpy(unsigned char *destination,unsigned char *source, int count);
unsigned short *memcpyWord(unsigned short *destination,unsigned short *source,int count);
unsigned char *memset(unsigned char *destination,unsigned char value,int count);
unsigned short *memsetWord(unsigned short *destination,unsigned short value,int count);
unsigned char inPortB(unsigned short port);
void outPortB(unsigned short port,unsigned char value);

struct registers
{
	unsigned int gs,fs,es,ds;
	unsigned int edi,esi,ebp,esp,ebx,edx,ecx,eax;		/*pushal result*/
	unsigned int intNum;
	unsigned int eip,cs,eflags,useresp,ss;
};
#endif
