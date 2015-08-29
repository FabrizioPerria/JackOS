#ifndef __STRING_H
#define __STRING_H

int strlen(const char *str);
int strncmp(char *str1,char *str2,unsigned int length);
char *strtok(const char* str,char token,unsigned int index);
char *substr(const char* str,unsigned int start,unsigned int end);
void strcpyWord(char *destination,char *src,unsigned int length);
void strcpy(char *destination,const char *src,unsigned int length);
int strFindChar(const char *text,char target);
char charToUpper(const char text);
char charToLower(const char text);

#endif
