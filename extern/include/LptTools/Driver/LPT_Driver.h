//*****************************************************************************
//*
//*
//*		LPT_Driver.h
//*
//*
//*****************************************************************************
#include <ntddk.h>
#include <string.h>
#include <devioctl.h>
#include "LPT_Driver_Ioctl.h"



// Basisadressen für ISA-Karte

#define LPT_DEVICE_NAME_IO		L"\\Device\\LPT_Driver"
#define LPT_DOS_DEVICE_NAME_IO	L"\\DosDevices\\LPT_Driver"
#define LPT_MAPMEM_NAME			L"\\Device\\PhysicalMemory"

#define PORT_MEMORY_TYPE		1
#define MEM_MEMORY_TYPE			0





//*****************************************************************************
// IOPM array.
// This holds 8K * 8 bits -> 64K bits of the IOPM
// 0 bit -> enables access
// 1 bit -> disables access for user mode process.

#define	IOPM_SIZE	0x2000
typedef UCHAR IOPM;

// Undocumented kernel functions to manipulate access map.
void Ke386SetIoAccessMap(int, IOPM *);         // Copies the passed map to the TSS.
void Ke386QueryIoAccessMap(int, IOPM *);       // Copies the current map from the TSS.
void Ke386IoSetAccessProcess(PEPROCESS, int);  // Adjusts IOPM offset ptr to newly copied map
                                               // int: 1 enable I/O, 0 disable I/O

typedef struct
	{
	ULONG				uDeviceType;			// Device Type
	PDEVICE_OBJECT		pDeviceObject;			// The Device-Ovject itself
	IOPM			   *pIoPmLocal;				// Local version to be copied to TSS
	}LOCAL_DEVICE_INFO, *PLOCAL_DEVICE_INFO;


/********************* function prototypes ********************/


NTSTATUS    DriverEntry(       	IN  PDRIVER_OBJECT		pDriverObject,
                               	IN  PUNICODE_STRING		pRegistryPath );

NTSTATUS    LptCreateDevice(    IN  PWSTR				pPrototypeName,
								IN  PWSTR				pDosDeviceName,
                               	IN  DEVICE_TYPE			uDeviceType,
                               	IN  PDRIVER_OBJECT		pDriverObject,
                               	OUT PDEVICE_OBJECT	   *ppDevObj     );

NTSTATUS	LptCreate(			IN  PDEVICE_OBJECT		pDO,
								IN  PIRP				pIrp );

NTSTATUS	LptClose(			IN  PDEVICE_OBJECT		pDO,
								IN  PIRP				pIrp );

NTSTATUS    LptDispatch(       	IN  PDEVICE_OBJECT		pDO,
                               	IN  PIRP				pIrp );

NTSTATUS	LptIoctlReadVersion(IN  PLOCAL_DEVICE_INFO	pLDI,
								IN  PIRP				pIrp,
								IN  PIO_STACK_LOCATION	pIrpStack );

NTSTATUS	LptIoctlMapMemory(	IN  PLOCAL_DEVICE_INFO	pLDI,
								IN  PIRP				pIrp,
								IN  PIO_STACK_LOCATION	pIrpStack );

NTSTATUS	LptIoctlUnmapMemory(IN  PLOCAL_DEVICE_INFO	pLDI,
								IN  PIRP				pIrp,
								IN  PIO_STACK_LOCATION	pIrpStack );

NTSTATUS	LptIoctlMapPorts(	IN  PLOCAL_DEVICE_INFO	pLDI,
								IN  PIRP				pIrp,
								IN  PIO_STACK_LOCATION	pIrpStack );

NTSTATUS	LptIoctlUnmapPorts(	IN  PLOCAL_DEVICE_INFO	pLDI,
								IN  PIRP				pIrp,
								IN  PIO_STACK_LOCATION	pIrpStack );

VOID        LptUnload(			IN  PDRIVER_OBJECT		pDriverObject  );

NTSTATUS	LptFreeDevice(		IN  PDRIVER_OBJECT		pDriverObject  );











