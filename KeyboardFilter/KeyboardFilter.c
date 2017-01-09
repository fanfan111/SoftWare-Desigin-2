#include <ntddk.h>
#include <wdm.h>
#include <ntstrsafe.h>

ULONG gC2pKeyCount = 0;
PDRIVER_OBJECT gDriverObject;

//实际上是一个全局变量，但是头文件中没有
extern POBJECT_TYPE *IoDriverObjectType;

//KbdClass驱动的名字
#define KBD_DRIVER_NAME L"\\Driver\\kbdclass"
#define DELAY_ONE_MICROSECOND   (-10)
#define DELAY_ONE_MILLISECOND   (DELAY_ONE_MICROSECOND*1000)


//-----------------------------------------------------------------------------------------------------------------

//定义符号链接
#define	DEVICE_NAME			L"\\Device\\KeyboardFilter"
#define LINK_NAME			L"\\DosDevices\\KeyboardFilter"
#define LINK_GLOBAL_NAME	L"\\DosDevices\\Global\\KeyboardFilter"

//全局控制设备对象
#pragma LOCKEDDATA
PDEVICE_OBJECT g_devControl = NULL;

//保存的默认派遣函数指针
NTSTATUS (*IopInvalidDeviceRequest)(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp) = NULL;

#include <windef.h>

#define IOCTL_CMD_READ_KEY	 		 	 CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CMD_CLEAN_MONITOR			 CTL_CODE(FILE_DEVICE_UNKNOWN, 0x806, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define MAX_QUEUE_SIZE 500

typedef struct queue
{  
    UCHAR data[MAX_QUEUE_SIZE];  
    ULONG front;    //指向队列第一个元素
    ULONG rear;     //指向队列最后一个元素的下一个元素
} QUEUE, *PQUEUE;

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

//-----------------------------------------------------------------------------------------------------------------



//此函数事实存在
//声明一下就可使用
NTSTATUS 
ObReferenceObjectByName(
	PUNICODE_STRING ObjectName,
	ULONG Attributes,
	PACCESS_STATE AccessState,
	ACCESS_MASK DesiredAccess,
	POBJECT_TYPE ObjectType,
	KPROCESSOR_MODE AccessMode,
	PVOID ParseContext,
	PVOID *Object
	);
	

//设备扩展
typedef struct _C2P_DEV_EXT
{
	//结构的大小
	ULONG NodeSize;
	//过滤设备对象
	PDEVICE_OBJECT pFilterDeviceObject;
	//同时调用时的保护锁
	KSPIN_LOCK IoRequestSpinLock;
	//进程间同步处理
	KEVENT IoInProgressEvent;
	//绑定的设备对象
	PDEVICE_OBJECT TargetDeviceObject;
	//绑定前底层设备对象
	PDEVICE_OBJECT LowerDeviceObject;
	
	//为通信增加
	PQUEUE pMonitorQueue;
	PKSPIN_LOCK pQueueSpinLock; //自旋锁
	
} C2P_DEV_EXT, *PC2P_DEV_EXT;

NTSTATUS
c2pDevExtInit(
	IN PC2P_DEV_EXT devExt,
	IN PDEVICE_OBJECT pFilterDeviceObject,
	IN PDEVICE_OBJECT pTargetDeviceObject,
	IN PDEVICE_OBJECT pLowerDeviceObject
	)
{
	memset(devExt, 0, sizeof(C2P_DEV_EXT));
	devExt->NodeSize = sizeof(C2P_DEV_EXT);
	devExt->pFilterDeviceObject = pFilterDeviceObject;
	KeInitializeSpinLock(&(devExt->IoRequestSpinLock));
	KeInitializeEvent(&(devExt->IoInProgressEvent), NotificationEvent, FALSE);
	devExt->TargetDeviceObject = pTargetDeviceObject;
	devExt->LowerDeviceObject = pLowerDeviceObject;
	return STATUS_SUCCESS;
}


//这个函数经过改造，能打开驱动对象KbdClass，然后
//绑定它下面的所有设备
NTSTATUS
c2pAttachDevices(
	IN PDRIVER_OBJECT DriverObject,
	IN PUNICODE_STRING RegistryPath,
	IN PQUEUE pMonitorQueue,
	IN PKSPIN_LOCK pQueueSpinLock
	)
{
	NTSTATUS status = 0;
	UNICODE_STRING uniNtNameString;
	PC2P_DEV_EXT devExt;
	PDEVICE_OBJECT pFilterDeviceObject = NULL;
	PDEVICE_OBJECT pTargetDeviceObject = NULL;
	PDEVICE_OBJECT pLowerDeviceObject = NULL;

	PDRIVER_OBJECT KbdDriverObject = NULL;

	DbgPrint("[ATTACH] MyAttach\n");

	//初始化一个字符串，KbdClass驱动的名字
	RtlInitUnicodeString(&uniNtNameString, KBD_DRIVER_NAME);

	//打开驱动对象
	status = ObReferenceObjectByName(
		&uniNtNameString,
		OBJ_CASE_INSENSITIVE,
		NULL,
		0,
		*IoDriverObjectType,
		KernelMode,
		NULL,
		(PVOID*)&KbdDriverObject
		);

	if (!NT_SUCCESS(status))
	{
		DbgPrint("[ATTACH] fail to get Driver Object\n");
		return (status);
	}
	else
	{
		//调用ObReferenceObjectByName会导致驱动对象的引用计数增加
		//必须相应的调用解引用ObDereferenceObject
		ObDereferenceObject(DriverObject);
	}

	//这是设备链中的第一个设备
	pTargetDeviceObject = KbdDriverObject->DeviceObject;
	while (pTargetDeviceObject)
	{
		//生成一个过滤设备
		status = IoCreateDevice(
			IN DriverObject,
			IN sizeof(C2P_DEV_EXT),
			IN NULL,
			IN pTargetDeviceObject->DeviceType,
			IN pTargetDeviceObject->Characteristics,
			IN FALSE,
			OUT &pFilterDeviceObject
			);
		if (!NT_SUCCESS(status))
		{
			DbgPrint("[ATTACH] fail to create filter device object");
			return (status);
		}

		//绑定。pLowerDeviceObject是绑定之后得到的下一个设备
		//也就是真实设备
		pLowerDeviceObject =
			IoAttachDeviceToDeviceStack(pFilterDeviceObject,
				pTargetDeviceObject);
		//如果绑定失败，放弃之前的操作，退出
		if (!pLowerDeviceObject)
		{
			DbgPrint("[ATTACH] fail to attach to Device Object");
			IoDeleteDevice(pFilterDeviceObject);
			pFilterDeviceObject = NULL;
			return (status);
		}

		
		//设备扩展
		devExt = (PC2P_DEV_EXT)(pFilterDeviceObject->DeviceExtension);
		c2pDevExtInit(
			devExt,
			pFilterDeviceObject,
			pTargetDeviceObject,
			pLowerDeviceObject
			);
		devExt->pMonitorQueue = pMonitorQueue;
		devExt->pQueueSpinLock = pQueueSpinLock;
		

		pFilterDeviceObject->DeviceType =
			pLowerDeviceObject->DeviceType;
		pFilterDeviceObject->Characteristics =
			pLowerDeviceObject->Characteristics;
		pFilterDeviceObject->StackSize =
			pLowerDeviceObject->StackSize;
		pFilterDeviceObject->Flags |=
			pLowerDeviceObject->Flags &
			(DO_BUFFERED_IO | DO_DIRECT_IO | DO_POWER_PAGABLE);

		// 移动到下一个设备，继续遍历
		pTargetDeviceObject = pTargetDeviceObject->NextDevice;
	}
	return status;
}


VOID c2pUnload(PDRIVER_OBJECT DriverObject)
{
	UNICODE_STRING Win32Device;
	
	PDEVICE_OBJECT DeviceObject;
	PDEVICE_OBJECT OldDeviceObject;
	PC2P_DEV_EXT devExt;

	LARGE_INTEGER	lDelay;
	PRKTHREAD CurrentThread;
	//delay some time
	lDelay = RtlConvertLongToLargeInteger(100 * DELAY_ONE_MILLISECOND);
	CurrentThread = KeGetCurrentThread();
	//把当前线程设置为低实时模式，以便让它的运行尽量少影响其他程序
	KeSetPriorityThread(CurrentThread, LOW_REALTIME_PRIORITY);

	UNREFERENCED_PARAMETER(DriverObject);
	DbgPrint("[Unload] Driver unLoading...\n");
	
	
	//卸载控制设备
	RtlInitUnicodeString(&Win32Device, LINK_NAME);
	IoDeleteSymbolicLink(&Win32Device);
	IoDeleteDevice(g_devControl);
	DbgPrint("[Unload] KeyboardFilter unLoad OK!\n");

	
	//遍历所有设备并一律解除绑定
	DeviceObject = DriverObject->DeviceObject;
	while (DeviceObject)
	{
		devExt = (PC2P_DEV_EXT)DeviceObject->DeviceExtension;
		IoDetachDevice(devExt->LowerDeviceObject);
		DeviceObject = DeviceObject->NextDevice;
	}
	IoDeleteDevice(DriverObject->DeviceObject);

	while (gC2pKeyCount)
	{
		KeDelayExecutionThread(KernelMode, FALSE, &lDelay);
	}
	DbgPrint("[Unload] Driver unLoad OK!\n");
	return;
}

NTSTATUS c2pDispatchGeneral(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP Irp
	)
{
	//控制设备分发至默认派遣函数
	if (DeviceObject == g_devControl)
	{
		return IopInvalidDeviceRequest(DeviceObject, Irp);
	}
	
	//其他的分发函数，直接skip，然后用IoCallDriver
	//把irp发送到真实设备的设备对象
	DbgPrint(("[GeneralDispatch] Other Dispatch"));
	IoSkipCurrentIrpStackLocation(Irp);
	return IoCallDriver(((PC2P_DEV_EXT)
		DeviceObject->DeviceExtension)->LowerDeviceObject, Irp);
}

NTSTATUS c2pPower(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP Irp
	)
{
	PC2P_DEV_EXT devExt;
	devExt =
		(PC2P_DEV_EXT)DeviceObject->DeviceExtension;

	PoStartNextPowerIrp(Irp);
	IoSkipCurrentIrpStackLocation(Irp);
	return PoCallDriver(devExt->LowerDeviceObject, Irp);
}

NTSTATUS c2pPnP(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP Irp
	)
{
	PC2P_DEV_EXT devExt;
	PIO_STACK_LOCATION irpStack;
	NTSTATUS status = STATUS_SUCCESS;
	KIRQL oldIrql;
	KEVENT event;

	//获得真实设备
	devExt = (PC2P_DEV_EXT)(DeviceObject->DeviceExtension);
	irpStack = IoGetCurrentIrpStackLocation(Irp);

	switch (irpStack->MinorFunction)
	{
	case IRP_MN_REMOVE_DEVICE:
		DbgPrint(("IRP_MN_REMOVE_DEVICE\n"));

		//首先把请求发下去jie'chu
		IoSkipCurrentIrpStackLocation(Irp);
		IoCallDriver(devExt->LowerDeviceObject, Irp);
		//然后解除绑定
		IoDetachDevice(devExt->LowerDeviceObject);
		status = STATUS_SUCCESS;
		break;
	default:
		//对于其他类型的IRP，全部直接下发即可
		IoSkipCurrentIrpStackLocation(Irp);
		status = IoCallDriver(devExt->LowerDeviceObject, Irp);
	}
	return status;
}

NTSTATUS c2pReadComplete(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP Irp,
	IN PVOID Context
	)
{
	KIRQL OldIrql;
	PC2P_DEV_EXT devExt;
	
	PIO_STACK_LOCATION IrpSp;
	ULONG buf_len = 0;
	PUCHAR buf = NULL;
	size_t i;

	IrpSp = IoGetCurrentIrpStackLocation(Irp);

	//假设这个请求是成功的，如果请求失败，获取
	//进一步的信息是无意义的
	if (NT_SUCCESS(Irp->IoStatus.Status))
	{
		//获得都请求完成后的输出缓冲区
		buf = Irp->AssociatedIrp.SystemBuffer;
		//获得缓冲区的长度，
		//一般来说，不管返回值有多长都保存在Infomation中
		buf_len = (ULONG)Irp->IoStatus.Information;
		
		
		devExt = (PC2P_DEV_EXT)(DeviceObject->DeviceExtension);
		
		//获取自旋锁
		KeAcquireSpinLock(devExt->pQueueSpinLock, &OldIrql);
		
		if (buf_len >= 5)
		{
			if (buf[4]==0)        //普通
			{
				Enqueue(devExt->pMonitorQueue, buf[2]);
			}
			else if (buf[4]==1)
			{
				Enqueue(devExt->pMonitorQueue, buf[2]+0x80);
			}
			else if (buf[4]==2)   //E0
			{
				Enqueue(devExt->pMonitorQueue, 0xE0);
				Enqueue(devExt->pMonitorQueue, buf[2]);
			}
			else if (buf[4]==3)
			{
				Enqueue(devExt->pMonitorQueue, 0xE0);
				Enqueue(devExt->pMonitorQueue, buf[2]+0x80);
			}
			else if (buf[4]==4)   //E1
			{
				Enqueue(devExt->pMonitorQueue, 0xE1);
				Enqueue(devExt->pMonitorQueue, buf[2]);
			}
			else if (buf[4]==5)
			{
				Enqueue(devExt->pMonitorQueue, 0xE1);
				Enqueue(devExt->pMonitorQueue, buf[2]+0x80);
			}
		}
		
		//释放自旋锁
		KeReleaseSpinLock(devExt->pQueueSpinLock, OldIrql);
		
		//打印显示
		DbgPrint("[READ COMPLETE] buf_len: %d -------------------------------", buf_len);
		for (i=0; i<buf_len; ++i)
		{
			DbgPrint("ctrl2cap[i = %d]: 0x%02x\r\n", i, buf[i]);
		}
		
	}
	--gC2pKeyCount;
	if (Irp->PendingReturned)
	{
		IoMarkIrpPending(Irp);
	}
	return Irp->IoStatus.Status;
}

NTSTATUS c2pDispatchRead(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP Irp
	)
{
	NTSTATUS status = STATUS_SUCCESS;
	PC2P_DEV_EXT devExt;
	PIO_STACK_LOCATION currentIrpStack;
	
	KEVENT waitEvent;
	KeInitializeEvent(&waitEvent, NotificationEvent, FALSE);

	if (Irp->CurrentLocation == 1)
	{
		ULONG ReturnedInfomation = 0;
		DbgPrint(("[DispatchRead] Dispatch encountered bogus current location\n"));
		status = STATUS_INVALID_DEVICE_REQUEST;
		Irp->IoStatus.Status = status;
		Irp->IoStatus.Information = ReturnedInfomation;
		IoCompleteRequest(Irp, IO_NO_INCREMENT);
		return(status);
	}

	//全局变量计数器加1
	++gC2pKeyCount;

	//得到设备扩展，目的是为了获得下一个设备的指针
	devExt =
		(PC2P_DEV_EXT)DeviceObject->DeviceExtension;

	//设置回掉函数并把IRP传递下去，读处理就结束了
	currentIrpStack = IoGetCurrentIrpStackLocation(Irp);
	IoCopyCurrentIrpStackLocationToNext(Irp);
	IoSetCompletionRoutine(Irp, c2pReadComplete,
		DeviceObject, TRUE, TRUE, TRUE);
	return IoCallDriver(devExt->LowerDeviceObject, Irp);
}

NTSTATUS DispatchCreate(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
	//非控制设备单独分发
	if (pDevObj != g_devControl)
	{
		return c2pDispatchGeneral(pDevObj, pIrp);
	}
	
	DbgPrint("[KeyboardFilter]DispatchCreate\n");
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS DispatchClose(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
	//非控制设备单独分发
	if (pDevObj != g_devControl)
	{
		return c2pDispatchGeneral(pDevObj, pIrp);
	}
	
	DbgPrint("[KeyboardFilter]DispatchClose\n");
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS DispatchIoctl(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
	NTSTATUS status = STATUS_INVALID_DEVICE_REQUEST;
	PIO_STACK_LOCATION pIrpStack;
	ULONG uIoControlCode;
	PVOID pIoBuffer;
	ULONG uInSize;
	ULONG uOutSize;

	KIRQL OldIrql;
	PC2P_DEV_EXT devExt;
	BYTE temp;
	
	//非控制设备单独分发
	if (pDevObj != g_devControl)
	{
		return c2pDispatchGeneral(pDevObj, pIrp);
	}
	
	devExt = (PC2P_DEV_EXT)(pDevObj->DeviceExtension);
	
	pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
	uIoControlCode = pIrpStack->Parameters.DeviceIoControl.IoControlCode;
	pIoBuffer = pIrp->AssociatedIrp.SystemBuffer;						
	uInSize = pIrpStack->Parameters.DeviceIoControl.InputBufferLength;	
	uOutSize = pIrpStack->Parameters.DeviceIoControl.OutputBufferLength;
	
	//获取自旋锁
	KeAcquireSpinLock(devExt->pQueueSpinLock, &OldIrql);
	
	switch(uIoControlCode)
	{
		case IOCTL_CMD_READ_KEY:
		{
			if (Dequeue(devExt->pMonitorQueue, &temp))
			{
				*(BYTE*)pIoBuffer = temp;
			}
			else
			{
				*(BYTE*)pIoBuffer = 0x00;
			}
			
			pIrp->IoStatus.Information = sizeof(BYTE);
			status = STATUS_SUCCESS;
			break;
		}
		case IOCTL_CMD_CLEAN_MONITOR:
		{
			CreateQueue(devExt->pMonitorQueue);
			
			pIrp->IoStatus.Information = sizeof(BYTE);
			status = STATUS_SUCCESS;
			break;
		}
	}
	
	//释放自旋锁
	KeReleaseSpinLock(devExt->pQueueSpinLock, OldIrql);

	if(status == STATUS_SUCCESS)
		pIrp->IoStatus.Information = uOutSize;	
	else
		pIrp->IoStatus.Information = 0;	
	pIrp->IoStatus.Status = status;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return status;
}

NTSTATUS DriverEntry(
	IN PDRIVER_OBJECT DriverObject,
	IN PUNICODE_STRING RegistryPath)
{
	UNICODE_STRING DeviceName,Win32Device;
	PC2P_DEV_EXT devExt;
	
	ULONG i;
	NTSTATUS status;
	DbgPrint("[ENTRY]It's entry...");
	
	
	// 初始化符号链接字符串
	RtlInitUnicodeString(&DeviceName, DEVICE_NAME);
	RtlInitUnicodeString(&Win32Device, LINK_NAME);
	
	// 保存默认派遣函数地址
	IopInvalidDeviceRequest = DriverObject->MajorFunction[0];

	// 填写所有分发函数的地址
	for (i=0; i<IRP_MJ_MAXIMUM_FUNCTION; ++i)
	{
		DriverObject->MajorFunction[i] = c2pDispatchGeneral;
	}

	// 单独填写一个Read分发函数
	DriverObject->MajorFunction[IRP_MJ_READ] = c2pDispatchRead;

	// 单独填写一个IRP_MJ_POWER函数
	DriverObject->MajorFunction[IRP_MJ_POWER] = c2pPower;

	// 我们想知道什么时候绑定过的设备被卸载了
	// 所以写一个PNP(即插即用)分发函数
	DriverObject->MajorFunction[IRP_MJ_PNP] = c2pPnP;
	
	// 卸载函数
	DriverObject->DriverUnload = c2pUnload;
	
	// 保存一份全局驱动对象指针
	gDriverObject = DriverObject;
	
	
	
	// 控制设备的通信
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DispatchIoctl;
	DriverObject->MajorFunction[IRP_MJ_CREATE] = DispatchCreate;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = DispatchClose;
	
	
	// 开始生成控制设备
	status = IoCreateDevice(DriverObject, sizeof(C2P_DEV_EXT), &DeviceName, FILE_DEVICE_UNKNOWN, 0, FALSE, &g_devControl);
	if (!NT_SUCCESS(status))
		return status;
	if (!g_devControl)
		return STATUS_UNEXPECTED_IO_ERROR;
	
	g_devControl->Flags |= DO_BUFFERED_IO;
	
	status = IoCreateSymbolicLink(&Win32Device, &DeviceName);
	if(!NT_SUCCESS(status))
	{
		IoDeleteDevice(g_devControl); 
		return status;
	}
	
	//设备扩展
	devExt = (PC2P_DEV_EXT)(g_devControl->DeviceExtension);
	devExt->pMonitorQueue = (PQUEUE)ExAllocatePool(NonPagedPool, sizeof(QUEUE));
	devExt->pQueueSpinLock = (PKSPIN_LOCK)ExAllocatePool(NonPagedPool, sizeof(KSPIN_LOCK));
	CreateQueue(devExt->pMonitorQueue);
	KeInitializeSpinLock(devExt->pQueueSpinLock);
	
	// 绑定所有键盘设备
	status = c2pAttachDevices(DriverObject, RegistryPath, devExt->pMonitorQueue, devExt->pQueueSpinLock);

	// 启动成功
	DbgPrint("[KeyboardFilter]DriverEntry\n");

	return status;
}