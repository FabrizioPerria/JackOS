#ifndef __VARFUNC_H
#define __VARFUNC_H

typedef int* va_list;

#define va_start(argPtr, firstParam) 	(argPtr=(va_list)(&firstParam+1))
#define va_arg(argPtr,type)		(* (((type*)(argPtr=(va_list)((type*)argPtr)+1))-1))
#define va_end(argPtr)			(argPtr=(void*)0)

#endif
