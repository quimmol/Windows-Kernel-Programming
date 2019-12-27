#include <ntddk.h>

void SampleUnload(_In_ PDRIVER_OBJECT DriverObject) {
	UNREFERENCED_PARAMETER(DriverObject);
	KdPrint(("[*] Driver unloaded!"));
}

extern "C"
NTSTATUS
DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath) {
	UNREFERENCED_PARAMETER(RegistryPath);
	DriverObject->DriverUnload = SampleUnload;

	KdPrint(("[*] Driver loaded!"));

	

	RTL_OSVERSIONINFOW osversion = { sizeof(osversion)};
	
	NTSTATUS status = RtlGetVersion(&osversion);
	
	/*
	KdPrint(("dwBuildNumber: %d\n", osversion.dwBuildNumber));
	KdPrint(("dwMajorVersion: %d\n", osversion.dwMajorVersion));
	KdPrint(("dwMinorVersion: %d\n", osversion.dwMinorVersion));
	KdPrint(("dwOSVersionInfoSize: %d\n", osversion.dwOSVersionInfoSize));
	KdPrint(("dwPlatformId: %d\n", osversion.dwPlatformId));
	*/

	if (NT_SUCCESS(status))
	{
		switch (osversion.dwMajorVersion)
		{
			case 10:
				KdPrint(("Windows 10 BUILD %d\n", osversion.dwBuildNumber));
				break;
			case 6:
				if (osversion.dwMinorVersion == 3)
				{
					KdPrint(("Windows 8.1 BUILD %d\n", osversion.dwBuildNumber));
				}
				else if (osversion.dwMinorVersion == 2)
				{
					KdPrint(("Windows 8 BUILD %d\n", osversion.dwBuildNumber));
				}
				else if (osversion.dwMinorVersion == 1)
				{
					KdPrint(("Windows 7 BUILD %d\n", osversion.dwBuildNumber));
				}
				else
				{
					KdPrint(("Windows Vista BUILD %d\n", osversion.dwBuildNumber));
				}
				break;
			case 5: 
				KdPrint(("Windows XP BUILD %d\n", osversion.dwBuildNumber));
				break;
			default:
				KdPrint(("Default case"));
				break;
		}
	}
	else
	{
		KdPrint(("[*] RtlGetVersion() error"));
	}
	
	return STATUS_SUCCESS;
}