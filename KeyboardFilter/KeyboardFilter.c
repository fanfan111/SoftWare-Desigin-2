#include <ntddk.h>
#include <wdm.h>
#include <ntstrsafe.h>

ULONG gC2pKeyCount = 0;
PDRIVER_OBJECT gDriverObject;

//ʵ������һ��ȫ�ֱ���������ͷ�ļ���û��
extern POBJECT_TYPE *IoDriverObjectType;

//KbdClass����������
#define KBD_DRIVER_NAME L"\\Driver\\kbdclass"
#define DELAY_ONE_MICROSECOND   (-10)
#define DELAY_ONE_MILLISECOND   (DELAY_ONE_MICROSECOND*1000)


//-----------------------------------------------------------------------------------------------------------------

//�����������
#define	DEVICE_NAME			L"\\Device\\KeyboardFilter"
#define LINK_NAME			L"\\DosDevices\\KeyboardFilter"
#define LINK_GLOBAL_NAME	L"\\DosDevices\\Global\\KeyboardFilter"

//ȫ�ֿ����豸����
#pragma LOCKEDDATA
PDEVICE_OBJECT g_devControl = NULL;

//�����Ĭ����ǲ����ָ��
NTSTATUS (*IopInvalidDeviceRequest)(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp) = NULL;

#include <windef.h>

#define IOCTL_CMD_READ_KEY	 		 	 CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CMD_CLEAN_MONITOR			 CTL_CODE(FILE_DEVICE_UNKNOWN, 0x806, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define MAX_QUEUE_SIZE 500

typedef struct queue
{  
    UCHAR data[MAX_QUEUE_SIZE];  
    ULONG front;    //ָ����е�һ��Ԫ��
    ULONG rear;     //ָ��������һ��Ԫ�ص���һ��Ԫ��
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



//�˺�����ʵ����
//����һ�¾Ϳ�ʹ��
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
	

//�豸��չ
typedef struct _C2P_DEV_EXT
{
	//�ṹ�Ĵ�С
	ULONG NodeSize;
	//�����豸����
	PDEVICE_OBJECT pFilterDeviceObject;
	//ͬʱ����ʱ�ı�����
	KSPIN_LOCK IoRequestSpinLock;
	//���̼�ͬ������
	KEVENT IoInProgressEvent;
	//�󶨵��豸����
	PDEVICE_OBJECT TargetDeviceObject;
	//��ǰ�ײ��豸����
	PDEVICE_OBJECT LowerDeviceObject;
	
	//Ϊͨ������
	PQUEUE pMonitorQueue;
	PKSPIN_LOCK pQueueSpinLock; //������
	
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


//��������������죬�ܴ���������KbdClass��Ȼ��
//��������������豸
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

	//��ʼ��һ���ַ�����KbdClass����������
	RtlInitUnicodeString(&uniNtNameString, KBD_DRIVER_NAME);

	//����������
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
		//����ObReferenceObjectByName�ᵼ��������������ü�������
		//������Ӧ�ĵ��ý�����ObDereferenceObject
		ObDereferenceObject(DriverObject);
	}

	//�����豸���еĵ�һ���豸
	pTargetDeviceObject = KbdDriverObject->DeviceObject;
	while (pTargetDeviceObject)
	{
		//����һ�������豸
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

		//�󶨡�pLowerDeviceObject�ǰ�֮��õ�����һ���豸
		//Ҳ������ʵ�豸
		pLowerDeviceObject =
			IoAttachDeviceToDeviceStack(pFilterDeviceObject,
				pTargetDeviceObject);
		//�����ʧ�ܣ�����֮ǰ�Ĳ������˳�
		if (!pLowerDeviceObject)
		{
			DbgPrint("[ATTACH] fail to attach to Device Object");
			IoDeleteDevice(pFilterDeviceObject);
			pFilterDeviceObject = NULL;
			return (status);
		}

		
		//�豸��չ
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

		// �ƶ�����һ���豸����������
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
	//�ѵ�ǰ�߳�����Ϊ��ʵʱģʽ���Ա����������о�����Ӱ����������
	KeSetPriorityThread(CurrentThread, LOW_REALTIME_PRIORITY);

	UNREFERENCED_PARAMETER(DriverObject);
	DbgPrint("[Unload] Driver unLoading...\n");
	
	
	//ж�ؿ����豸
	RtlInitUnicodeString(&Win32Device, LINK_NAME);
	IoDeleteSymbolicLink(&Win32Device);
	IoDeleteDevice(g_devControl);
	DbgPrint("[Unload] KeyboardFilter unLoad OK!\n");

	
	//���������豸��һ�ɽ����
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
	//�����豸�ַ���Ĭ����ǲ����
	if (DeviceObject == g_devControl)
	{
		return IopInvalidDeviceRequest(DeviceObject, Irp);
	}
	
	//�����ķַ�������ֱ��skip��Ȼ����IoCallDriver
	//��irp���͵���ʵ�豸���豸����
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

	//�����ʵ�豸
	devExt = (PC2P_DEV_EXT)(DeviceObject->DeviceExtension);
	irpStack = IoGetCurrentIrpStackLocation(Irp);

	switch (irpStack->MinorFunction)
	{
	case IRP_MN_REMOVE_DEVICE:
		DbgPrint(("IRP_MN_REMOVE_DEVICE\n"));

		//���Ȱ�������ȥjie'chu
		IoSkipCurrentIrpStackLocation(Irp);
		IoCallDriver(devExt->LowerDeviceObject, Irp);
		//Ȼ������
		IoDetachDevice(devExt->LowerDeviceObject);
		status = STATUS_SUCCESS;
		break;
	default:
		//�����������͵�IRP��ȫ��ֱ���·�����
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

	//������������ǳɹ��ģ��������ʧ�ܣ���ȡ
	//��һ������Ϣ���������
	if (NT_SUCCESS(Irp->IoStatus.Status))
	{
		//��ö�������ɺ�����������
		buf = Irp->AssociatedIrp.SystemBuffer;
		//��û������ĳ��ȣ�
		//һ����˵�����ܷ���ֵ�ж೤��������Infomation��
		buf_len = (ULONG)Irp->IoStatus.Information;
		
		
		devExt = (PC2P_DEV_EXT)(DeviceObject->DeviceExtension);
		
		//��ȡ������
		KeAcquireSpinLock(devExt->pQueueSpinLock, &OldIrql);
		
		if (buf_len >= 5)
		{
			if (buf[4]==0)        //��ͨ
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
		
		//�ͷ�������
		KeReleaseSpinLock(devExt->pQueueSpinLock, OldIrql);
		
		//��ӡ��ʾ
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

	//ȫ�ֱ�����������1
	++gC2pKeyCount;

	//�õ��豸��չ��Ŀ����Ϊ�˻����һ���豸��ָ��
	devExt =
		(PC2P_DEV_EXT)DeviceObject->DeviceExtension;

	//���ûص���������IRP������ȥ��������ͽ�����
	currentIrpStack = IoGetCurrentIrpStackLocation(Irp);
	IoCopyCurrentIrpStackLocationToNext(Irp);
	IoSetCompletionRoutine(Irp, c2pReadComplete,
		DeviceObject, TRUE, TRUE, TRUE);
	return IoCallDriver(devExt->LowerDeviceObject, Irp);
}

NTSTATUS DispatchCreate(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
	//�ǿ����豸�����ַ�
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
	//�ǿ����豸�����ַ�
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
	
	//�ǿ����豸�����ַ�
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
	
	//��ȡ������
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
	
	//�ͷ�������
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
	
	
	// ��ʼ�����������ַ���
	RtlInitUnicodeString(&DeviceName, DEVICE_NAME);
	RtlInitUnicodeString(&Win32Device, LINK_NAME);
	
	// ����Ĭ����ǲ������ַ
	IopInvalidDeviceRequest = DriverObject->MajorFunction[0];

	// ��д���зַ������ĵ�ַ
	for (i=0; i<IRP_MJ_MAXIMUM_FUNCTION; ++i)
	{
		DriverObject->MajorFunction[i] = c2pDispatchGeneral;
	}

	// ������дһ��Read�ַ�����
	DriverObject->MajorFunction[IRP_MJ_READ] = c2pDispatchRead;

	// ������дһ��IRP_MJ_POWER����
	DriverObject->MajorFunction[IRP_MJ_POWER] = c2pPower;

	// ������֪��ʲôʱ��󶨹����豸��ж����
	// ����дһ��PNP(���弴��)�ַ�����
	DriverObject->MajorFunction[IRP_MJ_PNP] = c2pPnP;
	
	// ж�غ���
	DriverObject->DriverUnload = c2pUnload;
	
	// ����һ��ȫ����������ָ��
	gDriverObject = DriverObject;
	
	
	
	// �����豸��ͨ��
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DispatchIoctl;
	DriverObject->MajorFunction[IRP_MJ_CREATE] = DispatchCreate;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = DispatchClose;
	
	
	// ��ʼ���ɿ����豸
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
	
	//�豸��չ
	devExt = (PC2P_DEV_EXT)(g_devControl->DeviceExtension);
	devExt->pMonitorQueue = (PQUEUE)ExAllocatePool(NonPagedPool, sizeof(QUEUE));
	devExt->pQueueSpinLock = (PKSPIN_LOCK)ExAllocatePool(NonPagedPool, sizeof(KSPIN_LOCK));
	CreateQueue(devExt->pMonitorQueue);
	KeInitializeSpinLock(devExt->pQueueSpinLock);
	
	// �����м����豸
	status = c2pAttachDevices(DriverObject, RegistryPath, devExt->pMonitorQueue, devExt->pQueueSpinLock);

	// �����ɹ�
	DbgPrint("[KeyboardFilter]DriverEntry\n");

	return status;
}