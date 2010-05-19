//*****************************************************************************
//*
//*
//*		LPT_Driver_Ioctl.h
//*
//*
//*****************************************************************************
#ifndef 	__LPT_DRIVER_IOCTRL_H__
#define 	__LPT_DRIVER_IOCTRL_H__



#ifndef CTL_CODE
#define CTL_CODE(DeviceType,Function,Method,Access) (((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method))
#endif 	

#ifndef METHOD_BUFFERED                	
#define METHOD_BUFFERED		0
#endif 	

#ifndef FILE_ANY_ACCESS		
#define FILE_ANY_ACCESS		0x0000
#endif 	

#ifndef FILE_READ_ACCESS		
#define FILE_READ_ACCESS	0x0001
#endif 	

#ifndef FILE_WRITE_ACCESS		
#define FILE_WRITE_ACCESS	0x0002
#endif 	


// Device type           -- in the "User Defined" range."
#define LPT_TYPE_IO		40000


// Types for uAccess in LPT_MAP_MEM
#define LPT_READ		0x0001
#define LPT_WRITE		0x0002
#define LPT_ALL			0x0003

// The IOCTL function codes from 0x800 to 0xFFF are for customer use.

#define IOCTL_LPT_READ_VERSION	CTL_CODE( LPT_TYPE_IO, 0x0A00, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_LPT_MAP_MEM		CTL_CODE( LPT_TYPE_IO, 0x0A01, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_LPT_MAP_MEM64		CTL_CODE( LPT_TYPE_IO, 0x0A05, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_LPT_MAP_PORT		CTL_CODE( LPT_TYPE_IO, 0x0A02, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_LPT_UNMAP_MEM		CTL_CODE( LPT_TYPE_IO, 0x0A03, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_LPT_UNMAP_PORT	CTL_CODE( LPT_TYPE_IO, 0x0A04, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_LPT_UNMAP_MEM64	CTL_CODE( LPT_TYPE_IO, 0x0A06, METHOD_BUFFERED, FILE_ANY_ACCESS)


/******************** Strukturen für IOCTRL **********************************/

#ifdef 	__cplusplus
extern "C"	{
#endif 	


typedef struct
	{
	ULONG	uAddress;
	ULONG	uSize;
	ULONG	uAccess;
	}LPT_MAP_MEM;

typedef struct
	{
	ULONG	uPort;
	ULONG	uSize;
	}LPT_MAP_PORT;

typedef	struct
	{
	UCHAR	uMajor;	
	UCHAR	uMinor;
	USHORT	wReserve;
	CHAR	cName[60];
	CHAR	cLinkTime[32];
	}LPT_VERSION;

#ifdef 	__cplusplus
}
#endif 	

#endif 	