//*****************************************************************************
//*
//*
//*		LPT_Driver.c
//*
//*
//*****************************************************************************
//
//	LPT_Driver-Driver für WIN-NT
//
//		Funktionen:		*)  Einblenden von Port-IO Adressräumen in
//							in einen User Prozess
//						*)  Einblenden von Speicher Adressräumen in
//							in einen User Prozess
//
//
//
//	Include - Files: LPT_Driver.h
//					 LPT_Driver_Ioctl.h
//
//
//	Latest Version:	28.8.2002, Anton Zechner
//


#include <stdlib.h>
#include <stdarg.h>
#include "LPT_Driver.h"


LPT_VERSION	sVersion={ 1,0,0,"AZ-LPT-Driver for WinNT Version 1.01" , __DATE__ "   " __TIME__ };


#define DEBUG_ON		0


#if DEBUG_ON
#define KDPRINT(a)			KdPrint(a);
#else
#define KDPRINT(a)
#endif


#ifndef INVALID_HANDLE_VALUE
#ifdef	__AMD64__
#define INVALID_HANDLE_VALUE	((HANDLE)((UINT64)0xFFFFFFFFFFFFFFFF))
#else 	
#define INVALID_HANDLE_VALUE	((HANDLE)0xFFFFFFFF)
#endif 	
#endif 	

//*****************************************************************************
//*
//*		DriverEntry
//*
//*****************************************************************************
NTSTATUS DriverEntry(	IN OUT PDRIVER_OBJECT  pDriverObject,
						IN     PUNICODE_STRING pRegistryPath)
{
NTSTATUS			uStatus;
PDEVICE_OBJECT		pDeviceObject;



	KDPRINT(("\nDreiverEntry() start"))

    pDriverObject->MajorFunction[IRP_MJ_CREATE]          = LptCreate;
    pDriverObject->MajorFunction[IRP_MJ_CLOSE]           = LptClose;
    pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]  = LptDispatch;
    pDriverObject->DriverUnload                          = LptUnload;

    uStatus = LptCreateDevice(	LPT_DEVICE_NAME_IO,
								LPT_DOS_DEVICE_NAME_IO,
								LPT_TYPE_IO,
								pDriverObject,
							   &pDeviceObject	);

	if(!(NT_SUCCESS(uStatus)))
		{
		KDPRINT(("\nDreiverEntry() error"))
		uStatus = LptFreeDevice(pDriverObject);
		return uStatus;
		}


	KDPRINT(("\nDreiverEntry() ok"))

return uStatus;
}




//*****************************************************************************
//*
//*		LptCreateDevice
//*
//*****************************************************************************
//
// 	Routine Description:
// 		This routine creates the device object and the symbolic link in
// 		\DosDevices.
//
// 		Ideally a name derived from a "Prototype", with a number appended at
// 		the end should be used.  For simplicity, just use the fixed name defined
// 		in the include file.  This means that only one device can be created.
//
// 		A symbolic link must be created between the device name and an entry
// 		in \DosDevices in order to allow Win32 applications to open the device.
//
// 	Arguments:
//
// 		pPrototypeName - Name base, # WOULD be appended to this.
//
// 		uDeviceType - Type of device to create
//
// 		pDriverObject - Pointer to driver object created by the system.
//
// 		ppDevObj - Pointer to place to store pointer to created device object
//
// 	Return Value:
//
// 		STATUS_SUCCESS if the device and link are created correctly, otherwise
// 		an error indicating the reason for failure.
//
NTSTATUS LptCreateDevice(	IN   PWSTR              pPrototypeName,
							IN   PWSTR              pDosDeviceName,
							IN   DEVICE_TYPE        uDeviceType,
							IN   PDRIVER_OBJECT     pDriverObject,
							OUT  PDEVICE_OBJECT    *ppDevObj	 )


{
NTSTATUS			uStatus;
UNICODE_STRING		sNtDeviceName;
UNICODE_STRING		sDosDeviceName;
LOCAL_DEVICE_INFO  *pDeviceInfo;




	KDPRINT(("\nLptCreateDevice() start"))

    // Get UNICODE name for device.
    RtlInitUnicodeString(&sNtDeviceName,  pPrototypeName);
	RtlInitUnicodeString(&sDosDeviceName, pDosDeviceName);

    uStatus = IoCreateDevice(   pDriverObject,
								sizeof(LOCAL_DEVICE_INFO),
							   &sNtDeviceName,
								uDeviceType,
								0,
								FALSE,
								ppDevObj
								);


	if(!NT_SUCCESS(uStatus))return uStatus;

    // Clear local device info memory

	pDeviceInfo=(LOCAL_DEVICE_INFO*)((*ppDevObj)->DeviceExtension);
    RtlZeroMemory(pDeviceInfo, sizeof(LOCAL_DEVICE_INFO));

    (*ppDevObj)->Flags |= DO_BUFFERED_IO;


    uStatus = IoCreateSymbolicLink( &sDosDeviceName, &sNtDeviceName );

    if (!NT_SUCCESS(uStatus))
		{
        IoDeleteDevice(*ppDevObj);
		return uStatus;
		}


	pDeviceInfo->pIoPmLocal = MmAllocateNonCachedMemory(IOPM_SIZE);

    if(!pDeviceInfo->pIoPmLocal)
		{
		KDPRINT(("\nLptCreateDevice() error"))
        IoDeleteDevice(*ppDevObj);
		return STATUS_INSUFFICIENT_RESOURCES;
		}


	KDPRINT(("\nLptCreateDevice() ok"))


return uStatus;
}


//*****************************************************************************
//*
//*		LptFreeDevice
//*
//*****************************************************************************
//	Error Creating Device..
//	Just free him..
NTSTATUS LptFreeDevice(PDRIVER_OBJECT pDriverObject)

{
CM_RESOURCE_LIST	 sNullResourceList;
LOCAL_DEVICE_INFO	*pDeviceInfo;
ULONG				 uStatus;
BOOLEAN				 bResourceConflict;



	KDPRINT(("\nLptFreeDevice() start"))

	pDeviceInfo=(LOCAL_DEVICE_INFO*)(pDriverObject->DeviceObject->DeviceExtension);

	RtlZeroMemory((PVOID)&sNullResourceList, sizeof(sNullResourceList));

	uStatus=IoReportResourceUsage(NULL,
								  pDriverObject,
								  &sNullResourceList,
								  sizeof(ULONG),
								  NULL,
								  NULL,
								  0,
								  FALSE,
								  &bResourceConflict);

	uStatus=STATUS_SUCCESS;

return uStatus;
}


//*****************************************************************************
//*
//*		LptCreate
//*
//*****************************************************************************
//
// 	Routine Description:
// 		This routine is called when a new Driver was opened
//
// 	Arguments:
//
// 		pDO  - Pointer to device object.
// 		pIrp - Pointer to the current IRP.
//
// 	Return Value:
//
// 		STATUS_SUCCESS if the IRP was processed successfully, otherwise an error
// 		indicating the reason for failure.
//
//
NTSTATUS LptCreate(		IN    PDEVICE_OBJECT pDO,
						IN    PIRP pIrp )
{

	KDPRINT(("\nLptCreate() start"))
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT );

return STATUS_SUCCESS;
}

//*****************************************************************************
//*
//*		LptClose
//*
//*****************************************************************************
//
// 	Routine Description:
// 		This routine is called when a new Driver was opened
//
// 	Arguments:
//
// 		pDO  - Pointer to device object.
// 		pIrp - Pointer to the current IRP.
//
// 	Return Value:
//
// 		STATUS_SUCCESS if the IRP was processed successfully, otherwise an error
// 		indicating the reason for failure.
//
//
NTSTATUS LptClose(		IN    PDEVICE_OBJECT pDO,
						IN    PIRP pIrp )
{

	KDPRINT(("\nLptClose() start"))
    pIrp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT );


return STATUS_SUCCESS;
}

//*****************************************************************************
//*
//*		LptDispatch
//*
//*****************************************************************************
//
// 	Routine Description:
// 		This routine is the dispatch handler for the driver.  It is responsible
// 		for processing the IRPs.
//
// 	Arguments:
//
// 		pDO  - Pointer to device object.
// 		pIrp - Pointer to the current IRP.
//
// 	Return Value:
//
// 		STATUS_SUCCESS if the IRP was processed successfully, otherwise an error
// 		indicating the reason for failure.
//
//
NTSTATUS LptDispatch(	IN    PDEVICE_OBJECT pDO,
						IN    PIRP pIrp )

{
PLOCAL_DEVICE_INFO	pDevInfo;
PIO_STACK_LOCATION	pStackLoc;
NTSTATUS			uStatus;
PULONG				pulPort = 0;
ULONG				ulPort = 0;


	KDPRINT(("\nLptDispatch() start"))

    //  Initialize the irp info field.
    //  This is used to return the number of bytes transfered.

    pIrp->IoStatus.Information = 0;

    pDevInfo	= (PLOCAL_DEVICE_INFO)(pDO->DeviceExtension);	// Get local info struct
    pStackLoc	= IoGetCurrentIrpStackLocation(pIrp);
    uStatus		= STATUS_NOT_IMPLEMENTED;						//  Set default return status

    // Dispatch based on major fcn code.

    switch (pStackLoc->MajorFunction)
		{
    case IRP_MJ_DEVICE_CONTROL:

        switch (pStackLoc->Parameters.DeviceIoControl.IoControlCode)
			{
		case IOCTL_LPT_READ_VERSION:

				uStatus = LptIoctlReadVersion(
							pDevInfo,
							pIrp,
							pStackLoc);
				break;

		case IOCTL_LPT_MAP_MEM:

				uStatus = LptIoctlMapMemory(
							pDevInfo,
							pIrp,
							pStackLoc);
				break;

		case IOCTL_LPT_UNMAP_MEM:

				uStatus = LptIoctlUnmapMemory(
							pDevInfo,
							pIrp,
							pStackLoc);
				break;

		case IOCTL_LPT_MAP_PORT:

				uStatus = LptIoctlMapPorts(
							pDevInfo,
							pIrp,
							pStackLoc);
				break;

		case IOCTL_LPT_UNMAP_PORT:

				uStatus = LptIoctlUnmapPorts(
							pDevInfo,
							pIrp,
							pStackLoc);
				break;

			}

		break;
    }

    pIrp->IoStatus.Status = uStatus;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT );



return uStatus;
}



//*****************************************************************************
//*
//*		LptIoctlReadVersion
//*
//*****************************************************************************
//
// 	Routine Description:
// 		This routine processes the IOCTLs which read the version info
//
// 	Arguments:
//
// 		pDevInfo        - our local device data
// 		pIrp			- IO request packet
// 		pStackLoc		- The current stack location
// 		uIoctlCode		- The ioctl code from the IRP
//
// 	Return Value:
//
// 		STATUS_SUCCESS           -- OK
//
// 		STATUS_INVALID_PARAMETER -- The buffer sent to the driver
// 									was too small to contain the
// 									port, or the buffer which
// 									would be sent back to the driver
// 									was not a multiple of the data size.
//
// 		STATUS_ACCESS_VIOLATION  -- An illegal port number was given.
//
//

NTSTATUS LptIoctlReadVersion(IN PLOCAL_DEVICE_INFO	pDevInfo,
							IN PIRP					pIrp,
							IN PIO_STACK_LOCATION	pStackLoc )




{
ULONG  *pIoBuffer;				// Pointer to transfer buffer
ULONG	uInBufferSize;			// Amount of data avail. from caller.
ULONG	uOutBufferSize;			// Max data that caller can accept.





	KDPRINT(("\nLPT: Ioctl Read-Version !"));


    uInBufferSize  = pStackLoc->Parameters.DeviceIoControl.InputBufferLength;
    uOutBufferSize = pStackLoc->Parameters.DeviceIoControl.OutputBufferLength;
    pIoBuffer      = (PULONG)pIrp->AssociatedIrp.SystemBuffer;


	if(uOutBufferSize<sizeof(LPT_VERSION))return STATUS_INVALID_PARAMETER;


	pIrp->IoStatus.Information = sizeof(LPT_VERSION);
	RtlCopyMemory(pIoBuffer,&sVersion,sizeof(LPT_VERSION));


return STATUS_SUCCESS;
}



//*****************************************************************************
//*
//*		LptIoctlMapPorts
//*
//*****************************************************************************
//
// 	Routine Description:
// 		Map's io ports into users address-space
// 	Arguments:
//
// 	    pDevInfo        - our local device data
// 		pIrp			- IO request packet
// 		pStackLoc		- The current stack location
// 		uIoctlCode		- The ioctl code from the IRP
//
// 	Return Value:
// 		STATUS_SUCCESS           -- OK
//
// 		STATUS_INVALID_PARAMETER -- The buffer sent to the driver
// 									was too small to contain the
// 									port, or the buffer which
// 									would be sent back to the driver
// 									was not a multiple of the data size.
//
// 		STATUS_ACCESS_VIOLATION  -- An illegal port number was given.
//


NTSTATUS LptIoctlMapPorts(	IN PLOCAL_DEVICE_INFO pDevInfo,
							IN PIRP pIrp,
							IN PIO_STACK_LOCATION pStackLoc )


{
										// NOTE:  Use METHOD_BUFFERED ioctls.
PULONG	pIoBuffer;						// Pointer to transfer buffer											// (treated as an array of longs).
ULONG	uInBufferSize=0;				// Amount of data avail. from caller.
ULONG	uOutBufferSize=0;				// Max data that caller can accept.
ULONG	uPort,uSize,uPos;




	KDPRINT(("\nLPT: ioctl map io ports !"));

    uInBufferSize  = pStackLoc->Parameters.DeviceIoControl.InputBufferLength;
	uOutBufferSize = pStackLoc->Parameters.DeviceIoControl.OutputBufferLength;
    pIoBuffer      = (PULONG)pIrp->AssociatedIrp.SystemBuffer;

	if(uInBufferSize<sizeof(LPT_MAP_PORT))return STATUS_INVALID_PARAMETER;

	uPort	= pIoBuffer[0];
	uSize	= pIoBuffer[1];

	if(uPort+uSize>=IOPM_SIZE*8)return STATUS_INVALID_PARAMETER;


	Ke386IoSetAccessProcess(IoGetCurrentProcess(), 1);
	Ke386QueryIoAccessMap(1,pDevInfo->pIoPmLocal);

	for(uPos=0;uPos<uSize;uPos++,uPort++)
		{
		pDevInfo->pIoPmLocal[uPort>>3] &= ~(1<<(uPort&7));
		}

	Ke386SetIoAccessMap(1,pDevInfo->pIoPmLocal);


	pIrp->IoStatus.Information = 0;



return STATUS_SUCCESS;
}

//*****************************************************************************
//*
//*		LptIoctlUnmapPorts
//*
//*****************************************************************************
//
// 	Routine Description:
// 		Unmap's io ports  into users address-space
// 	Arguments:
//
// 	    pDevInfo        - our local device data
// 		pIrp			- IO request packet
// 		pStackLoc		- The current stack location
// 		uIoctlCode		- The ioctl code from the IRP
//
// 	Return Value:
// 		STATUS_SUCCESS           -- OK
//
// 		STATUS_INVALID_PARAMETER -- The buffer sent to the driver
// 									was too small to contain the
// 									port, or the buffer which
// 									would be sent back to the driver
// 									was not a multiple of the data size.
//
// 		STATUS_ACCESS_VIOLATION  -- An illegal port number was given.
//


NTSTATUS LptIoctlUnmapPorts(	IN PLOCAL_DEVICE_INFO pDevInfo,
							IN PIRP pIrp,
							IN PIO_STACK_LOCATION pStackLoc )


{
										// NOTE:  Use METHOD_BUFFERED ioctls.
PULONG	pIoBuffer;						// Pointer to transfer buffer											// (treated as an array of longs).
ULONG	uInBufferSize=0;				// Amount of data avail. from caller.
ULONG	uOutBufferSize=0;				// Max data that caller can accept.
ULONG	uPort,uSize,uPos;




	KDPRINT(("\nLPT: ioctl unmap io ports !"));

    uInBufferSize  = pStackLoc->Parameters.DeviceIoControl.InputBufferLength;
	uOutBufferSize = pStackLoc->Parameters.DeviceIoControl.OutputBufferLength;
    pIoBuffer      = (PULONG)pIrp->AssociatedIrp.SystemBuffer;

	if(uInBufferSize<sizeof(LPT_MAP_PORT))return STATUS_INVALID_PARAMETER;

	uPort	= pIoBuffer[0];
	uSize	= pIoBuffer[1];

	if(uPort+uSize>=IOPM_SIZE*8)return STATUS_INVALID_PARAMETER;


	Ke386IoSetAccessProcess(IoGetCurrentProcess(), 1);
	Ke386QueryIoAccessMap(1,pDevInfo->pIoPmLocal);

	for(uPos=0;uPos<uSize;uPos++,uPort++)
		{
		pDevInfo->pIoPmLocal[uPort>>3] |= 1<<(uPort&7);
		}

	Ke386SetIoAccessMap(1,pDevInfo->pIoPmLocal);


	pIrp->IoStatus.Information = 0;



return STATUS_SUCCESS;
}

//*****************************************************************************
//*
//*		LptIoctlMapMemory
//*
//*****************************************************************************
//
// 	Routine Description:
// 		Map's physicaly memory into users address-space
// 	Arguments:
//
// 	    pDevInfo        - our local device data
// 		pIrp			- IO request packet
// 		pStackLoc		- The current stack location
// 		uIoctlCode		- The ioctl code from the IRP
//
// 	Return Value:
// 		STATUS_SUCCESS           -- OK
//
// 		STATUS_INVALID_PARAMETER -- The buffer sent to the driver
// 									was too small to contain the
// 									port, or the buffer which
// 									would be sent back to the driver
// 									was not a multiple of the data size.
//
// 		STATUS_ACCESS_VIOLATION  -- An illegal port number was given.
//


NTSTATUS LptIoctlMapMemory(	IN PLOCAL_DEVICE_INFO pDevInfo,
							IN PIRP pIrp,
							IN PIO_STACK_LOCATION pStackLoc
								)


{
										// NOTE:  Use METHOD_BUFFERED ioctls.
PULONG	pIoBuffer=NULL;					// Pointer to transfer buffer											// (treated as an array of longs).
ULONG	uInBufferSize=0;					// Amount of data avail. from caller.
ULONG	uOutBufferSize=0;				// Max data that caller can accept.
ULONG	uAddress,uAccess,uMode,uSize;

OBJECT_ATTRIBUTES		uObjectAttributes;
UNICODE_STRING			pNameUnicodeString;
NTSTATUS				uStatus;
HANDLE					hHandle=NULL;
PVOID					pPhysMemSection=NULL;
PHYSICAL_ADDRESS		pPhysAddress;
PHYSICAL_ADDRESS		pPhysAddressStart;
PHYSICAL_ADDRESS		pPhysAddressEnd;
PHYSICAL_ADDRESS		pVirtAddressStart;
PHYSICAL_ADDRESS		pVirtAddressLen;
ULONG					ulDataLength=0;
ULONG					ulMemType1=MEM_MEMORY_TYPE;
ULONG					ulMemType2=MEM_MEMORY_TYPE;
BOOLEAN					bTranslate1, bTranslate2;
PVOID					pToVirtMappedMem;





	KDPRINT(("\nLPT: ioctl map data memory !"));

    uInBufferSize  = pStackLoc->Parameters.DeviceIoControl.InputBufferLength;
	uOutBufferSize = pStackLoc->Parameters.DeviceIoControl.OutputBufferLength;
    pIoBuffer      = (PULONG)pIrp->AssociatedIrp.SystemBuffer;

	if(uInBufferSize <sizeof(LPT_MAP_MEM))return STATUS_INVALID_PARAMETER;
	if(uOutBufferSize<sizeof(PVOID		))return STATUS_INVALID_PARAMETER;

	uAddress = pIoBuffer[0];
	uSize	 = pIoBuffer[1];
	uAccess	 = pIoBuffer[2];


	RtlInitUnicodeString(&pNameUnicodeString,LPT_MAPMEM_NAME);

	InitializeObjectAttributes( &uObjectAttributes,
								&pNameUnicodeString,
								OBJ_CASE_INSENSITIVE,
								(HANDLE) NULL,
								(PSECURITY_DESCRIPTOR) NULL);


	uStatus = ZwOpenSection (&hHandle,
							SECTION_ALL_ACCESS,
							&uObjectAttributes);

	if(!NT_SUCCESS(uStatus))
		{
		KDPRINT(("\nLPT: Map Memory failed uStatus <%d!>",uStatus));
		return uStatus;
		}


	uStatus = ObReferenceObjectByHandle(	hHandle,
											SECTION_ALL_ACCESS,
											(POBJECT_TYPE)NULL,
											KernelMode,
											&pPhysMemSection,
											(POBJECT_HANDLE_INFORMATION) NULL);

	if(!NT_SUCCESS(uStatus))
		{
		KDPRINT(("\nLPT: Ob Reference failed uStatus <%d!>",uStatus));
		ZwClose(hHandle);
		return uStatus;
		}


	pPhysAddressStart = RtlConvertUlongToLargeInteger(uAddress);
	pPhysAddress      = RtlConvertUlongToLargeInteger(uAddress);
	pPhysAddressEnd   = RtlLargeIntegerAdd( RtlConvertUlongToLargeInteger(uAddress),
					  					    RtlConvertUlongToLargeInteger(uSize));


	// do a physical to virtual

	bTranslate1 = HalTranslateBusAddress(Isa,
										 0,
										 pPhysAddress,
										 &ulMemType1,
										 &pPhysAddressStart);

	bTranslate2 = HalTranslateBusAddress(Isa,
										 0,
										 pPhysAddressEnd,
										 &ulMemType2,
										 &pPhysAddressEnd);



	pVirtAddressLen   = RtlLargeIntegerSubtract(pPhysAddressEnd,pPhysAddressStart);
	ulDataLength      = pVirtAddressLen.LowPart;
	pVirtAddressStart = pPhysAddressStart;
	pToVirtMappedMem  = NULL;

	uMode   = (uAccess&LPT_WRITE)? PAGE_READWRITE:PAGE_READONLY;
	uStatus = ZwMapViewOfSection( hHandle,
								  INVALID_HANDLE_VALUE,
								  &pToVirtMappedMem,
								  0L,
								  ulDataLength,
								  &pVirtAddressStart,
								  &ulDataLength,
								  ViewShare,
								  0,
								  uMode | PAGE_NOCACHE);

	if(!NT_SUCCESS(uStatus))
		{
		ZwClose(hHandle);
		return uStatus;
		}

	(UCHAR*)pToVirtMappedMem += (ULONG)pPhysAddressStart.LowPart - (ULONG)pVirtAddressStart.LowPart;

	*((PVOID*)pIoBuffer) = pToVirtMappedMem;

    pIrp->IoStatus.Information = sizeof(pToVirtMappedMem);


	ZwClose(hHandle);





return STATUS_SUCCESS;
}


//*****************************************************************************
//*
//*		LptIoctlUnmapMemory
//*
//*****************************************************************************
//
// 	Routine Description:
// 		Unmap's physicaly memory into users address-space
// 	Arguments:
//
// 	    pDevInfo        - our local device data
// 		pIrp			- IO request packet
// 		pStackLoc		- The current stack location
// 		uIoctlCode		- The ioctl code from the IRP
//
// 	Return Value:
// 		STATUS_SUCCESS           -- OK
//
// 		STATUS_INVALID_PARAMETER -- The buffer sent to the driver
// 									was too small to contain the
// 									port, or the buffer which
// 									would be sent back to the driver
// 									was not a multiple of the data size.
//
// 		STATUS_ACCESS_VIOLATION  -- An illegal port number was given.
//


NTSTATUS LptIoctlUnmapMemory(IN PLOCAL_DEVICE_INFO pDevInfo,
							 IN PIRP pIrp,
							 IN PIO_STACK_LOCATION pStackLoc
								)


{
PUCHAR		pAddress;						// NOTE:  Use METHOD_BUFFERED ioctls.
PULONG		pIoBuffer=NULL;					// Pointer to transfer buffer											// (treated as an array of longs).
ULONG		uInBufferSize=0;				// Amount of data avail. from caller.
ULONG		uOutBufferSize=0;				// Max data that caller can accept.
ULONG		uAddress,uAccess,uSize;
NTSTATUS	uStatus;





	KDPRINT(("\nLPT: ioctl unmap data memory !"));

    uInBufferSize  = pStackLoc->Parameters.DeviceIoControl.InputBufferLength;
	uOutBufferSize = pStackLoc->Parameters.DeviceIoControl.OutputBufferLength;
    pIoBuffer      = (PULONG)pIrp->AssociatedIrp.SystemBuffer;

	if(uInBufferSize<sizeof(LPT_MAP_MEM))return STATUS_INVALID_PARAMETER;

	if(uInBufferSize==sizeof(ULONG)*3)
		{
		uAddress = pIoBuffer[0];
		uSize	 = pIoBuffer[1];
		uAccess	 = pIoBuffer[2];
		pAddress = (PUCHAR)uAddress;
		}
	else{
		uAddress = pIoBuffer[0];
		uSize	 = pIoBuffer[1];
		uAccess	 = pIoBuffer[2];
		pAddress = (PUCHAR)uAddress;

		#ifdef __AMD64__
		pAddress+= ((ULONG64)pIoBuffer[2])<<32; 
		#endif 	
		}


	uStatus  = ZwUnmapViewOfSection(INVALID_HANDLE_VALUE,pAddress);

	if(!NT_SUCCESS(uStatus))
		{
		return uStatus;
		}


return STATUS_SUCCESS;
}

//*****************************************************************************
//*
//*		LptUnload
//*
//*****************************************************************************
//
// 	Routine Description:
// 		This routine prepares our driver to be unloaded.  It is responsible
// 		for freeing all resources allocated by DriverEntry as well as any
// 		allocated while the driver was running.  The symbolic link must be
// 		deleted as well.
//
// 	Arguments:
//
// 		pDriverObject - Pointer to driver object created by the system.
//
// 	Return Value:
//
// 		None
//

VOID LptUnload(PDRIVER_OBJECT pDriverObject)


{
PLOCAL_DEVICE_INFO	pDevInfo;
CM_RESOURCE_LIST	sNullResourceList;
BOOLEAN				bResourceConflict;
UNICODE_STRING		sWin32DeviceName;




	KDPRINT(("\nLptUnload() start\n"));

    pDevInfo = (LOCAL_DEVICE_INFO*)(pDriverObject->DeviceObject->DeviceExtension);


	Ke386IoSetAccessProcess(IoGetCurrentProcess(), 0);

	if(pDevInfo->pIoPmLocal)
		{
		MmFreeNonCachedMemory(pDevInfo->pIoPmLocal, sizeof(IOPM));
		pDevInfo->pIoPmLocal=0;
		}


	RtlZeroMemory((PVOID)&sNullResourceList, sizeof(sNullResourceList));

    IoReportResourceUsage(	  NULL,
							  pDriverObject,
							  &sNullResourceList,
							  sizeof(ULONG),
							  NULL,
							  NULL,
							  0,
							  FALSE,
							  &bResourceConflict );



    KDPRINT(("\nLptUnload() 1"));

	RtlInitUnicodeString(&sWin32DeviceName, LPT_DOS_DEVICE_NAME_IO);
    IoDeleteSymbolicLink(&sWin32DeviceName);
    IoDeleteDevice(pDriverObject->DeviceObject);

	KDPRINT(("\nLptUnload() end"));

}





