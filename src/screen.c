#include <screen.h>
#include <system.h>
#include <string.h>
#include <header.h>
#include <keyboard.h>
#include <RTC.h>
#include <timer.h>

static unsigned short *screenPtr=(unsigned short *)(VGA_MEMORY);
static unsigned short *textPtr=(unsigned short *)(VGA_MEMORY + ((VGA_WIDTH)*10*2));
static unsigned int xCursor=0;
static unsigned int yCursor=0;
static unsigned char color=WHITE;
static int fullscreen=0;
static int scrollIt=1;

/* Set the background color for a character*/
void setBackground(enum vga_colors background)
{
	if(background != (color & 0xF))
		color|=background << 4;
}

/*Set the text Color of a character*/
void setForeground(enum vga_colors foreground)
{
	if(foreground !=(color >> 4))
		color|=foreground;
}

void scrollEnable()
{
	scrollIt=1;
}

void scrollDisable()
{
	scrollIt=0;
}

/* scroll down the window when you get the end.
   The hidden content is actually discarded */
void scrollDown()
{
	if(yCursor >= 25){
		if(!scrollIt){
			while(getLastKeyPressed() != ' ');
		}
		memcpyWord(textPtr,textPtr+(VGA_WIDTH),(VGA_WIDTH-1)*(VGA_HEIGHT));
		memsetWord((unsigned short*)textPtr+((VGA_WIDTH)*(VGA_HEIGHT-1)),(color<<8 | ' '),VGA_WIDTH);
		xCursor=0;
		yCursor--;
	}
}

/*Bring the cursor in the position defined by (xCursor;yCursor) */
void refreshCursor()
{
	unsigned int position=(yCursor *VGA_WIDTH)+xCursor;
	outPortB(0x3d4,0xe);
	outPortB(0x3d5,position >> 8);
	outPortB(0x3d4,0xf);
	outPortB(0x3d5,position);
}

/* Clear the entire screen putting only the background color */
void clearScreen()
{
	unsigned short blank = (color<<8)|' ';
	unsigned int i=0;

	for(i=0;i<((VGA_WIDTH)*(VGA_HEIGHT));i++){
		screenPtr[i]=blank;
	}

	xCursor=0;
	yCursor=0;
	refreshCursor();
	if(fullscreen == 0){
		putString(title);
		refreshTimer();
		xCursor=0;
		yCursor=10;
		refreshCursor();
	}
}

void refreshTimer()
{
	int tmpX=xCursor,tmpY=yCursor;
	unsigned char *timeDate;
	if(!fullscreen){
		timeDate=getTimeDate();
		asm("cli");
		moveCursor(0,9);
		print("%2x-%2x-%2x%2x %2x:%2x:%2x\r\n",
			timeDate[3],timeDate[4],timeDate[6],timeDate[5],timeDate[2],timeDate[1],timeDate[0]);
		moveCursor(tmpX,tmpY);
		asm("sti");
	}
}

int getXCursor()
{
	return xCursor;
}

int getYCursor()
{
	return yCursor;
}

void moveCursor(int x,int y)
{
	xCursor=x;
	yCursor=y;
	refreshCursor();
}

/* Write a character on the screen */
void putChar(char c)
{
	switch(c){
		case 0x0:{
			break;
		}
		case 0x8:{
			if(xCursor > 0){
				xCursor--;
			} else if(xCursor == 0 && yCursor > 0){
				yCursor--;
				xCursor=VGA_WIDTH-1;
			}
			putChar(' ');

			if(xCursor > 0){
				xCursor--;
			} else if(xCursor == 0 && yCursor > 0){
				yCursor--;
				xCursor=VGA_WIDTH-1;
			}

			break;
		}
		case 0x11:{
			/* Mapped as left arrow */
			if(xCursor > 0){
				xCursor--;
			} else if(xCursor == 0 && yCursor > 0){
				yCursor--;
				xCursor=VGA_WIDTH-1;
			}
			break;
		}
		case 0x12:{
			/* Mapped as right arrow */
			if(xCursor < VGA_WIDTH-1){
				xCursor++;
			} else if(xCursor == (VGA_WIDTH-1)){
				if(yCursor < VGA_HEIGHT){
					yCursor++;
					xCursor=0;
				}else scrollDown();
			}
			break;
		}
        case 0x13:{
			/* Mapped as up arrow */
			if(yCursor > 0){
				yCursor--;
			}
			break;
		}
		case 0x14:{
			/* Mapped as down arrow */
			if(yCursor < VGA_HEIGHT){
				yCursor++;
			} else scrollDown();
			break;
		}
		case '\r':{
			xCursor = 0;
			break;
		}
		case '\n':{
			xCursor = 0;
			yCursor++;
			break;
		}
		case '\t':{
			xCursor+=4;
			break;
		}
		default:{
			if(c>= 0x20){
				screenPtr[yCursor * VGA_WIDTH + xCursor]=(color<<8 | c);
				xCursor++;
			}
		}
	}
	if(xCursor >= VGA_WIDTH){
		xCursor-=VGA_WIDTH;
		yCursor++;
	}
	scrollDown();
	refreshCursor();
}

/* Write a string in the screen */
void putString(char *string){
	int i=0;
	int len=strlen(string);

	for(i=0;i < len;i++){
		putChar(string[i]);
	}
}

/* Convert a number base 2,10,16 (whatever not more than 16...)
   into a string */
static char *convert(unsigned int num,int base)
{
	static char buffer[33];
	char *ptr;
	if (base == 0 || base > 16) return NULL;
	ptr=&buffer[sizeof(buffer)-1];
	*ptr='\0';
	do{
		*--ptr="0123456789ABCDEF"[num%base];
		num/=base;
	}while(num != 0);

	return(ptr);
}

/* parameterized Print to the screen; capable to print 
  - decimal integers (%d)
  - hexadecimal integers (%x)
  - characters (%c)
  - strings (%s) 
*/
void print(char *str,...)
{
	char *p=0;
	int i=0,tmp=0,j=0;
	unsigned long x=0;
	int digit=-1;
	va_list arguments;
	va_start(arguments,str);

	for(p=str; *p!='\0'; p++){
		if(*p!='%'){
			putChar(*p);
			continue;
		} else {
			p++;
			if(*p >= '0' && *p <= '9'){
				digit=*p-48;
				p++;
			}
			switch(*p){
				case 'd':
					i=va_arg(arguments,int);
					if(i<0){
						putChar('-');
						i=-i;
					}
					tmp=strlen(convert(i,10));
					if(tmp < digit){
						for(j=0;j<(digit-tmp);j++)
							putChar('0');
					}
					putString(convert(i,10));
					break;
				case 'c':
					putChar(va_arg(arguments,int));
					break;
				case 'x':
					x=va_arg(arguments,long);
					tmp=strlen(convert(x,16));
					if(tmp < digit){
						for(j=0;j<(digit-tmp);j++)
							putChar('0');
					}
					putString(convert(x,16));
					break;
				case 's':
					putString(va_arg(arguments,char*));
					break;
			}
		}
	}
	va_end(arguments);
}

/* Print a number on the screen */
void printNum(int num)
{
	while(num/10){
		if(num / 10 >= 10){
			printNum(num/10);
		}
		if ((num/10)+48 <= '9')
			putChar((char)(num/10 + 48));
		num%=10;
	}
	putChar((char)(num%10+48));
}

/* Initialize the screen */
void initScreen(){
	setBackground(BLACK);
	setForeground(GREEN);
	clearScreen();
}

void fullScreen(int enable)
{
	if(enable == 0){
		fullscreen =0;
		textPtr=(unsigned short *)(VGA_MEMORY + ((VGA_WIDTH)*10*2));
	}else if(enable == 1){
		fullscreen =1;
		textPtr=screenPtr;
	}
	clearScreen();
}
