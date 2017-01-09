#include "stdafx.h"
#include "vtsystem.h"

#include <windef.h>

#define IOCTL_CMD_READ_KEY	 		 	 CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CMD_START_INTERCEPT	 	 CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CMD_STOP_INTERCEPT		 CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CMD_GLOBAL_RUN			 CTL_CODE(FILE_DEVICE_UNKNOWN, 0x803, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CMD_GLOBAL_STOP			 CTL_CODE(FILE_DEVICE_UNKNOWN, 0x804, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CMD_READ_INTERCEPT_KEY	 CTL_CODE(FILE_DEVICE_UNKNOWN, 0x805, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CMD_CLEAN_MONITOR			 CTL_CODE(FILE_DEVICE_UNKNOWN, 0x806, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CMD_STATUS				 CTL_CODE(FILE_DEVICE_UNKNOWN, 0x807, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CMD_CATCH_PAUSE_STATUS	 CTL_CODE(FILE_DEVICE_UNKNOWN, 0x808, METHOD_BUFFERED, FILE_ANY_ACCESS)


extern PVT_DATA g_vtData;
VOID CreateQueue(PQUEUE Q);
BOOL Dequeue(PQUEUE Q, BYTE *val);

void MiniVTUnload(IN PDRIVER_OBJECT DriverObject);
NTSTATUS MiniVTCreateClose(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS MiniVTDefaultHandler(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS KBDispatchIoctl(PDEVICE_OBJECT pDevObj, PIRP pIrp);

#ifdef __cplusplus
extern "C" NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING  RegistryPath);
#endif

NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING  RegistryPath)
{
	UNICODE_STRING DeviceName,Win32Device;
	PDEVICE_OBJECT DeviceObject = NULL;
	NTSTATUS status;
	unsigned i;

	RtlInitUnicodeString(&DeviceName,L"\\Device\\KeyboardVT");
	RtlInitUnicodeString(&Win32Device,L"\\DosDevices\\KeyboardVT");

	for (i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++)
		DriverObject->MajorFunction[i] = MiniVTDefaultHandler;

	DriverObject->MajorFunction[IRP_MJ_CREATE] = MiniVTCreateClose;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = MiniVTCreateClose;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = KBDispatchIoctl;
	
	DriverObject->DriverUnload = MiniVTUnload;
	status = IoCreateDevice(DriverObject,
							sizeof(VT_DATA),
							&DeviceName,
							FILE_DEVICE_UNKNOWN,
							0,
							FALSE,
							&DeviceObject);
	if (!NT_SUCCESS(status))
		return status;
	if (!DeviceObject)
		return STATUS_UNEXPECTED_IO_ERROR;
	
	g_vtData = (PVT_DATA)(DeviceObject->DeviceExtension);
	CreateQueue(&g_vtData->monitorQueue);
	CreateQueue(&g_vtData->interceptQueue);
	g_vtData->isRunning = 0;
	g_vtData->isIntercept = 0;
	g_vtData->isCatchPause = 0;
	KeInitializeSpinLock(&g_vtData->queueSpinLock);

	DeviceObject->Flags |= DO_DIRECT_IO;
	DeviceObject->AlignmentRequirement = FILE_WORD_ALIGNMENT;
	status = IoCreateSymbolicLink(&Win32Device, &DeviceName);

	DeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

	StartVirtualTechnology();
	Log("驱动加载完毕.",0);
	return STATUS_SUCCESS;
}

void MiniVTUnload(IN PDRIVER_OBJECT DriverObject)
{
	UNICODE_STRING Win32Device;
	StopVirtualTechnology();
	RtlInitUnicodeString(&Win32Device,L"\\DosDevices\\KeyboardVT");
	IoDeleteSymbolicLink(&Win32Device);
	IoDeleteDevice(DriverObject->DeviceObject);
	Log("驱动卸载完毕.",0);
}

NTSTATUS MiniVTCreateClose(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS MiniVTDefaultHandler(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	Irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return Irp->IoStatus.Status;
}

NTSTATUS KBDispatchIoctl(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
	NTSTATUS status = STATUS_INVALID_DEVICE_REQUEST;
	PIO_STACK_LOCATION pIrpStack;
	ULONG uIoControlCode;
	PVOID pIoBuffer;
	ULONG uInSize;
	ULONG uOutSize;
	
	PVT_DATA pVtData = (PVT_DATA)(pDevObj->DeviceExtension);
	BYTE temp;
	
	pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
	
	uIoControlCode = pIrpStack->Parameters.DeviceIoControl.IoControlCode;
	
	pIoBuffer = pIrp->AssociatedIrp.SystemBuffer;						
	uInSize = pIrpStack->Parameters.DeviceIoControl.InputBufferLength;	
	uOutSize = pIrpStack->Parameters.DeviceIoControl.OutputBufferLength;
	
	//获取自旋锁
	KIRQL OldIrql;
	KeAcquireSpinLock(&pVtData->queueSpinLock, &OldIrql);
	
	switch(uIoControlCode)
	{
		//在这里加入接口
		
		case IOCTL_CMD_GLOBAL_RUN:
		{
			pVtData->isRunning = 1;
			pVtData->isIntercept = 0;
			pVtData->isCatchPause = 0;
			CreateQueue(&pVtData->monitorQueue);
			CreateQueue(&pVtData->interceptQueue);
			status = STATUS_SUCCESS;
			break;
		}
		
		case IOCTL_CMD_GLOBAL_STOP:
		{
			pVtData->isRunning = 0;
			pVtData->isIntercept = 0;
			pVtData->isCatchPause = 0;
			CreateQueue(&pVtData->monitorQueue);
			CreateQueue(&pVtData->interceptQueue);
			status = STATUS_SUCCESS;
			break;
		}
		
		case IOCTL_CMD_START_INTERCEPT:
		{
			if (!pVtData->isRunning)
			{
				status = STATUS_SUCCESS;
				break;
			}
			pVtData->isIntercept = 1;
			CreateQueue(&pVtData->interceptQueue);
			status = STATUS_SUCCESS;
			break;
		}
		
		case IOCTL_CMD_STOP_INTERCEPT:
		{
			if (!pVtData->isRunning)
			{
				status = STATUS_SUCCESS;
				break;
			}
			pVtData->isIntercept = 0;
			CreateQueue(&pVtData->interceptQueue);
			status = STATUS_SUCCESS;
			break;
		}

		case IOCTL_CMD_READ_KEY:
		{
			if (Dequeue(&pVtData->monitorQueue, &temp))
			{
				*(BYTE*)pIoBuffer = temp;
			}
			else
			{
				*(BYTE*)pIoBuffer = 0x00;
			}
			
			status = STATUS_SUCCESS;
			break;
		}
		
		case IOCTL_CMD_READ_INTERCEPT_KEY:
		{
			if (!pVtData->isRunning || !pVtData->isIntercept)
			{
				*(BYTE*)pIoBuffer = 0x00;
				status = STATUS_SUCCESS;
				break;
			}
			if (Dequeue(&pVtData->interceptQueue, &temp))
			{
				*(BYTE*)pIoBuffer = temp;
			}
			else
			{
				*(BYTE*)pIoBuffer = 0x00;
			}
			
			status = STATUS_SUCCESS;
			break;
		}
		
		case IOCTL_CMD_CLEAN_MONITOR:
		{
			CreateQueue(&pVtData->monitorQueue);
			status = STATUS_SUCCESS;
			break;
		}
		
		case IOCTL_CMD_STATUS:
		{
			if (pVtData->isRunning && pVtData->isIntercept)
			{
				*(BYTE*)pIoBuffer = 0x00;
				status = STATUS_SUCCESS;
			}
			else if (pVtData->isRunning && !pVtData->isIntercept)
			{
				*(BYTE*)pIoBuffer = 0x01;
				status = STATUS_SUCCESS;
			}
			else if (!pVtData->isRunning && pVtData->isIntercept)
			{
				*(BYTE*)pIoBuffer = 0x02;
				status = STATUS_SUCCESS;
			}
			else
			{
				*(BYTE*)pIoBuffer = 0x03;
				status = STATUS_SUCCESS;
			}

			break;
		}
		
		case IOCTL_CMD_CATCH_PAUSE_STATUS:
		{
			if (pVtData->isCatchPause == 1)
			{
				*(BYTE*)pIoBuffer = 0x01;
			}
			else
			{
				*(BYTE*)pIoBuffer = 0x00;
			}
			
			pVtData->isCatchPause = 0;
			status = STATUS_SUCCESS;
			break;
		}

	}
	
	//释放自旋锁
	KeReleaseSpinLock(&pVtData->queueSpinLock, OldIrql);
	
	//这里设定DeviceIoControl的*lpBytesReturned的值（如果通信失败则返回0长度）
	if(status == STATUS_SUCCESS)
		pIrp->IoStatus.Information = sizeof(BYTE);	
	else
		pIrp->IoStatus.Information = 0;	
	//这里设定DeviceIoControl的返回值是成功还是失败
	pIrp->IoStatus.Status = status;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return status;
}

