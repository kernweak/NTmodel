#include <ntddk.h>

#define DEVICE_NAME L"\\device\\ntmodeldrv"  //�豸��device��ͷ�̶����������ȡ��һ��ͱ������.sysһ��
#define LINK_NAME L"\\dosdevices\\ntmodeldrv"

#define IOCTRL_BASE 0X8000  //����IoControl��ʼ��Χ

#define MYIOCTRL_CODE(i) \
	CTL_CODE(FILE_DEVICE_UNKNOWN,IOCTRL_BASE+i,METHOD_BUFFERED,FILE_ANY_ACCESS)//���ﵹ���ڶ��������Ƿ���ģʽ

#define CTL_HELLO MYIOCTRL_CODE(0)
#define CTL_PRINT MYIOCTRL_CODE(1)
#define CTL_BYE MYIOCTRL_CODE(2)

//����Ring3�ķַ�����

//�����ַ�����
NTSTATUS DispatchCommon(PDEVICE_OBJECT pObject, PIRP pIrp)
{
	pIrp->IoStatus.Status = STATUS_SUCCESS;//����Ǹ�ring3�ķ���ֵ
	pIrp->IoStatus.Information = 0;//��¼Irp�������ݣ������д������Ч�ֽ���
	
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;//����Ǹ��ں����������ܵ�
}

NTSTATUS DispatchCreate(PDEVICE_OBJECT pObject, PIRP pIrp)//��һ�����������豸������DriverEntry����
{
	pIrp->IoStatus.Status = STATUS_SUCCESS;//����Ǹ�ring3�ķ���ֵ
	pIrp->IoStatus.Information = 0;//��¼Irp�������ݣ������д������Ч�ֽ���

	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;//����Ǹ��ں����������ܵ�
}

NTSTATUS DispatchRead(PDEVICE_OBJECT pObject,PIRP pIrp)
{
	//�����Bufferָ��
	PVOID pReadBuffer = NULL;
	ULONG uReadLength = 0;
	PIO_STACK_LOCATION pStack = NULL;
	ULONG uMin = 0;
	ULONG uHelloStr = 0;

	uHelloStr = (wcslen(L"hello world") + 1)*sizeof(WCHAR);

	
	pReadBuffer = pIrp->AssociatedIrp.SystemBuffer;//��ַ

	pStack = IoGetCurrentIrpStackLocation(pIrp);//��õ�ǰ��IRPջ��ָ��
	
	uReadLength = pStack->Parameters.Read.Length;//��ջ�Ľṹ������������ó���
	uMin = uReadLength > uHelloStr ? uHelloStr : uReadLength;

	RtlCopyMemory(pReadBuffer, L"hello world", uMin);//����Ҫ��Ŀ�ĺ�Դ��С�ģ��Է�й¶�����

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

	//ģ��д�����Ƿ�����ڴ濽��ȥ����

	pBuffer = ExAllocatePoolWithTag(PagedPool, uWriteLength, 'NQRY');

	if (NULL == pBuffer)
	{
		pIrp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
		pIrp->IoStatus.Information = 0;
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);

		return STATUS_INSUFFICIENT_RESOURCES;
	}


	//����ɹ�
	//��ʼ��������ڴ�

	memset(pBuffer, 0, uWriteLength);
	RtlCopyMemory(pBuffer, pWriteBuffer, uWriteLength);

	//�ͷ�buffer
	ExFreePool(pBuffer);
	pBuffer = NULL;

	pIrp->IoStatus.Status = STATUS_SUCCESS;
	pIrp->IoStatus.Information = uWriteLength;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;

}

NTSTATUS DispatchClose(PDEVICE_OBJECT pObject, PIRP pIrp)
{
	pIrp->IoStatus.Status = STATUS_SUCCESS;//����Ǹ�ring3�ķ���ֵ
	pIrp->IoStatus.Information = 0;//��¼Irp�������ݣ������д������Ч�ֽ���

	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;//����Ǹ��ں����������ܵ�
}

NTSTATUS DispatchClean(PDEVICE_OBJECT pObject, PIRP pIrp)
{
	pIrp->IoStatus.Status = STATUS_SUCCESS;//����Ǹ�ring3�ķ���ֵ
	pIrp->IoStatus.Information = 0;//��¼Irp�������ݣ������д������Ч�ֽ���

	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;//����Ǹ��ں����������ܵ�
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

	//���ݲ�ͬcode����ͬ����

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
	//ɾ����������
	IoDeleteSymbolicLink(&uLinkName);
	//�ͷ��豸����
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

	//�����豸���󣬽�֮ǰ������豸�����룬����IRP
	RtlInitUnicodeString(&uDeviceName, DEVICE_NAME);
	RtlInitUnicodeString(&uLinkName, LINK_NAME);

	ntStatus = IoCreateDevice(pDriverObject,
		0, &uDeviceName, FILE_DEVICE_UNKNOWN, 0, FALSE, &pDeviceObject);
	if (!NT_SUCCESS(ntStatus))
	{
		DbgPrint("IoCreateDevice Failed:%x\n", ntStatus);
		return ntStatus;
	}

	pDeviceObject->Flags |= DO_BUFFERED_IO;//ring3��ring0ͨѶЭ��

	//�����������ӣ�Ϊ����Ring3����ʶ������
	ntStatus = IoCreateSymbolicLink(&uLinkName, &uDeviceName);
	if (!NT_SUCCESS(ntStatus))
	{
		IoDeleteDevice(pDeviceObject);//ʧ����ɾ��
		DbgPrint("IoCreateDevice Failed:%x\n", ntStatus);
		return ntStatus;
	}
	//��ʼ��ע�ữ�ַ�����
	for (i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++)
	{
		pDriverObject->MajorFunction[i] = DispatchCommon;

	}
	//��ʼ������ַ�
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

