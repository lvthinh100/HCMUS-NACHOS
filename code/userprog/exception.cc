// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------
#define MaxFileLength 32
#define MaxNumLength 11

void increaseProgramCounter()
{
	int counter = machine->ReadRegister(PCReg);
   	machine->WriteRegister(PrevPCReg, counter);
    counter = machine->ReadRegister(NextPCReg);
    machine->WriteRegister(PCReg, counter);
   	machine->WriteRegister(NextPCReg, counter + 4);
}

char* User2System(int virtAddr,int limit)
{
	int i;// index
	int oneChar;
	char* kernelBuf = NULL;
	kernelBuf = new char[limit +1];//need for terminal string
	if (kernelBuf == NULL)
	return kernelBuf;
	memset(kernelBuf,0,limit+1);
	//printf("\n Filename u2s:");
	for (i = 0 ; i < limit ;i++)
	{
	machine->ReadMem(virtAddr+i,1,&oneChar);
	kernelBuf[i] = (char)oneChar;
	//printf("%c",kernelBuf[i]);
	if (oneChar == 0)
	break;
	}
	return kernelBuf;
}

/*
	Input: string to convert
	output: 0 if buffer is invalid number
			interger number itself
	
*/
int StringToNum(char* buffer) {
	int len = strlen(buffer);
	int result = 0;
	//Kiem tra chuoi co phai la so hop le khong ?

	//Case 1: Chuoi rong
	//Case 2: So thap phan sau dau '.' la mot so khac 0
	//Case 3: Chua ky tu khong phai la so
	//Case 4: Overflow handler (So qua to hoac qua nho)
	if (len == 0 || len > MaxNumLength) return 0;

	bool isNegative = 0;
	if (buffer[0] == '-') {
		isNegative = 1;
	}
	
	for (int i = isNegative; i < len; i++) {
		if (buffer[i] == '.') {
			
			for (int j = i + 1 ; j < len ; j++) {
					if ( buffer[j] != '0') return 0;
			}
			if (isNegative) return result*-1;
			return result;
		}
		if (buffer[i] < '0' || buffer[i] > '9') return 0;
		result = result * 10 + (int)(buffer[i] - 48);
	}
	

	if (isNegative) return result*-1;
	return result;
}

char* NumToString(int buffer) {
	char* result;
    int MAX_BUFFER = 255;
    result = new char[MAX_BUFFER + 1];
	if (buffer == 0) return "0";

	//Tinh chieu dai cua chuoi so
	int tempNum = buffer;
	int len = 0;
	while(tempNum != 0) {
		tempNum = tempNum / 10;
		len++;
	}

	//Xac dinh dau
	bool isNegative = 0;
	if (buffer < 0) {
		isNegative = 1;
		result[0] = '-';
		len++;
		buffer = -buffer;
	}

	//Dua phan so vao ket qua

	for (int i = len - 1; i >= isNegative;i--) {
		result[i] = (char)((buffer % 10) + 48);
        buffer /= 10;
	}
	
	return result;
}


void ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);

	// printf("Shutdown, initiated by user program.\n");
 //    if ((which == SyscallException) && (type == SC_Halt)) {
	// DEBUG('a', "Shutdown, initiated by user program.\n");
 //   	interrupt->Halt();
 //    } else {
	// printf("Unexpected user mode exception %d %d\n", which, type);
	// ASSERT(FALSE);
 //    }

    switch (which) 
    {
		case NoException:
			interrupt->setStatus(SystemMode);
            DEBUG('s', "Switch to system mode\n");
            break;
		return;
		case PageFaultException:
			DEBUG('a', "Exception Handler: Page PageFault Exception");
			interrupt->Halt();
        case ReadOnlyException:
        	DEBUG('a', "Exception Handler: Read Only Exception");
			interrupt->Halt();
        case BusErrorException:
        	DEBUG('a', "Exception Handler: BusErrorException");
			interrupt->Halt();
        case AddressErrorException:
        	DEBUG('a', "Exception Handler: AddressErrorException");
			interrupt->Halt();
        case OverflowException:
        	DEBUG('a', "Exception Handler: OverflowException");
			interrupt->Halt();
        case IllegalInstrException:
        	DEBUG('a', "Exception Handler: IllegalInstrException");
			interrupt->Halt();
        case NumExceptionTypes:
            DEBUG('a', "Error occur");
            interrupt->Halt();
            
		case SyscallException:
			switch (type)
			{
				case SC_Halt:
					DEBUG('a',"\n Shutdown, initiated by user program.");
					printf ("\n\n Shutdown, initiated by user program.");
					interrupt->Halt();
					return;
				case SC_Create:
					{
						int virtAddr;
						char* filename;
						DEBUG('a',"\n SC_Create call ...");
						DEBUG('a',"\n Reading virtual address of filename");
						// Lấy tham số tên tập tin từ thanh ghi r4
						virtAddr = machine->ReadRegister(4);
						DEBUG('a',"\n Reading filename.");
						// MaxFileLength là = 32
						filename = User2System(virtAddr,MaxFileLength+1);
						if (filename == NULL)
						{
							printf("\n Not enough memory in system");
							DEBUG('a',"\n Not enough memory in system");
							machine->WriteRegister(2,-1); // trả về lỗi cho chương
							// trình người dùng
							delete filename;
							return;
						}
						DEBUG('a',"\n Finish reading filename.");
						//DEBUG(‘a’,"\n File name : '"<<filename<<"'");
						// Create file with size = 0
						// Dùng đối tượng fileSystem của lớp OpenFile để tạo file,
						// việc tạo file này là sử dụng các thủ tục tạo file của hệ điều
						// hành Linux, chúng ta không quản ly trực tiếp các block trên
						// đĩa cứng cấp phát cho file, việc quản ly các block của file
						// trên ổ đĩa là một đồ án khác
						if (!fileSystem->Create(filename,0))
						{
							printf("\n Error create file '%s'",filename);
							machine->WriteRegister(2,-1);
							delete filename;
							return;
						}
						machine->WriteRegister(2,0); // trả về cho chương trình
						// người dùng thành công
						delete filename;
						return;
					}
				case SC_ReadInt:
					{
						char* buffer;
	                    int MAX_BUFFER = 255;
	                    buffer = new char[MAX_BUFFER + 1];
	                    gSynchConsole->Read(buffer, MAX_BUFFER);
	                    int number = StringToNum(buffer);

	                    //Tra ve ket qua vao thanh ghi
	                    machine->WriteRegister(2, number);
						increaseProgramCounter();
	                    return;
					}
				case SC_PrintInt:
					{
						int number = machine->ReadRegister(4);
						int tempNum = number;
						char *numStr = NumToString(number);
						int len = 0;
						if (number <= 0) len++;
						while(tempNum != 0) {
							tempNum = tempNum / 10;
							len++;
						}
						gSynchConsole->Write(numStr, len);
                  		//Chuyen so nguyen thanh chuoi de in
						increaseProgramCounter();
						return;
					}
				case SC_Message:
					{
						printf("Day la Mot message");
						increaseProgramCounter();
						return;
					}
				default:
					printf("\n Unexpected Systemcall: %d",type);
					interrupt->Halt();
			}
		default: 
			DEBUG('a',"Unexpected ExceptionHandler");
	}
}
