#include <RTC.h>
#include <system.h>

/*****************************************
REAL TIME CLOCK

Returns the system hour

 The format returned is:
    dd-MM-yyyy hh:mm:ss
*******************************************/

unsigned char *getTimeDate(){
	static unsigned char result[7]={0};

	__asm__ __volatile__("cli");
	outPortB(0x70,0x0);
	result[0]=inPortB(0x71);
	outPortB(0x70,0x2);
	result[1]=inPortB(0x71);
	outPortB(0x70,0x4);
	result[2]=inPortB(0x71);
	outPortB(0x70,0x7);
	result[3]=inPortB(0x71);
	outPortB(0x70,0x8);
	result[4]=inPortB(0x71);
	outPortB(0x70,0x9);
	result[5]=inPortB(0x71);
	outPortB(0x70,0x32);
	result[6]=inPortB(0x71);
	__asm__ __volatile__ ("sti");

	return result;
/*
    print("%2x-%2x-%2x%2x %2x:%2x:%2x\r\n",
           result[3],result[4],result[6],result[5],result[2],result[1],result[0]);
*/
}
