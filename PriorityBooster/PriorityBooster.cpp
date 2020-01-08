#include <ntifs.h>
#include <ntddk.h>
#include "PriorityBoosterCommon.h"

// prototypes

void PriorityBoosterUnload(_In_ PDRIVER_OBJECT DriverObject);
NTSTATUS PriorityBoosterCreateClose(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp);
NTSTATUS PriorityBoosterDeviceControl(PDEVICE_OBJECT, PIRP Irp);

// DriverEntry

extern "C" NTSTATUS
DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath) {
	UNREFERENCED_PARAMETER(RegistryPath);

	DriverObject->DriverUnload = PriorityBoosterUnload;

	DriverObject->MajorFunction[IRP_MJ_CREATE] = PriorityBoosterCreateClose;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = PriorityBoosterCreateClose;

	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = PriorityBoosterDeviceControl;

	/*
	*	Nombre del dispositivo para crear el objeto
	*/
	UNICODE_STRING devName = RTL_CONSTANT_STRING(L"\\Device\\PriorityBooster");	// RtlInitUnicodeString(&devName, L"\\Device\\ThreadBoost");

	/*
	*	Creacion del dispositivo
	*/
	PDEVICE_OBJECT DeviceObject;
	NTSTATUS status = IoCreateDevice(
		DriverObject,			// our driver object,
		0,						// no need for extra bytes,
		&devName,				// the device name,
		FILE_DEVICE_UNKNOWN,	// device type,
		0,						// characteristics flags,
		FALSE,					// not exclusive,
		&DeviceObject			// the resulting pointer
	);

	if (!NT_SUCCESS(status)) 
	{
		KdPrint(("Failed to create device object (0x%08X)\n", status));
		return status;
	}

	/*
	*	Mediante este symlink el cliente puede enviar irp request al driver
	*/
	UNICODE_STRING symLink = RTL_CONSTANT_STRING(L"\\??\\PriorityBooster");

	status = IoCreateSymbolicLink(&symLink, &devName);
	
	if (!NT_SUCCESS(status)) 
	{
		KdPrint(("Failed to create symbolic link (0x%08X)\n", status));
		IoDeleteDevice(DeviceObject);
		return status;
	}

	return STATUS_SUCCESS;
}

void PriorityBoosterUnload(_In_ PDRIVER_OBJECT DriverObject) 
{
	UNICODE_STRING symLink = RTL_CONSTANT_STRING(L"\\??\\PriorityBooster");
	// delete symbolic link
	IoDeleteSymbolicLink(&symLink);
	// delete device object
	IoDeleteDevice(DriverObject->DeviceObject);
}

_Use_decl_annotations_
NTSTATUS PriorityBoosterCreateClose(PDEVICE_OBJECT DeviceObject, PIRP Irp) 
{
	UNREFERENCED_PARAMETER(DeviceObject);

	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	
	return STATUS_SUCCESS;
}

_Use_decl_annotations_
NTSTATUS PriorityBoosterDeviceControl(PDEVICE_OBJECT, PIRP Irp) {
	// get our IO_STACK_LOCATION
	auto stack = IoGetCurrentIrpStackLocation(Irp); // IO_STACK_LOCATION*
	auto status = STATUS_SUCCESS;
	switch (stack->Parameters.DeviceIoControl.IoControlCode) 
	{
		case IOCTL_PRIORITY_BOOSTER_SET_PRIORITY: {

			/*
			*	Comprueba que el tamaño del buffer sea el correcto
			*/
			auto len = stack->Parameters.DeviceIoControl.InputBufferLength;
			if (len < sizeof(ThreadData)) {
				status = STATUS_BUFFER_TOO_SMALL;
				break;
			}
			
			/*
			*	Comprueba que el contenido del buffer no sea NULL
			*/
			auto data = (ThreadData*)stack->Parameters.DeviceIoControl.Type3InputBuffer;
			if (data == nullptr) {
				status = STATUS_INVALID_PARAMETER;
				break;
			}

			/*
			*	Comprueba que la prioridad deseada sea entre 1 y 31
			*/
			if (data->Priority < 1 || data->Priority > 31) {
				status = STATUS_INVALID_PARAMETER;
				break;
			}

			/*
			*	Utilizamos PsLookupThreadByThreadId para obtener el handle del thread que queremos priorizar mediante el ProcessID
			*/
			PETHREAD Thread;
			status = PsLookupThreadByThreadId(ULongToHandle(data->ThreadId), &Thread);
			if (!NT_SUCCESS(status))
				break;
			/*
			*	Damos la prioridad al proceso mediante KeSetPriorityThread -> Ke son el grupo de funciones generales del kernel.
			*/
			KeSetPriorityThread((PKTHREAD)Thread, data->Priority);
	
			/*
			*	Desreferenciar el objeto thread
			*/
			ObDereferenceObject(Thread);
			KdPrint(("Thread Priority change for %d to %d succeeded!\n",
				data->ThreadId, data->Priority));
			break;
	}
	default:
		status = STATUS_INVALID_DEVICE_REQUEST;
		break;
	}

	/*
	*	Completamos el IRP request con la información del status
	*/
	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return status;
}