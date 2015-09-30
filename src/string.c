#include <string.h>
#include <system.h>
#include <screen.h>

/* Return the length of a NULL-terminated string */
int strlen(const char *text)
{
	int counter=0;
	if(text == NULL){
		return -1;
	}

	for(counter=0; text[counter] != '\0'; counter++);

	return counter;
}

char *strtok(const char* str,char token,unsigned int index){
	static char res[50];
	int i=0,j=0;
	unsigned cnt=0;
	if(str == NULL)
		return NULL;
	memset((unsigned char*)res,0,50);
	for(i=0;i<strlen(str);i++){
		if(str[i] == token){
			if(cnt++==index){
				break;
			} else {
				for(j=0;j<strlen(str);j++)
					res[j]='\0';
				j=0;
			}
		} else {
			res[j++]=str[i];
		}
	}
	return res;
}

char *substr(const char* str,unsigned int start,unsigned int end){
	static char res[50];
	unsigned int i=0;

	if(str == NULL || strlen(str) < (int)start)
		return NULL;
	if(end < start)
		end=strlen(str);
	memset((unsigned char*)res,0,50);

	for(i=start;(i<end) && ((i-start)< 50) && (i < (unsigned int)(strlen(str)));i++){
		res[i-start] = str[i];
		if(str[i]=='\0'){
			return res;
		}
	}

	return res;
}

int strncmp(char *str1,char *str2,unsigned int length)
{
	int i=0;

	if(str1 == NULL || str2 == NULL || length < 1)
		return 1;
	for(i=0;(i<=(int)length);i++){
		if(str1[i] != str2[i])
			return 1;
	}

	return 0;
}

void strcpyWord(char *destination,char *src,unsigned int length)
{
	unsigned int i=0;
	if(destination ==NULL || src == NULL){
		print("STRCPY: error");
		return;
	}
	for(i=0;i<length*2;i+=2){
		destination[i/2]=src[i];
	}
}

void strcpy(char *destination,const char *src,unsigned int length)
{
	unsigned int i=0;
	if(destination ==NULL || src == NULL){
		print("STRCPY: error");
		return;
	}
	for(i=0;i<length;i++){
		destination[i]=src[i];
	}
}

/* Find the first occurrence of a character on the text
   and return the index of the character */
int strFindChar(const char *text,char target)
{
	int i=0,length = strlen(text);
	if(text == NULL)
		return -1;
	for(i = 0; i < length; i++){
		if(text[i] == target)
			return i;
	}
	return -1;
}

char charToUpper(const char text)
{
	if(text >= 'a' && text <= 'z')
		return (text - 32);					/*To Upper case */
	else
		return text;
}

char charToLower(const char text)
{
	if(text >= 'A' && text <= 'Z')
		return (text+32);                    /*To Lower case */
	else
	return text;
}
