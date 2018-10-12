#include <ntddk.h>

#define DEVICE_NAME L"\\device\\ntmodeldrv"  //设备名device开头固定，后面随便取，一般和编译出的.sys一致
#define LINK_NAME L"\\dosdevices\\ntmodeldrv"

#define IOCTRL_BASE 0X8000  //定义IoControl起始范围

#define MYIOCTRL_CODE(i) \
	CTL_CODE(FILE_DEVICE_UNKNOWN,IOCTRL_BASE+i,METHOD_BUFFERED,FILE_ANY_ACCESS)//这里倒数第二个参数是访问模式

#define CTL_HELLO MYIOCTRL_CODE(0)
#define CTL_PRINT MYIOCTRL_CODE(1)
#define CTL_BYE MYIOCTRL_CODE(2)

//定义Ring3的分发函数

//创建分发函数
NTSTATUS DispatchCommon(PDEVICE_OBJECT pObject, PIRP pIrp)
{
	pIrp->IoStatus.Status = STATUS_SUCCESS;//这个是给ring3的返回值
	pIrp->IoStatus.Information = 0;//记录Irp特殊数据，比如读写就是有效字节数
	
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;//这个是给内核驱动管理框架的
}

NTSTATUS DispatchCreate(PDEVICE_OBJECT pObject, PIRP pIrp)//第一个参数驱动设备对象，在DriverEntry创建
{
	pIrp->IoStatus.Status = STATUS_SUCCESS;//这个是给ring3的返回值
	pIrp->IoStatus.Information = 0;//记录Irp特殊数据，比如读写就是有效字节数

	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;//这个是给内核驱动管理框架的
}

NTSTATUS DispatchRead(PDEVICE_OBJECT pObject,PIRP pIrp)
{
	//定义读Buffer指针
	PVOID pReadBuffer = NULL;
	ULONG uReadLength = 0;
	PIO_STACK_LOCATION pStack = NULL;
	ULONG uMin = 0;
	ULONG uHelloStr = 0;

	uHelloStr = (wcslen(L"hello world") + 1)*sizeof(WCHAR);

	
	pReadBuffer = pIrp->AssociatedIrp.SystemBuffer;//地址

	pStack = IoGetCurrentIrpStackLocation(pIrp);//获得当前层IRP栈的指针
	
	uReadLength = pStack->Parameters.Read.Length;//从栈的结构体里的联合体获得长度
	uMin = uReadLength > uHelloStr ? uHelloStr : uReadLength;

	RtlCopyMemory(pReadBuffer, L"hello world", uMin);//这里要用目的和源最小的，以防泄露和溢出

	pIrp->IoStatus.Status = STATUS_SUCCESS;
	pIrp->IoStatus.Information = uMin;

	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;

}

NTSTATUS DispatchWrite(PDEVICE_OBJECT pObject, PIRP pIrp)
{
	PVOID pWriteBuffer = NULL;
	ULONG uWriteLength = 0;
	PIO_STACK_LOCATION pStack = NULL;

	PVOID pBuffer = NULL;

	pWriteBuffer = pIrp->AssociatedIrp.SystemBuffer;

	pStack = IoGetCurrentIrpStackLocation(pIrp);
	uWriteLength = pStack->Parameters.Write.Length;

	//模拟写，就是分配个内存拷进去数据

	pBuffer = ExAllocatePoolWithTag(PagedPool, uWriteLength, 'NQRY');

	if (NULL == pBuffer)
	{
		pIrp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
		pIrp->IoStatus.Information = 0;
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);

		return STATUS_INSUFFICIENT_RESOURCES;
	}


	//分配成功
	//初始化申请的内存

	memset(pBuffer, 0, uWriteLength);
	RtlCopyMemory(pBuffer, pWriteBuffer, uWriteLength);

	//释放buffer
	ExFreePool(pBuffer);
	pBuffer = NULL;

	pIrp->IoStatus.Status = STATUS_SUCCESS;
	pIrp->IoStatus.Information = uWriteLength;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;

}

NTSTATUS DispatchClose(PDEVICE_OBJECT pObject, PIRP pIrp)
{
	pIrp->IoStatus.Status = STATUS_SUCCESS;//这个是给ring3的返回值
	pIrp->IoStatus.Information = 0;//记录Irp特殊数据，比如读写就是有效字节数

	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;//这个是给内核驱动管理框架的
}

NTSTATUS DispatchClean(PDEVICE_OBJECT pObject, PIRP pIrp)
{
	pIrp->IoStatus.Status = STATUS_SUCCESS;//这个是给ring3的返回值
	pIrp->IoStatus.Information = 0;//记录Irp特殊数据，比如读写就是有效字节数

	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;//这个是给内核驱动管理框架的
}

NTSTATUS DispatchIoControl(PDEVICE_OBJECT pObject, PIRP pIrp)
{
	ULONG uIoCtrlCode = 0;
	PVOID pInputBuffer = NULL;
	PVOID pOutputBuffer = NULL;

	ULONG uInputLengrh = 0;
	ULONG uOutputLength = 0;

	PIO_STACK_LOCATION pStack = NULL;
	pInputBuffer = pOutputBuffer = pIrp->AssociatedIrp.SystemBuffer;

	pStack = IoGetCurrentIrpStackLocation(pIrp);

	uInputLengrh = pStack->Parameters.DeviceIoControl.InputBufferLength;
	uOutputLength = pStack->Parameters.DeviceIoControl.OutputBufferLength;

	uIoCtrlCode = pStack->Parameters.DeviceIoControl.IoControlCode;

	//根据不同code处理不同代码

	switch (uIoCtrlCode)
	{
	case CTL_HELLO:
		DbgPrint("Hello IoCtrlControl\n");
		break;
	case CTL_PRINT:
		DbgPrint("%ws\n",pInputBuffer);
		break;
	case CTL_BYE:
		DbgPrint("Good Bye\n");
		break;
	default:
		DbgPrint("UnKown IoCtrlControl\n");
		break;
	}

	pIrp->IoStatus.Status = STATUS_SUCCESS;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;

}

VOID DriverUnload(PDRIVER_OBJECT pDriverObject)
{
	UNICODE_STRING uLinkName = { 0 };
	RtlInitUnicodeString(&uLinkName, LINK_NAME);
	//删除符号链接
	IoDeleteSymbolicLink(&uLinkName);
	//释放设备对象
	IoDeleteDevice(pDriverObject->DeviceObject);

	DbgPrint("Driver UnLoaded\n");
}

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject,
	PUNICODE_STRING pRegPath)
{
	UNICODE_STRING uDeviceName = { 0 };
	UNICODE_STRING uLinkName = { 0 };
	NTSTATUS ntStatus = 0;
	PDEVICE_OBJECT pDeviceObject = NULL;
	ULONG i = 0;

	DbgPrint("Driver load begin\n");

	//创建设备对象，将之前定义的设备名传入，接收IRP
	RtlInitUnicodeString(&uDeviceName, DEVICE_NAME);
	RtlInitUnicodeString(&uLinkName, LINK_NAME);

	ntStatus = IoCreateDevice(pDriverObject,
		0, &uDeviceName, FILE_DEVICE_UNKNOWN, 0, FALSE, &pDeviceObject);
	if (!NT_SUCCESS(ntStatus))
	{
		DbgPrint("IoCreateDevice Failed:%x\n", ntStatus);
		return ntStatus;
	}

	pDeviceObject->Flags |= DO_BUFFERED_IO;//ring3和ring0通讯协议

	//创建符号链接，为了让Ring3看到识别驱动
	ntStatus = IoCreateSymbolicLink(&uLinkName, &uDeviceName);
	if (!NT_SUCCESS(ntStatus))
	{
		IoDeleteDevice(pDeviceObject);//失败先删掉
		DbgPrint("IoCreateDevice Failed:%x\n", ntStatus);
		return ntStatus;
	}
	//初始和注册化分发函数
	for (i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++)
	{
		pDriverObject->MajorFunction[i] = DispatchCommon;

	}
	//初始化特殊分发
	pDriverObject->MajorFunction[IRP_MJ_CREATE] = DispatchCreate;
	pDriverObject->MajorFunction[IRP_MJ_READ] = DispatchRead;
	pDriverObject->MajorFunction[IRP_MJ_WRITE] = DispatchWrite;
	pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DispatchIoControl;
	pDriverObject->MajorFunction[IRP_MJ_CLEANUP] = DispatchClean;
	pDriverObject->MajorFunction[IRP_MJ_CLOSE] = DispatchClose;

	pDriverObject->DriverUnload = DriverUnload;

	DbgPrint("Driver load OK!\n");

	//

	return STATUS_SUCCESS;
}

