#include "syscall.h"
#include "copyright.h"
#define MaxFileLength 32
int main()
{
	/*Create a file*/
	Create("Text.txt");
	Halt();
	return 0;
}