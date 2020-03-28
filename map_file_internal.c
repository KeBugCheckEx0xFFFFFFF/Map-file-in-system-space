#include <ntifs.h>
#include <ntddk.h>

#define SEC_IMAGE 0x1000000

typedef NTSTATUS (*fnMmCreateSection)(
OUT PVOID *SectionObject,
IN ACCESS_MASK DesiredAccess,
IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
IN PLARGE_INTEGER MaximumSize,
IN ULONG SectionPageProtection,
IN ULONG AllocationAttributes,
IN HANDLE FileHandle OPTIONAL,
IN PFILE_OBJECT File OPTIONAL
);

typedef NTSTATUS (*ptrMiCreateSection)(PVOID *a1, POBJECT_ATTRIBUTES a2, char a3, PLARGE_INTEGER *a4, ULONG a5, ULONG a6, char a7, HANDLE a8, PFILE_OBJECT a9, KPROCESSOR_MODE a10);
typedef NTSTATUS (*ptrMiMapViewInSystemSpace)(PVOID a,PVOID b,PVOID *c, PSIZE_T size, PULONG a3,ULONG mask OPTIONAL);
typedef NTSTATUS (*ptrMiRemoveFromSystemSpace)(PVOID Session,PVOID mappedbase,INT a1);
typedef NTSTATUS (*ptrIopCreateFile)(
	PHANDLE            FileHandle,
	ACCESS_MASK        DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes,
	PIO_STATUS_BLOCK   IoStatusBlock,
	PLARGE_INTEGER     AllocationSize,
	ULONG              FileAttributes,
	ULONG              ShareAccess,
	ULONG              Disposition,
	ULONG              CreateOptions,
	PVOID              EaBuffer,
	ULONG              EaLength,
	CREATE_FILE_TYPE   CreateFileType,
	PVOID              InternalParameters,
	ULONG              Options,
	ULONG Flags,
	PVOID pIoDriverCreateContext

	);



VOID Unload(PDRIVER_OBJECT pdriver)
{
	DbgPrint("\r\n");
}



NTSTATUS DriverEntry(PDRIVER_OBJECT pdriver, PUNICODE_STRING pregister)
{

	NTSTATUS nStatus = STATUS_SUCCESS;
	UNICODE_STRING uni;
	OBJECT_ATTRIBUTES oa;
	IO_STATUS_BLOCK io;
	HANDLE handle;
	FILE_STANDARD_INFORMATION fileinfo;
	PVOID section = NULL;
	PFILE_OBJECT fileobject = NULL;
	SIZE_T viewsize = 0;
	PVOID mappedbase = NULL;
	ptrMiCreateSection MiCreateSection = NULL;
	ptrMiMapViewInSystemSpace MiMapViewInSystemSpace = NULL;
	ptrMiRemoveFromSystemSpace MiRemoveFromSystemSpace = NULL;
	ptrIopCreateFile IopCreateFileWin81 = NULL;
	PVOID buffer = NULL;
	ULONG value = 0;

	PVOID MmSession = 0xfffff801f73451e0;
	IopCreateFileWin81 = (ptrIopCreateFile)0xfffff801f7444470;
	MiCreateSection = (ptrMiCreateSection)0xfffff801f7436350;
	MiMapViewInSystemSpace = (ptrMiMapViewInSystemSpace)0xfffff801f7493108;
	MiRemoveFromSystemSpace = (ptrMiRemoveFromSystemSpace)0xfffff801f71399a4;

	pdriver->DriverUnload = (PDRIVER_UNLOAD)Unload;

	RtlInitUnicodeString(&uni, L"\\??\\C:\\Windows\\System32\\ntdll.dll");
	InitializeObjectAttributes(&oa, &uni, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);

	nStatus = IopCreateFileWin81(&handle, FILE_GENERIC_READ | SYNCHRONIZE, &oa,
		&io, NULL, FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ, FILE_OPEN,
		FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0,
		CreateFileTypeNone, NULL, IO_NO_PARAMETER_CHECKING, 0, NULL);

	if (NT_SUCCESS(nStatus))
	{
		
			InitializeObjectAttributes(&oa, NULL, OBJ_KERNEL_HANDLE, NULL, NULL);

			RtlSecureZeroMemory(&fileinfo, sizeof(FILE_STANDARD_INFORMATION));
			nStatus = ZwQueryInformationFile(handle, &io, &fileinfo, sizeof(FILE_STANDARD_INFORMATION), FileStandardInformation);
			if (NT_SUCCESS(nStatus))
			{
				nStatus = ObReferenceObjectByHandle(handle, FILE_GENERIC_READ | SYNCHRONIZE, *IoFileObjectType, KernelMode, &fileobject, NULL);
				if (NT_SUCCESS(nStatus))
				{
					
					nStatus = MiCreateSection(&section,NULL,2,&fileinfo.EndOfFile.QuadPart,PAGE_READWRITE,SEC_COMMIT,0,handle,fileobject,KernelMode);
					if (NT_SUCCESS(nStatus))
					{
						
						nStatus = MiMapViewInSystemSpace(section, MmSession, &mappedbase, &viewsize,&value,0);
						if (NT_SUCCESS(nStatus))
						{
							DbgPrint("\r\nSection successfully mapped in system space");
						}

					}

					ObfDereferenceObject(fileobject);

				}

			}		
	}

	if (handle)
		ZwClose(handle);
	if (mappedbase)
		MiRemoveFromSystemSpace(MmSession, mappedbase, 0);


	return nStatus;
	
}