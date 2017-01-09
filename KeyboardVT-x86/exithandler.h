#pragma once

#include "stdafx.h"
#include <windef.h>

GUEST_REGS g_GuestRegs[128];

PVT_DATA g_vtData;

VOID CreateQueue(PQUEUE Q)
{
    Q->front = 0;
    Q->rear = 0;   
} 

BOOL EmptyQueue(PQUEUE Q)  
{  
    if(Q->front == Q->rear)
        return TRUE;  
    else  
        return FALSE;  
}  

BOOL FullQueue(PQUEUE Q)  
{  
    if(Q->front == (Q->rear+1)%MAX_QUEUE_SIZE)
        return TRUE;  
    else  
        return FALSE;  
}

BOOL Enqueue(PQUEUE Q, BYTE val)  
{  
    if(FullQueue(Q))  
        return FALSE;  
    else  
    {  
        Q->data[Q->rear] = val;  
        Q->rear = (Q->rear+1)%MAX_QUEUE_SIZE;  
        return TRUE;  
    }  
}  
  
BOOL Dequeue(PQUEUE Q, BYTE *val)  
{  
    if(EmptyQueue(Q))  
    {  
        return FALSE;  
    }  
    else  
    {  
        *val = Q->data[Q->front];  
        Q->front = (Q->front+1)%MAX_QUEUE_SIZE;  
        return TRUE;  
    }  
}  

ULONG g_pause_counter = 0;

void HandleIO()
{
	//获取自旋锁
	KIRQL OldIrql;
	KeAcquireSpinLock(&g_vtData->queueSpinLock, &OldIrql);
	
	ULONG uCPUID;
	uCPUID = KeGetCurrentProcessorNumber();
	
	ULONG ExitQualification;
	ExitQualification = Vmx_VmRead(EXIT_QUALIFICATION);
	
	ULONG bytes = (ExitQualification & 0x07) + 1;
	ULONG isIN = ExitQualification & 0x08;
	ULONG isSTR = ExitQualification & 0x10;
	ULONG isREP = ExitQualification & 0x20;  //从HAL.dll来看，似乎总会是REP
	ULONG isIMM = ExitQualification & 0x40;  //从HAL.dll来看，似乎不会有立即数
	USHORT uPort = (ExitQualification >> 16) & 0xFFFF;
	
	ULONG inTemp = 0;
	
	if (isSTR)
	{
		DbgPrint("Catch INS/OUTS, EAX is %08X\n", g_GuestRegs[uCPUID].eax);
	}
	else
	{
		if (isIN)
		{
			if (bytes == 1)
			{
				//IN  AL, DX
				inTemp = Asm_IOINBYTE(uPort);
				
				
				//拦截pause
				if (g_pause_counter)
				{
					g_pause_counter--;
					g_GuestRegs[uCPUID].eax = 0x0;
					goto IO_END;
				}
				if ((BYTE)inTemp==0xE1)
				{
					g_vtData->isCatchPause = 1;
					g_pause_counter = 5;
					g_GuestRegs[uCPUID].eax = 0x0;
					goto IO_END;
				}
				
				
				Enqueue(&g_vtData->monitorQueue, (BYTE)inTemp);
				
				if (g_vtData->isIntercept == 1)
				{
					g_GuestRegs[uCPUID].eax = 0x0;
					Enqueue(&g_vtData->interceptQueue, (BYTE)inTemp);
				}
				else
				{
					g_GuestRegs[uCPUID].eax = inTemp;
				}
				
				DbgPrint("Catch IN BYTE %08X, EAX is %08X\n", inTemp, g_GuestRegs[uCPUID].eax);
			}
			else if (bytes == 2)
			{
				//IN  AX, DX
				g_GuestRegs[uCPUID].eax = Asm_IOINWORD(uPort);
				DbgPrint("Catch IN WORD, EAX is %08X\n", g_GuestRegs[uCPUID].eax);
			}
			else if (bytes == 4)
			{
				//IN  EAX, DX
				g_GuestRegs[uCPUID].eax = Asm_IOINDWORD(uPort);
				DbgPrint("Catch IN DWORD, EAX is %08X\n", g_GuestRegs[uCPUID].eax);
			}
			else
			{
				DbgPrint("Catch IN OTHER, EAX is %08X\n", g_GuestRegs[uCPUID].eax);
			}
		}
		else
		{
			if (bytes == 1)
			{
				//OUT  DX, AL
				Asm_IOOUTBYTE(uPort, (BYTE)g_GuestRegs[uCPUID].eax);
				DbgPrint("Catch OUT BYTE, EAX is %08X\n", g_GuestRegs[uCPUID].eax);
			}
			else if (bytes == 2)
			{
				//OUT  DX, AX
				Asm_IOOUTWORD(uPort, (WORD)g_GuestRegs[uCPUID].eax);
				DbgPrint("Catch OUT WORD, EAX is %08X\n", g_GuestRegs[uCPUID].eax);
			}
			else if (bytes == 4)
			{
				//OUT  DX, EAX
				Asm_IOOUTDWORD(uPort, (DWORD)g_GuestRegs[uCPUID].eax);
				DbgPrint("Catch OUT DWORD, EAX is %08X\n", g_GuestRegs[uCPUID].eax);
			}
			else
			{
				DbgPrint("Catch OUT OTHER, EAX is %08X\n", g_GuestRegs[uCPUID].eax);
			}
		}
	}
	
IO_END:
	
	//释放自旋锁
	KeReleaseSpinLock(&g_vtData->queueSpinLock, OldIrql);
}

void HandleCPUID()
{
	ULONG uCPUID;
	uCPUID = KeGetCurrentProcessorNumber();
	if (g_GuestRegs[uCPUID].eax == 'Mini')
	{
		g_GuestRegs[uCPUID].ebx = 0x88888888;
		g_GuestRegs[uCPUID].ecx = 0x11111111;
		g_GuestRegs[uCPUID].edx = 0x12345678;
	}
	else Asm_CPUID(g_GuestRegs[uCPUID].eax,&g_GuestRegs[uCPUID].eax,&g_GuestRegs[uCPUID].ebx,&g_GuestRegs[uCPUID].ecx,&g_GuestRegs[uCPUID].edx);
}

void HandleInvd()
{
	Asm_Invd();
}

void HandleVmCall()
{
	ULONG JmpEIP;
	ULONG uCPUID;
	uCPUID = KeGetCurrentProcessorNumber();
	if (g_GuestRegs[uCPUID].eax == 'SVT')
	{
		JmpEIP = g_GuestRegs[uCPUID].eip + Vmx_VmRead(VM_EXIT_INSTRUCTION_LEN);
		Vmx_VmxOff();
		
		Asm_AfterVMXOff(g_GuestRegs[uCPUID].esp,JmpEIP);
	}
}

void HandleMsrRead()
{
	ULONG uCPUID;
	uCPUID = KeGetCurrentProcessorNumber();
	switch(g_GuestRegs[uCPUID].ecx)
	{
	case MSR_IA32_SYSENTER_CS:
		{
			g_GuestRegs[uCPUID].eax = Vmx_VmRead(GUEST_SYSENTER_CS);
			break;
		}
	case MSR_IA32_SYSENTER_ESP:
		{
			g_GuestRegs[uCPUID].eax = Vmx_VmRead(GUEST_SYSENTER_ESP);
			break;
		}
	case MSR_IA32_SYSENTER_EIP:	// KiFastCallEntry
		{
			g_GuestRegs[uCPUID].eax = Vmx_VmRead(GUEST_SYSENTER_EIP);
			break;
		}
	default:
		g_GuestRegs[uCPUID].eax = Asm_ReadMsr(g_GuestRegs[uCPUID].ecx);
	}

}

void HandleMsrWrite()
{
	ULONG uCPUID;
	uCPUID = KeGetCurrentProcessorNumber();
	switch(g_GuestRegs[uCPUID].ecx)
	{
	case MSR_IA32_SYSENTER_CS:
		{
			Vmx_VmWrite(GUEST_SYSENTER_CS,g_GuestRegs[uCPUID].eax);
			break;
		}
	case MSR_IA32_SYSENTER_ESP:
		{
			Vmx_VmWrite(GUEST_SYSENTER_ESP,g_GuestRegs[uCPUID].eax);
			break;
		}
	case MSR_IA32_SYSENTER_EIP:	// KiFastCallEntry
		{
			Vmx_VmWrite(GUEST_SYSENTER_EIP,g_GuestRegs[uCPUID].eax);
			break;
		}
	default:
		Asm_WriteMsr(g_GuestRegs[uCPUID].ecx,g_GuestRegs[uCPUID].eax,g_GuestRegs[uCPUID].edx);
	}
}

void HandleCrAccess()
{
	ULONG		movcrControlRegister;
	ULONG		movcrAccessType;
	ULONG		movcrOperandType;
	ULONG		movcrGeneralPurposeRegister;
	ULONG		movcrLMSWSourceData;
	ULONG		ExitQualification;
	ULONG uCPUID;
	uCPUID = KeGetCurrentProcessorNumber();

	ExitQualification = Vmx_VmRead(EXIT_QUALIFICATION) ;
	movcrControlRegister = ( ExitQualification & 0x0000000F );
	movcrAccessType = ( ( ExitQualification & 0x00000030 ) >> 4 );
	movcrOperandType = ( ( ExitQualification & 0x00000040 ) >> 6 );
	movcrGeneralPurposeRegister = ( ( ExitQualification & 0x00000F00 ) >> 8 );

	//	Control Register Access (CR3 <-- reg32)
	//
	if( movcrControlRegister == 3 && movcrAccessType == 0 && movcrOperandType == 0 && movcrGeneralPurposeRegister == 0 )
	{
		Vmx_VmWrite( GUEST_CR3, g_GuestRegs[uCPUID].eax );
	}
	if( movcrControlRegister == 3 && movcrAccessType == 0 && movcrOperandType == 0 && movcrGeneralPurposeRegister == 1 )
	{
		Vmx_VmWrite( GUEST_CR3, g_GuestRegs[uCPUID].ecx );
	}
	if( movcrControlRegister == 3 && movcrAccessType == 0 && movcrOperandType == 0 && movcrGeneralPurposeRegister == 2 )
	{
		Vmx_VmWrite( GUEST_CR3, g_GuestRegs[uCPUID].edx );
	}
	if( movcrControlRegister == 3 && movcrAccessType == 0 && movcrOperandType == 0 && movcrGeneralPurposeRegister == 3 )
	{
		Vmx_VmWrite( GUEST_CR3, g_GuestRegs[uCPUID].ebx );
	}
	if( movcrControlRegister == 3 && movcrAccessType == 0 && movcrOperandType == 0 && movcrGeneralPurposeRegister == 4 )
	{
		Vmx_VmWrite( GUEST_CR3, g_GuestRegs[uCPUID].esp );
	}
	if( movcrControlRegister == 3 && movcrAccessType == 0 && movcrOperandType == 0 && movcrGeneralPurposeRegister == 5 )
	{
		Vmx_VmWrite( GUEST_CR3, g_GuestRegs[uCPUID].ebp );
	}
	if( movcrControlRegister == 3 && movcrAccessType == 0 && movcrOperandType == 0 && movcrGeneralPurposeRegister == 6 )
	{
		Vmx_VmWrite( GUEST_CR3, g_GuestRegs[uCPUID].esi );
	}
	if( movcrControlRegister == 3 && movcrAccessType == 0 && movcrOperandType == 0 && movcrGeneralPurposeRegister == 7 )
	{
		Vmx_VmWrite( GUEST_CR3, g_GuestRegs[uCPUID].edi );
	}
	//	Control Register Access (reg32 <-- CR3)
	//
	if( movcrControlRegister == 3 && movcrAccessType == 1 && movcrOperandType == 0 && movcrGeneralPurposeRegister == 0 )
	{
		g_GuestRegs[uCPUID].eax = g_GuestRegs[uCPUID].cr3;
	}
	if( movcrControlRegister == 3 && movcrAccessType == 1 && movcrOperandType == 0 && movcrGeneralPurposeRegister == 1 )
	{
		g_GuestRegs[uCPUID].ecx = g_GuestRegs[uCPUID].cr3;
	}
	if( movcrControlRegister == 3 && movcrAccessType == 1 && movcrOperandType == 0 && movcrGeneralPurposeRegister == 2 )
	{
		g_GuestRegs[uCPUID].edx = g_GuestRegs[uCPUID].cr3;
	}
	if( movcrControlRegister == 3 && movcrAccessType == 1 && movcrOperandType == 0 && movcrGeneralPurposeRegister == 3 )
	{
		g_GuestRegs[uCPUID].ebx = g_GuestRegs[uCPUID].cr3;
	}
	if( movcrControlRegister == 3 && movcrAccessType == 1 && movcrOperandType == 0 && movcrGeneralPurposeRegister == 4 )
	{
		g_GuestRegs[uCPUID].esp = g_GuestRegs[uCPUID].cr3;
	}
	if( movcrControlRegister == 3 && movcrAccessType == 1 && movcrOperandType == 0 && movcrGeneralPurposeRegister == 5 )
	{
		g_GuestRegs[uCPUID].ebp = g_GuestRegs[uCPUID].cr3;
	}
	if( movcrControlRegister == 3 && movcrAccessType == 1 && movcrOperandType == 0 && movcrGeneralPurposeRegister == 6 )
	{
		g_GuestRegs[uCPUID].esi = g_GuestRegs[uCPUID].cr3;
	}
	if( movcrControlRegister == 3 && movcrAccessType == 1 && movcrOperandType == 0 && movcrGeneralPurposeRegister == 7 )
	{
		g_GuestRegs[uCPUID].edi = g_GuestRegs[uCPUID].cr3;
	}  
}

extern "C" ULONG GetGuestRegsAddress()
{
	ULONG uCPUID;
	uCPUID = KeGetCurrentProcessorNumber();
	return (ULONG)&g_GuestRegs[uCPUID];
}

extern "C" void VMMEntryPoint()
{
	ULONG ExitReason;
	ULONG ExitInstructionLength;
	ULONG GuestResumeEIP;
	ULONG uCPUID;
	uCPUID = KeGetCurrentProcessorNumber();

	ExitReason = Vmx_VmRead(VM_EXIT_REASON);
	ExitInstructionLength = Vmx_VmRead(VM_EXIT_INSTRUCTION_LEN);

	g_GuestRegs[uCPUID].esp = Vmx_VmRead(GUEST_RSP);
	g_GuestRegs[uCPUID].eip = Vmx_VmRead(GUEST_RIP);
	g_GuestRegs[uCPUID].cr3 = Vmx_VmRead(GUEST_CR3);

	switch(ExitReason)
	{
	case EXIT_REASON_IO_INSTRUCTION:
		{
			HandleIO();
			break;
		}
	case EXIT_REASON_CPUID:
		{
			HandleCPUID();
			break;
		}
	case EXIT_REASON_INVD:
		{
			HandleInvd();
			break;
		}
	case EXIT_REASON_VMCALL:
		{
			HandleVmCall();
			break;
		}
	case EXIT_REASON_MSR_READ:
		{
			HandleMsrRead();
			break;
		}
	case EXIT_REASON_MSR_WRITE:
		{
			HandleMsrWrite();
			break;
		}
	case EXIT_REASON_CR_ACCESS:
		{
			HandleCrAccess();
			break;
		}
	default:
		break;
	}

Resume:
	GuestResumeEIP = g_GuestRegs[uCPUID].eip+ExitInstructionLength;
	Vmx_VmWrite(GUEST_RIP,GuestResumeEIP);
	Vmx_VmWrite(GUEST_RSP,g_GuestRegs[uCPUID].esp);
}