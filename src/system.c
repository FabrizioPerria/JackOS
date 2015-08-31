#include <system.h>

/* Copy a memory portion byte-by-byte */

unsigned char *memcpy(unsigned char *destination,unsigned char *source, int count)
{
	int i=0;
	if ((destination == NULL)||(source == NULL)){
		return NULL;
	}
	if(count >= 1){
		for(i=0;i<count;i++){
			destination[i] = source[i];
		}
	}
	return destination;
}

/* Copy a memory portion word-by-word (used for the VGA driver) */

unsigned short *memcpyWord(unsigned short *destination,unsigned short *source,int count){
	int i=0;
	if((destination == NULL) || (source == NULL)){
		return NULL;
	}
	if(count >=1){
		for(i=0;i<count;i++){
			destination[i]=source[i];
		}
	}
	return destination;
}

/* Initialize a memory portion with a char value */

unsigned char *memset(unsigned char *destination,unsigned char value,int count)
{
	int i=0;
	if (destination == NULL){
		return NULL;
	}
	if(count >= 1){
		for(i=0;i<count;i++){
			destination[i]=value;
		}
	}
	return destination;
}

/* Initialize a memory portion with a word value (used in the VGA driver) */
unsigned short *memsetWord(unsigned short *destination,unsigned short value,int count)
{
	int i=0;
	if(destination == NULL){
		return NULL;
	}
	if(count >= 1){
		for(i=0;i<count;i++){
			destination[i]=value;
		}
	}
	return destination;
}

/* Get a value from a device's port */
unsigned char inPortB(unsigned short port)
{
	unsigned char rv;
	__asm__ __volatile__ ("inb %1, %0" : "=a" (rv) : "d" (port));
	return rv;
}

/* Send a value to a device's port */
void outPortB(unsigned short port,unsigned char value)
{
	__asm__ __volatile__ ("outb %1, %0" : : "d" (port) ,"a" (value));
}
