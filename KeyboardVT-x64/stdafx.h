#pragma once

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#ifdef __cplusplus
extern "C" 
{

#endif

#include <ntddk.h>
#include <ntddstor.h>
#include <mountdev.h>
#include <ntddvol.h>


#ifdef __cplusplus
}
#endif

#define Log(message,value) {{KdPrint(("[MiniVT] %-40s [%p]\n",message,value));}}

#pragma warning(disable:4244 4102 4104)

#define MAX_QUEUE_SIZE 500

typedef struct queue
{  
    UCHAR data[MAX_QUEUE_SIZE];  
    ULONG front;    //指向队列第一个元素
    ULONG rear;     //指向队列最后一个元素的下一个元素
} QUEUE, *PQUEUE;

typedef struct vtdata
{  
    QUEUE monitorQueue;
	QUEUE interceptQueue;
	ULONG isRunning;          //全局开关
	ULONG isIntercept;        //0为不拦截，1为拦截
	ULONG isCatchPause;       //pause捕获，0为未捕获到，1为捕获到
	KSPIN_LOCK queueSpinLock; //自旋锁
} VT_DATA, *PVT_DATA;



