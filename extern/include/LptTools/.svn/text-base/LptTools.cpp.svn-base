//*****************************************************************************
//*
//*
//*		LptTools.cpp
//*
//*
//*****************************************************************************
//
//	(C) Copyright Anton Zechner 2007
//
//	Diese Datei enthält Funktionen für den Zugriff auf die paralelle
//	Schnittstelle. Der Zufriff erfolgt nach folgendem Schema:
//
//	  if(!LptInit())				// Treiber initialisieren
//		{
//		printf("Es ist kein Treiber installiert");
//		return -1;
//		}
//
//		LptPortOut(0,0,0xAA);		// 0xAA auf LPT1 ausgeben (D1,D3,D5,D7)
//		LptPortOut(0,0,0x01);		// D0 auf 'high' setzen
//		LptPortOut(0,0,0x02);		// D1 auf 'high' setzen
//	  i=LptPortIn (0,2);			// Diverse Bits einlesen
//		LptPortOut(0,2,i| 0x01);	// Strobe auf LPT1 setzen
//		LptPortOut(0,2,i&~0x01);	// Strobe auf LPT1 löschen
//	  i=LptPortIn (0,1);			// Diverse Bits einlesen
//		...
//
//		LptExit();					// Treiber deinitialisieren
//
//	Anmerkung:	Unter NT/Win200/WinXP funktioniert die automatische
//				Treiber-Installation nur wenn die Datei LPT_Driver.sys
//				im selben Verzeichnis ist wie die Progammdatei (*.exe)
//				Weiters muss das Programm Administrator-Rechte besitzen
//				um den Treiber installieren zu können.
//				Ist das nicht der Fall muss die Datei LPT_Driver.sys
//				von Hand ins Windows-System-Verzeichnis kopiert werden.
//				(C:\WinNT\system32\drivers)
//
//				Zum Installiern des Treibers kann die Funktion
//				LptDriverInstall() benutzt werden, zum Deinstalliern
//				ist LptDriverRemove() zu verwenden. Beide Funktionen
//				benötigen Administrator-Rechte. Beim Installieren muss
//				die Datei LPT_Driver.sys im selben Verzeichnis ist wie
//				die Progammdatei (*.exe) sein.



#include	<windows.h>
#include	<memory.h>
#include	"LptTools.h"

#define		MAX_LPT_PORTS				4



static WORD			wPortAddr  [MAX_LPT_PORTS];
static WORD			wPortAddrEx[MAX_LPT_PORTS];
static unsigned		uPortSize  [MAX_LPT_PORTS];
static unsigned		uPortSizeEx[MAX_LPT_PORTS];
static BOOL			bIsWinNT	= FALSE;
static BOOL			bIsInit     = FALSE;
static BOOL			bUseService = FALSE;
static SC_HANDLE	hSCManager  = NULL;
static HANDLE		hLpt        = INVALID_HANDLE_VALUE;


#ifndef		MAX_PATH					
#define		MAX_PATH					256
#endif


static void LptDetectPorts9x(int &iLptCount,unsigned short *pLptAddress,int iLptMaxPorts,unsigned short *pLptAddressEx,unsigned *pLptLength,unsigned *pLptLengthEx);
static void LptDetectPortsNT(int &iLptCount,unsigned short *pLptAddress,int iLptMaxPorts,unsigned short *pLptAddressEx,unsigned *pLptLength,unsigned *pLptLengthEx);

#ifndef		CTL_CODE					
#define		CTL_CODE(DeviceType,Function,Method,Access)	(((DeviceType)<<16)|((Access)<<14)|((Function)<<2)|(Method))
#endif

// The IOCTL function codes from 0x800 to 0xFFF are for customer use.
#define		IOCTL_LPT_MAP_PORT			CTL_CODE( 40000, 0x0A02, 0, 0x0000)
#define		IOCTL_LPT_UNMAP_PORT		CTL_CODE( 40000, 0x0A04, 0, 0x0000)

#define		DRIVER_NAME 				"LPT_Driver"

typedef struct
	{
	ULONG	uPort;
	ULONG	uSize;
	}LPT_MAP_PORT;



//*****************************************************************************
//*
//*		LptServiceCreate
//*
//*****************************************************************************
// 	Erstellen Laden des Treibers als Service
// 	Ergibt TRUE wenn das Service erstellt wurde.
static BOOL LptServiceCreate()
{
SC_HANDLE	hSCService;
char		cPath[MAX_PATH];
int			iPsOsVersion;



	if(!hSCManager)return FALSE;

	iPsOsVersion=GetSystemDirectory(cPath,sizeof(cPath)-24);
	strcpy(cPath+iPsOsVersion,"\\Drivers\\" DRIVER_NAME ".sys");



	hSCService=CreateService(hSCManager ,						// handle to service control manager
							 DRIVER_NAME,						// pointer to name of service to start
							 DRIVER_NAME,						// pointer to display name
							 SERVICE_START | SERVICE_STOP | DELETE | SERVICE_QUERY_STATUS,	// type of access to service
							 SERVICE_KERNEL_DRIVER, 			// type of service
							 SERVICE_SYSTEM_START,				// when to start service
							 SERVICE_ERROR_NORMAL,				// severity if service fails to start
							 cPath, 							// pointer to name of binary file
							 NULL,								// pointer to name of load ordering
							 NULL,								// pointer to variable to get tag identifier
							 NULL,								// pointer to array of dependency names
							 NULL,								// pointer to account name of service
							 NULL								// pointer to password for service account
							 );

	if(!hSCService)return FALSE;

	CloseServiceHandle(hSCService);


return TRUE;
}

//*****************************************************************************
//*
//*		LptService
//*
//*****************************************************************************
// 	Laden des Treibers als Service
// 	Ergibt TRUE wenn das Service geladen wurde.
static BOOL LptService(BOOL bForceCopy=FALSE)
{
static int			iSet=0;
char				cBuffer1[MAX_PATH];
char				cBuffer2[MAX_PATH];
HANDLE				hFind=INVALID_HANDLE_VALUE;
SC_HANDLE			hSCService;
SERVICE_STATUS		sStatus;
WIN32_FIND_DATA		sFind;
int					iLen;


	bUseService=FALSE;

															// Prüfe ob der Treiber im Windows-System-Verzeichnis ist
	iLen=GetSystemDirectory(cBuffer1,sizeof(cBuffer1));
	strncpy(cBuffer1+iLen,"\\Drivers\\" DRIVER_NAME ".sys",sizeof(cBuffer1)-iLen);
	if(!bForceCopy)hFind=FindFirstFile(cBuffer1,&sFind);

	if(hFind==INVALID_HANDLE_VALUE) 							// Treiber neu kopieren
		{
		iLen=GetModuleFileName(GetModuleHandle(0),cBuffer2,sizeof(cBuffer2));
		for(;iLen>0;iLen--)
			{
			if(cBuffer2[iLen]=='\\')break;
			}

		strncpy(cBuffer2+iLen,"\\" DRIVER_NAME ".sys",sizeof(cBuffer2)-iLen);
		CopyFile(cBuffer2,cBuffer1,FALSE);
		}
	else{
		FindClose(hFind);
		}


	if(!hSCManager)
		{
																// Nur mit Administrator Rechten
		hSCManager	   = OpenSCManager(NULL,NULL,	SC_MANAGER_CONNECT |
									   SC_MANAGER_QUERY_LOCK_STATUS |
									   SC_MANAGER_ENUMERATE_SERVICE |
									   SC_MANAGER_CREATE_SERVICE);

																// Wenn keine Administrator Rechte neu versuchen
		if(!hSCManager && GetLastError()==ERROR_ACCESS_DENIED)
			{
			hSCManager = OpenSCManager(NULL,NULL,	SC_MANAGER_CONNECT |
									   SC_MANAGER_QUERY_LOCK_STATUS |
									   SC_MANAGER_ENUMERATE_SERVICE);
			}
		}

	if(!hSCManager)return FALSE;

		hSCService = OpenService(hSCManager,DRIVER_NAME, SERVICE_QUERY_STATUS);
	if(!hSCService)
		{
		LptServiceCreate();
		hSCService = OpenService(hSCManager,DRIVER_NAME, SERVICE_QUERY_STATUS);
		}

	if(!hSCService) 											// Wurde das Service geladen
		{
		CloseServiceHandle(hSCManager);
		hSCManager=NULL;
		return FALSE;
		}

	if(QueryServiceStatus(hSCService,&sStatus)) 				// Ist das Service gestartet
	if(sStatus.dwCurrentState==SERVICE_RUNNING)
		{
		CloseServiceHandle(hSCService);
		bUseService=TRUE;
		return TRUE;
		}

	CloseServiceHandle(hSCService);
	hSCService = OpenService(hSCManager,DRIVER_NAME, SERVICE_START);
	if(hSCService && StartService(hSCService,0,0))				// Service starten
		{
		CloseServiceHandle(hSCService);
		bUseService=TRUE;
		return TRUE;
		}


	if(hSCService)CloseServiceHandle(hSCService);
	CloseServiceHandle(hSCManager);
	hSCManager =NULL;

	if(iSet==0)
		{
		iSet=1;
		LptDriverInstall();
		iSet=0;
		}


return FALSE;
}

//*****************************************************************************
//*
//*		LptDriverInstall
//*
//*****************************************************************************
// 	Installiert den Treiber
// 	Ergibt TRUE wenn der Treiber installiert wurde.
int LptDriverInstall()
{
BOOL	bCloseScm=FALSE;
BOOL	bOk;
HKEY	hKey;



	LptDriverRemove();

	if(!hSCManager)												// Nur mit Administrator Rechten
		{
		hSCManager	   = OpenSCManager(NULL,NULL,	SC_MANAGER_CONNECT |
									   SC_MANAGER_QUERY_LOCK_STATUS |
									   SC_MANAGER_ENUMERATE_SERVICE |
									   SC_MANAGER_CREATE_SERVICE);

		bCloseScm=TRUE;
		}

	bOk=LptService(TRUE);										// Treiber starten

	if(bCloseScm)
		{
		CloseServiceHandle(hSCManager);
		hSCManager=NULL;
		}


	RegOpenKeyEx(HKEY_LOCAL_MACHINE,"SYSTEM\\CurrentControlSet\\Services\\" DRIVER_NAME,0,KEY_ALL_ACCESS,&hKey);
	RegDeleteValue(hKey,"DeleteFlag");
	RegCloseKey(hKey);



return bOk;
}

//*****************************************************************************
//*
//*		LptDriverRemove
//*
//*****************************************************************************
// 	Erntfernt ein Service und den Treiber
// 	Ergibt TRUE wenn der Treiber entfernt wurde.
int LptDriverRemove()
{
SC_HANDLE	hSCService;
BOOL		bCloseScm=FALSE;
BOOL		bOk=TRUE;


	if(!hSCManager)
		{
														// Nur mit Administrator Rechten
		hSCManager	   = OpenSCManager(NULL,NULL,	SC_MANAGER_CONNECT |
									   SC_MANAGER_QUERY_LOCK_STATUS |
									   SC_MANAGER_ENUMERATE_SERVICE |
									   SC_MANAGER_CREATE_SERVICE);

														// Wenn keine Administrator Rechte neuveruchen
		if(!hSCManager && GetLastError()==ERROR_ACCESS_DENIED)	
			{
			hSCManager = OpenSCManager(NULL,NULL,	SC_MANAGER_CONNECT |
									   SC_MANAGER_QUERY_LOCK_STATUS |
									   SC_MANAGER_ENUMERATE_SERVICE);
			}

		bCloseScm=TRUE;
		}

	if(!hSCManager)return FALSE;

	hSCService = OpenService(hSCManager,DRIVER_NAME,DELETE);

	if(hSCService)
		{
		if(!DeleteService(hSCService))bOk=FALSE;
		CloseServiceHandle(hSCService);
		}
	else{
		bOk=0;
		}

	/*
	int  iPsOsVersion;
	char cPath[MAX_PATH];
	iPsOsVersion=GetSystemDirectory(cPath,sizeof(cPath)-24);
	strcpy(cPath+iPsOsVersion,"\\Drivers\\" DRIVER_NAME ".sys");
	if(!DeleteFile(cPath))bOk=0;
	*/

	if(bCloseScm)
		{
		CloseServiceHandle(hSCManager);
		hSCManager=NULL;
		}


return bOk;
}

//*****************************************************************************
//*
//*		LptOpen
//*
//*****************************************************************************
// 	Öffnen des Treibers
// 	Ergibt TRUE wenn der Treiber geladen wurde.
static BOOL LptOpen()
{


	if(hLpt!=INVALID_HANDLE_VALUE)return FALSE;


	hLpt = CreateFile(	"\\\\.\\" DRIVER_NAME,
					  GENERIC_READ,
					  0,
					  NULL,
					  OPEN_EXISTING,
					  FILE_ATTRIBUTE_NORMAL,
					  NULL
					  );


	if(hLpt==INVALID_HANDLE_VALUE)								// versuche als service zu laden
		{
		if(!LptService())return FALSE;

		hLpt = CreateFile(	"\\\\.\\" DRIVER_NAME,
						  GENERIC_READ,
						  0,
						  NULL,
						  OPEN_EXISTING,
						  FILE_ATTRIBUTE_NORMAL,
						  NULL
						  );


		if(hLpt==INVALID_HANDLE_VALUE)
			{
			return FALSE;
			}
		}



return TRUE;
}

//*****************************************************************************
//*
//*		LptClose
//*
//*****************************************************************************
// 	Schließen des Treibers
// 	Ergibt TRUE wenn der Treiber geschlsOsVersionsen wurde.
// 
static BOOL LptClose()
{
BOOL	bOk;



	if(hSCManager)
		{
		bOk=CloseServiceHandle(hSCManager);
		hSCManager=0;
		}

	if(hLpt==INVALID_HANDLE_VALUE)return FALSE;


	bOk  = CloseHandle(hLpt);
	hLpt = INVALID_HANDLE_VALUE;


return bOk;
}

//*****************************************************************************
//*
//*		LptMapPorts
//*
//*****************************************************************************
// 	Einblenden von IO-Ports in den User-Process
// 	uPorts	: Erste Port-Adresse
// 	uSize	: Anzahl der einzublendenten Ports
// 	Ergibt TRUE wenn die Ports eingeblentet wurden.
int LptMapPorts(unsigned  uPort,unsigned uSize)
{
BOOL			bOk;
DWORD			dwLenght;
LPT_MAP_PORT	sMapPort;



	if(hLpt==INVALID_HANDLE_VALUE)
		{
		return FALSE;
		}

	sMapPort.uPort=uPort;
	sMapPort.uSize=uSize;

	bOk = DeviceIoControl(hLpt,									// Handle to device
						  IOCTL_LPT_MAP_PORT,					// IO Control code for Write
						  &sMapPort,							// Buffer to driver.  Holds port & data.
						  sizeof(LPT_MAP_PORT), 				// Length of buffer in bytes.
						  NULL, 								// Buffer from driver.	 Not used.
						  0,									// Length of buffer in bytes.
						  &dwLenght,							// Bytes placed in outbuf.	Should be 0.
						  NULL									// NULL means wait till I/O completes.
						  );


return bOk;
}

//*****************************************************************************
//*
//*		LptUnmapPorts
//*
//*****************************************************************************
// 	Ausblenden von IO-Ports in den User-Process
// 	uPorts	: Erste Port-Adresse
// 	uSize	: Anzahl der einzublendenten Ports
// 	Ergibt TRUE wenn die Ports eingeblentet wurden.
int LptUnmapPorts(unsigned	uPort,unsigned uSize)
{
BOOL			bOk;
DWORD			dwLenght;
LPT_MAP_PORT	sMapPort;



	if(hLpt==INVALID_HANDLE_VALUE)
		{
		return FALSE;
		}

	sMapPort.uPort=uPort;
	sMapPort.uSize=uSize;

	bOk = DeviceIoControl(hLpt,									// Handle to device
						  IOCTL_LPT_UNMAP_PORT	,				// IO Control code for Write
						  &sMapPort,							// Buffer to driver.  Holds port & data.
						  sizeof(LPT_MAP_PORT), 				// Length of buffer in bytes.
						  NULL, 								// Buffer from driver.	 Not used.
						  0,									// Length of buffer in bytes.
						  &dwLenght,							// Bytes placed in outbuf.	Should be 0.
						  NULL									// NULL means wait till I/O completes.
						  );


return bOk;
}


//*****************************************************************************
//*
//*		LptInit
//*
//*****************************************************************************
// 	Initialisiert den LPT-Port Treiber
int LptInit()
{
OSVERSIONINFO	sOS;
int				i,iCount;


	if(bIsInit)return 0;

	memset(&sOS, NULL, sizeof(OSVERSIONINFO));
	sOS.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&sOS);
	bIsWinNT=(sOS.dwPlatformId==VER_PLATFORM_WIN32_NT);

	LptDetectPorts(iCount,wPortAddr,MAX_LPT_PORTS,wPortAddrEx,uPortSize,uPortSizeEx);

	if(bIsWinNT)
		{
		if(!LptOpen())
			{
			Sleep(50);

			if(!LptOpen())								// Zeiter Versuch
				{
				memset(wPortAddr,0,sizeof(wPortAddr));
				return 0;
				}
			}
		
		for(i=0;i<MAX_LPT_PORTS;i++)
			{
			if(!wPortAddr  [i])continue;
			if( uPortSize  [i])LptMapPorts(wPortAddr  [i],uPortSize  [i]);
			if( uPortSizeEx[i])LptMapPorts(wPortAddrEx[i],uPortSizeEx[i]);
			}
		}

	bIsInit=TRUE;


return 1;
}


//*****************************************************************************
//*
//*		LptExit
//*
//*****************************************************************************
// 	Deitialisiert den LPT-Port Treiber
int LptExit()
{
int		i;


	if(!bIsInit)return 0;

	if(bIsWinNT)
		{
		for(i=0;i<MAX_LPT_PORTS;i++)
			{
			if(!wPortAddr[i])continue;
			LptUnmapPorts(wPortAddr[i]	 	 ,4);
			LptUnmapPorts(wPortAddr[i]+0x0400,4);
			}
		LptClose();
		}

	bIsInit=FALSE;


return 1;
}


//*****************************************************************************
//*
//*		LptPort
//*
//*****************************************************************************
// 	Ergibt die Port-Adresse einer LPT-Schnittstelle bzw. 0 wenn die
// 	Schnittstelle nicht vorhanden oder nicht initaliesiert ist.
// 	Nr	: Ist die Nummer der LPT-Schnittstelle (0=LPT1,1=LPT2,...)
int LptPort(unsigned Nr)
{

	if(Nr>=MAX_LPT_PORTS)return 0;



return	wPortAddr[Nr];
}

//*****************************************************************************
//*
//*		LptPortIn
//*
//*****************************************************************************
// 	Ergibt den Wert eines Ports bzw. -1 wenn die
// 	Schnittstelle nicht vorhanden oder nicht initaliesiert ist.
// 	uNum		: Ist die Nummer der LPT-Schnittstelle (0=LPT1,1=LPT2,...)
// 	uPort		: Ist der Port-Offset (0-4)
int LptPortIn(unsigned uNum,unsigned uPort)
{
int			iVal,iPort;
unsigned 	uOffsetEx;



	if(uNum>=MAX_LPT_PORTS)return -1;

	if(uPort>=uPortSize[uNum])
		{
		if(!wPortAddrEx[uNum] || !uPortSizeEx[uNum])return -1;

		uOffsetEx  = wPortAddrEx[uNum];
		uOffsetEx -= wPortAddr  [uNum];

		if(uPort<uOffsetEx || uPort>=uOffsetEx+uPortSizeEx[uNum])
			{
			if(uPort<0x400 || uPort>=0x400    +uPortSizeEx[uNum])
				{
				return -1;
				}

			uPort -= 0x400+wPortAddr[uNum]-wPortAddrEx[uNum];
			}
		}

		iPort=wPortAddr[uNum];
	if(!iPort)return -1;


	iPort += uPort;


	__asm mov	edx,iPort;
	__asm xor	eax,eax;
	__asm in	al,dx;
	__asm mov	iVal,eax;



return iVal;
}

//*****************************************************************************
//*
//*		LptPortOut
//*
//*****************************************************************************
//	Schreibt ein Byte zu einem LPT-Port
// 	uNum 	: Ist die Nummer der LPT-Schnittstelle (0=LPT1,1=LPT2,...)
// 	uPort	: Ist der Port-Offset (0-4)
// 	Ergibt 0 wenn der Wert des Ports gespeichert wurde bzw. -1 wenn die
// 	Schnittstelle nicht vorhanden oder nicht initaliesiert ist.
int LptPortOut(unsigned uNum,unsigned uPort,unsigned uData)
{
int			iPort;
unsigned 	uOffsetEx;



	if(uNum>=MAX_LPT_PORTS)return -1;
	if(uPort>=uPortSize[uNum])
		{
		if(!wPortAddrEx[uNum] || !uPortSizeEx[uNum])return -1;

		uOffsetEx  = wPortAddrEx[uNum];
		uOffsetEx -= wPortAddr  [uNum];

		if(uPort<uOffsetEx || uPort>=uOffsetEx+uPortSizeEx[uNum])
			{
			if(uPort<0x400 || uPort>=0x400    +uPortSizeEx[uNum])
				{
				return -1;
				}

			uPort -= 0x400+wPortAddr[uNum]-wPortAddrEx[uNum];
			}
		}

		iPort=wPortAddr[uNum];
	if(!iPort)return -1;


	iPort+=uPort;

	__asm mov	edx,iPort;
	__asm mov	eax,uData;
	__asm out	dx,al;



return 0;
}

//*****************************************************************************
//*
//*		LptDetectPorts
//*
//*****************************************************************************
// 	Speichert sucht nach allen Installierten LPT Ports
//	iLptCount		: Hier wird die Anzahl der gefundenen LPT Ports gespeichert
// 	pLptAddress		: Hier werden die Portadressen gespeichert.
// 							pLptAddress[0] für LPT1
// 							pLptAddress[1] für LPT2
// 	iLptMaxPorts	: Ist die Anzahl der Ports die gescannt werden.
// 	pLptAddressEx	: Hier werden die EPP-Portadressen gespeichert
//	pLptLength		: Hier wird die Größe des LPT-Port-Adressraums gespeichert
//	pLptLengthEx	: Hier wird die Größe des EPP-Port-Adressraums gespeichert
void  LptDetectPorts(int &iLptCount,unsigned short *pLptAddress,int iLptMaxPorts,unsigned short *pLptAddressEx,unsigned *pLptLength,unsigned *pLptLengthEx)
{
OSVERSIONINFO	sOsVersion;
BOOL			bRunningWinNT;
unsigned short	wAddrEx  [16];
unsigned		uLength  [16];
unsigned 		uLengthEx[16];


	if(iLptMaxPorts<=0)
		{
		iLptCount = 0;
		return;
		}


	if(!pLptAddressEx)
		{
		pLptAddressEx = wAddrEx;
		if(iLptMaxPorts>16)iLptMaxPorts=16;
		}
	
	if(!pLptLength)
		{
		pLptLength = uLength;
		if(iLptMaxPorts>16)iLptMaxPorts=16;
		}
	
	if(!pLptLengthEx)
		{
		pLptLengthEx = uLengthEx;
		if(iLptMaxPorts>16)iLptMaxPorts=16;
		}

	memset(pLptAddress  ,0,sizeof(*pLptAddress  )*iLptMaxPorts);
	memset(pLptAddressEx,0,sizeof(*pLptAddressEx)*iLptMaxPorts);
	memset(pLptLength   ,0,sizeof(*pLptLength   )*iLptMaxPorts);
	memset(pLptLengthEx ,0,sizeof(*pLptLengthEx )*iLptMaxPorts);


	memset(&sOsVersion,NULL,sizeof(OSVERSIONINFO));
	sOsVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&sOsVersion);
	bRunningWinNT=(sOsVersion.dwPlatformId==VER_PLATFORM_WIN32_NT);

	
	if(bRunningWinNT)
			LptDetectPortsNT(iLptCount,pLptAddress,iLptMaxPorts,pLptAddressEx,pLptLength,pLptLengthEx);	// WinNT version
	else	LptDetectPorts9x(iLptCount,pLptAddress,iLptMaxPorts,pLptAddressEx,pLptLength,pLptLengthEx);	// Win9x version

}



//*****************************************************************************
//*
//*		LptDetectPorts9x
//*
//*****************************************************************************
void LptDetectPorts9x(int &iLptCount,unsigned short *pLptAddress,int iLptMaxPorts,unsigned short *pLptAddressEx,unsigned *pLptLength,unsigned *pLptLengthEx)
{
HKEY			hKey; 										
unsigned		uPos;
FILETIME		sDummyFileTime;
WORD			wAllocation[64];
char			cHardwareSubKey[MAX_PATH];
char			cPortNumberStr [MAX_PATH];
char			cPortName      [MAX_PATH];
char			cKeyName       [MAX_PATH];	
char		  **pKeyList;			
BYTE		   *pData;
DWORD			dwIndex;
DWORD			dwKeyIndex;
DWORD			dwKeyCount; 		
DWORD			dwDataType;
DWORD			dwDataSize;
DWORD			dwDummyLength;
bool			bHasProblem;
int				iPortNumber;
int				iOsVersion;
int				i;
								
					
								

					


	dwDummyLength = MAX_PATH;	
	dwKeyCount	  = 0;
	iLptCount	  = 0;											// Clear the port count

	
	for(i=0;i<iLptMaxPorts; i++)								// Clear the port array
		{
		pLptAddress[i] = 0;
		}

	RegOpenKeyEx(HKEY_DYN_DATA,"Config Manager\\Enum",0,KEY_ENUMERATE_SUB_KEYS|KEY_QUERY_VALUE,&hKey);

	
	dwDummyLength = MAX_PATH;
	dwKeyCount    = 0;
	
	while(RegEnumKeyEx(hKey,dwKeyCount++,cKeyName,&dwDummyLength,NULL,NULL,NULL,&sDummyFileTime)!=ERROR_NO_MORE_ITEMS)
		{
		dwDummyLength = MAX_PATH;
		}

	pKeyList		= new char*[dwKeyCount];
	dwDummyLength	= MAX_PATH;
	dwKeyCount		= 0;
	
	while(RegEnumKeyEx(hKey,dwKeyCount,cKeyName,&dwDummyLength,NULL,NULL,NULL,&sDummyFileTime)!=ERROR_NO_MORE_ITEMS)
		{
		pKeyList[dwKeyCount] = new char[dwDummyLength+1];
		strcpy(pKeyList[dwKeyCount], cKeyName);
		dwDummyLength = MAX_PATH;
		dwKeyCount++;
		}

	
	RegCloseKey(hKey);

	// Cycle through all keys; looking for a string valued subkey called
	// 'HardWareKey' which is not NULL, and another subkey called 'Problem'
	// whsOsVersione fields are all valued 0.
	
	for(dwKeyIndex=0; dwKeyIndex<dwKeyCount; dwKeyIndex++)
		{
		bHasProblem = false;								// Is 'Problem' non-zero? Assume it is Ok

		
		strcpy(cKeyName, "Config Manager\\Enum");			// Open the key
		strcat(cKeyName, "\\");
		strcat(cKeyName, pKeyList[dwKeyIndex]);

		if(RegOpenKeyEx(HKEY_DYN_DATA, cKeyName, 0, KEY_ENUMERATE_SUB_KEYS|KEY_QUERY_VALUE, &hKey) != ERROR_SUCCESS)
			{
			continue;
			}

		// Test for a 0 valued Problem sub-key,
		// which must only consist of raw data
		
		RegQueryValueEx(hKey, "Problem", NULL, &dwDataType, NULL, &dwDataSize);

		if(dwDataType == REG_BINARY)
			{
																// We have a valid, binary "Problem" sub-key
																// Test to see if the fields are zero

			pData = new BYTE[dwDataSize];

																// Read the data from the "Problem" sub-key

			if(RegQueryValueEx(hKey,"Problem",NULL,NULL,pData,&dwDataSize)==ERROR_SUCCESS)
				{
				for(dwIndex=0;dwIndex<dwDataSize;dwIndex++)		// See if it has any problems
					{
					if(pData[dwIndex])bHasProblem=1;
					}
				}
			else{
				bHasProblem = true;								// No good
				}

			delete pData;

			
			dwDataSize = MAX_PATH;								// Now try and read the Hardware sub-key

			RegQueryValueEx(hKey,"HardwareKey",NULL,&dwDataType,(unsigned char*)cHardwareSubKey,&dwDataSize);
			
			if(dwDataType != REG_SZ)
				{
				bHasProblem = true;								// No good
				}

			
			if(!bHasProblem && strlen(cHardwareSubKey) > 0)		// Do we have no problem, and a non-null Hardware sub-key?
				{
				
				RegCloseKey(hKey);								// Now open the key which is "pointed at" by cHardwareSubKey

				strcpy(cKeyName, "Enum\\");
				strcat(cKeyName, cHardwareSubKey);
				
				if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, cKeyName, 0, KEY_ENUMERATE_SUB_KEYS|KEY_QUERY_VALUE, &hKey) != ERROR_SUCCESS)
					{
					continue;
					}

				
				dwDataSize = MAX_PATH;
				RegQueryValueEx(hKey,"PortName",NULL,&dwDataType,(unsigned char*)cPortName,&dwDataSize);
				
				if(dwDataType != REG_SZ)
					{
					strcpy(cPortName, "");						// No good
					}

				
				if(strstr(cPortName,"LPT")!=NULL)				// Make sure it has LPT in it
					{
																// Holds the registry data for the port address allocation

					memset(cPortNumberStr, '\0', MAX_PATH);
					strncpy(cPortNumberStr,
							strstr(cPortName, "LPT")+3,
							strlen(cPortName)-(strstr(cPortName, "LPT")-cPortName)-2);

																// Find the port number
					iPortNumber = atoi(cPortNumberStr)-1;


																// Find the address
					RegCloseKey(hKey);

					strcpy(cKeyName, "Config Manager\\Enum");
					strcat(cKeyName, "\\");
					strcat(cKeyName, pKeyList[dwKeyIndex]);
					RegOpenKeyEx(HKEY_DYN_DATA, cKeyName, 0, KEY_ENUMERATE_SUB_KEYS|KEY_QUERY_VALUE, &hKey);

					dwDataSize = sizeof(wAllocation);
					RegQueryValueEx(hKey,"Allocation",NULL,&dwDataType,(unsigned char*)wAllocation,&dwDataSize);
					
					if(dwDataType == REG_BINARY)
						{
																// Decode the Allocation data: the port address is present
																// directly after a 0x000C entry (which doesn't have 0x0000
																// after it).
						for(iOsVersion=0;iOsVersion<63;iOsVersion++)
							{
							if(wAllocation[iOsVersion]==0x000C && wAllocation[iOsVersion+1] != 0x0000 && iPortNumber<iLptMaxPorts && iPortNumber>=0)
								{
																// Found a port; add it to the list
								pLptAddress  [iPortNumber] = wAllocation[iOsVersion+1];
								pLptAddressEx[iPortNumber] = wAllocation[iOsVersion+1]+0x400;
								pLptLength	 [iPortNumber] = 8;
								pLptLengthEx [iPortNumber] = 8;
								iLptCount++;
								}
							
							break;
							}
						}
					}
				}
			}

		RegCloseKey(hKey);
		}

	
	for(uPos=0;uPos<dwKeyCount;uPos++)							// Destroy our key list	
		{
		delete pKeyList[uPos];
		}
	
	delete pKeyList;
}



//*****************************************************************************
//*
//*		LptDetectPortsNT
//*
//*****************************************************************************
void LptDetectPortsNT(int &iLptCount,unsigned short *pLptAddress,int iLptMaxPorts,unsigned short *pLptAddressEx,unsigned *pLptLength,unsigned *pLptLengthEx)
{
HKEY		hKey;												// Current key when using the registry
DWORD		dwVal;
DWORD		dwDataSize;
DWORD		dwValueType;
DWORD		dwValueCount;										// Count of the number of value names in ValueList
DWORD		dwDummyLength;
char		cKeyName[MAX_PATH]; 								// A key name when using the registry
char		cValue[64];
BYTE	   *pData;
char	   *pStr;
char	   *pLptPortList[10];
unsigned	uScanPort[4];
unsigned	uScanLen [4];
unsigned	uScanNum;
unsigned	uIndex;
unsigned	uPort;
unsigned	uPos;
unsigned	uLen;
unsigned	uPsOsVersion;
int			iLptIndex;
int			iFound;
int			i;





	
	for(uIndex=0;uIndex<10;uIndex++)
		{
		pLptPortList[uIndex] = 0;
		}

	iFound = 0;

																// Open the registry
	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,"SYSTEM\\CurrentControlSet\\Services\\Parport\\Enum",0,KEY_ENUMERATE_SUB_KEYS|KEY_QUERY_VALUE,&hKey) == ERROR_SUCCESS)
		{
		dwDummyLength = MAX_PATH;
		dwValueCount  = 0;

		while(RegEnumValue(hKey,dwValueCount,cKeyName,&dwDummyLength,NULL,&dwValueType,NULL,NULL)!=ERROR_NO_MORE_ITEMS)
			{
			dwDummyLength = MAX_PATH;

			RegQueryValueEx(hKey, cKeyName, NULL, &dwValueType, NULL, &dwDataSize);
		
			if(dwValueType==REG_SZ && cKeyName[0]>='0' && cKeyName[0]<='9')
				{
				i = cKeyName[0] - '0';
				pLptPortList[i] = new char[dwDataSize+64];
				pData           = new BYTE[dwDataSize   ];
				RegQueryValueEx(hKey,cKeyName,NULL,&dwValueType,pData,&dwDataSize);
				strcpy(pLptPortList[i], "SYSTEM\\CurrentControlSet\\Enum\\");
				strcat(pLptPortList[i], (char*)pData);
				iFound++;
				delete pData;
				}
			else{
				if(dwValueType == REG_DWORD && strcmp(cKeyName, "Count") == 0 && dwDataSize == 4)
					{
					RegQueryValueEx(hKey,cKeyName,NULL,&dwValueType,(BYTE*)&dwVal,&dwDataSize);
					iLptCount = dwVal;
					}
				}

			dwValueCount++;
			}

		
		RegCloseKey(hKey);
		}


//*****************************************************************************
																// Open the registry
	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,"SYSTEM\\CurrentControlSet\\Enum",0,KEY_ENUMERATE_SUB_KEYS|KEY_QUERY_VALUE,&hKey) == ERROR_SUCCESS)
		{
		char	cTemp[MAX_PATH*3];
		DWORD	dwCount0;
		DWORD	dwCount1;
		DWORD	dwCount2;
		HKEY	hKey0;
		HKEY	hKey1;
		HKEY	hKey2;
		HKEY	hKey3;
		int		iPos0;
		int		iPos1;
		int		iPos2;
		
		strcpy(cTemp,"SYSTEM\\CurrentControlSet\\Enum\\");
		iPos0		  = strlen(cTemp);
		dwDummyLength = MAX_PATH;
		dwCount0      = 0;


		while(RegEnumKey(hKey,dwCount0,cTemp+iPos0,sizeof(cTemp)-iPos0)!=ERROR_NO_MORE_ITEMS)
			{
			dwCount0++;

			if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,cTemp,0,KEY_ENUMERATE_SUB_KEYS|KEY_QUERY_VALUE,&hKey0) != ERROR_SUCCESS)
				{
				continue;
				}

			iPos1		 = strlen(cTemp);
			cTemp[iPos1] = '\\';
			dwCount1     = 0;
			iPos1++;
			
			while(RegEnumKey(hKey0,dwCount1,cTemp+iPos1,sizeof(cTemp)-iPos1)!=ERROR_NO_MORE_ITEMS)
				{
				dwCount1++;

				if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,cTemp,0,KEY_ENUMERATE_SUB_KEYS|KEY_QUERY_VALUE,&hKey1) != ERROR_SUCCESS)
					{
					continue;
					}

				iPos2		 = strlen(cTemp);
				cTemp[iPos2] = '\\';
				dwCount2     = 0;
				iPos2++;
				
				while(RegEnumKey(hKey1,dwCount2,cTemp+iPos2,sizeof(cTemp)-iPos2)!=ERROR_NO_MORE_ITEMS)
					{
					dwCount2++;	

					if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,cTemp,0,KEY_ENUMERATE_SUB_KEYS|KEY_QUERY_VALUE,&hKey2) != ERROR_SUCCESS)
						{
						continue;
						}
					
					dwDataSize  = sizeof(cValue);	
					dwValueType = REG_SZ;
					cValue[0]   = 0;

					RegQueryValueEx(hKey2,"Class",NULL,&dwValueType,(BYTE*)cValue,&dwDataSize);
					


					if(dwValueType!=REG_SZ || stricmp(cValue,"Ports"))
						{
						RegCloseKey(hKey2);
						continue;
						}

				
					if(RegOpenKeyEx(hKey2,"Device Parameters",0,KEY_ENUMERATE_SUB_KEYS|KEY_QUERY_VALUE,&hKey3) != ERROR_SUCCESS)
						{
						RegCloseKey(hKey2);
						continue;
						}

					dwDataSize  = sizeof(cValue);	
					dwValueType = REG_SZ;
					cValue[0]	= 0;

					RegQueryValueEx(hKey3,"PortName",NULL,&dwValueType,(BYTE*)cValue,&dwDataSize);
					RegCloseKey(hKey3);
					RegCloseKey(hKey2);

					if(dwValueType!=REG_SZ)
						{
						continue;
						}

					if(strnicmp(cValue,"LPT",3))
						{
						continue;
						}

					   uIndex = atoi(cValue+3);
					if(uIndex==0 || uIndex>9)continue;

					uIndex--;
					if(pLptPortList[uIndex])continue;
					
					pLptPortList[uIndex] = new char[strlen(cTemp)+64];
					pLptPortList[uIndex];
					strcpy(pLptPortList[uIndex],cTemp);
					iFound++;
					}
				
				RegCloseKey(hKey1);
				}
			
			RegCloseKey(hKey0);
			}

		
		RegCloseKey(hKey);
		}

//*****************************************************************************

	if(!iFound)return;

	for(uIndex=0;uIndex<10;uIndex++)
		{
		if(!pLptPortList[uIndex])continue;

		pStr      =  0;
		iLptIndex = -1;

		       pStr= new char[strlen(pLptPortList[uIndex]) + 20];
		strcpy(pStr, pLptPortList[uIndex]);
		strcat(pStr, "\\Device Parameters");

		if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,pStr,0,KEY_ENUMERATE_SUB_KEYS|KEY_QUERY_VALUE,&hKey)!=ERROR_SUCCESS)
			{
			delete pStr;
			continue;
			}
		
		delete pStr;
		
		pStr		  = 0;
		dwValueCount  = 0;
		dwDummyLength = MAX_PATH;

		while(RegEnumValue(hKey,dwValueCount,cKeyName,&dwDummyLength,NULL,&dwValueType,NULL,NULL)!=ERROR_NO_MORE_ITEMS)
			{
			dwDummyLength = MAX_PATH;

			RegQueryValueEx(hKey,cKeyName,NULL,&dwValueType,NULL,&dwDataSize);

			if(dwValueType==REG_SZ && !strcmp(cKeyName,"PortName"))
				{
				pData = new BYTE[dwDataSize];
				RegQueryValueEx(hKey, cKeyName, NULL, &dwValueType, pData, &dwDataSize);

				if(strlen((char*)pData)==4 && !strncmp("LPT",(char*)pData,3))
					{
					iLptIndex = pData[3] - '0';
					}

				delete pData;
				}

			dwValueCount++;
			}

		RegCloseKey(hKey);
		if(iLptIndex<0 || iLptIndex>iLptMaxPorts)continue;

			   pStr = new char[strlen(pLptPortList[uIndex]) + 20];
		strcpy(pStr, pLptPortList[uIndex]);
		strcat(pStr, "\\Control");

		if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,pStr,0,KEY_ENUMERATE_SUB_KEYS|KEY_QUERY_VALUE,&hKey)!=ERROR_SUCCESS)
			{
			delete pStr;
			continue;
			}
		else{
			delete pStr;
			pStr = 0;
			}

		iFound		  = 0;
		dwValueCount  = 0;
		dwDummyLength = MAX_PATH;

		while(RegEnumValue(hKey,dwValueCount,cKeyName,&dwDummyLength,NULL,&dwValueType,NULL,NULL)!=ERROR_NO_MORE_ITEMS)
			{
			dwDummyLength = MAX_PATH;

			RegQueryValueEx(hKey,cKeyName,NULL,&dwValueType,NULL,&dwDataSize);
			
			/* 
			if ( dwValueType == REG_SZ && strcmp(cKeyName, "ActiveService") == 0 )
			{
				pData = new BYTE[dwDataSize];

				RegQueryValueEx(hKey, cKeyName, NULL, &dwValueType, pData, &dwDataSize);
				if (strcmp((char*)pData,"Parport")==0)
					{
					}
				
				delete pData;
			}
			else
			*/

			if(dwValueType==REG_RESOURCE_LIST && !strcmp(cKeyName,"AllocConfig"))
				{
				pData  = new BYTE[dwDataSize];

				if(RegQueryValueEx(hKey,cKeyName,NULL,&dwValueType,(BYTE*)pData,&dwDataSize)!=ERROR_SUCCESS)
					{
					delete pData;
					continue;
					}

				uScanNum = 0;

				for(uPsOsVersion=4;uPsOsVersion<dwDataSize;uPsOsVersion+=16)
					{
					if(pData[uPsOsVersion]==1)				// Is it a port address
						{
						uPort = *(unsigned*)(pData+uPsOsVersion+ 4);
						uLen  = *(unsigned*)(pData+uPsOsVersion+12);

						if(uPort<0x00100             )continue;
						if(uPort>0x10000 || (uPort&3))continue;
						if(uLen >8		 || uLen<4 	 )continue;
						if(uScanNum >= 4 ){uScanNum++;continue;}

						uScanPort[uScanNum] = uPort;
						uScanLen [uScanNum] = uLen;
						uScanNum++;
						}
					}

				
								
				if(uScanNum>=2)							
					{
					/*
					for(uPos=1;uPos<uScanNum;uPos++)		// Sortiere die Einträge
						{
						if(uScanPort[uPos]>=uScanPort[uPos-1])continue;
						
						uPort=uScanPort[uPos-1];uScanPort[uPos-1]=uScanPort[uPos];uScanPort[uPos]=uPort;
						uLen =uScanLen [uPos-1];uScanLen [uPos-1]=uScanLen [uPos];uScanLen [uPos]=uLen ;
						uPos =0;
						}
					*/					

					for(uPos=0;uPos<uScanNum;uPos++)		// Suche Standard Adressen
						{
						if(uScanPort[uPos]!=0x378)
						if(uScanPort[uPos]!=0x278)
						if(uScanPort[uPos]!=0x38C)continue;

						if(uPos+1<uScanNum && uScanPort[uPos+1]-uScanPort[uPos]==0x400)
							{
							uScanNum=uPos+2;
							break;
							}

						if(uPos+2<uScanNum && uScanPort[uPos+2]-uScanPort[uPos]==0x400)
							{
							uScanPort[uPos+1]=uScanPort[uPos+2];
							uScanLen [uPos+1]=uScanLen [uPos+2];
							uScanNum=uPos+2;
							break;
							}

						if(uPos+3<uScanNum && uScanPort[uPos+3]-uScanPort[uPos]==0x400)
							{
							uScanPort[uPos+1]=uScanPort[uPos+3];
							uScanLen [uPos+1]=uScanLen [uPos+3];
							uScanNum=uPos+2;
							break;
							}

						if(uPos>=1 && uScanPort[uPos-1]-uScanPort[uPos]==0x400)
							{
							uPort=uScanPort[uPos-1];uScanPort[uPos-1]=uScanPort[uPos];uScanPort[uPos]=uPort;
							uLen =uScanLen [uPos-1];uScanLen [uPos-1]=uScanLen [uPos];uScanLen [uPos]=uLen ;
							uScanNum=uPos+1;
							uPos--;
							break;
							}

						if(uPos>=2 && uScanPort[uPos-2]-uScanPort[uPos]==0x400)
							{
							uScanPort[uPos-1]=uScanPort[uPos  ];
							uScanLen [uPos-1]=uScanLen [uPos  ];
							uScanPort[uPos  ]=uScanPort[uPos-2];
							uScanLen [uPos  ]=uScanLen [uPos-2];
							uScanNum=uPos+1;
							uPos--;
							break;
							}

						if(uPos>=3 && uScanPort[uPos-3]-uScanPort[uPos]==0x400)
							{
							uScanPort[uPos-1]=uScanPort[uPos  ];
							uScanLen [uPos-1]=uScanLen [uPos  ];
							uScanPort[uPos  ]=uScanPort[uPos-3];
							uScanLen [uPos  ]=uScanLen [uPos-3];
							uScanNum=uPos+1;
							uPos--;
							break;
							}

						uScanNum=1;
						break;
						}
					
					if(uPos>=uScanNum)						// Suche 0x400 Abstand
						{					
						for(uPos=1;uPos<uScanNum;uPos++)		
							{
							if(uScanPort[uPos]-uScanPort[uPos-1]!=0x400)continue;
							uScanNum=uPos+1;
							uPos--;
							break;
							}
						}
					
					if(uPos>=uScanNum)						// Suche 0x400 Abstand
						{					
						for(uPos=2;uPos<uScanNum;uPos++)		
							{
							if(uScanPort[uPos]-uScanPort[uPos-2]!=0x400)continue;
							uScanPort[uPos-1] =uScanPort[uPos];
							uScanLen [uPos-1] =uScanLen [uPos];
							uScanNum=uPos;
							uPos-=2;
							break;
							}
						}

					if(uPos>=uScanNum)						// Suche 0x400 Abstand
						{					
						for(uPos=3;uPos<uScanNum;uPos++)		
							{
							if(uScanPort[uPos]-uScanPort[uPos-3]!=0x400)continue;
							uScanPort[uPos-2] =uScanPort[uPos];
							uScanLen [uPos-2] =uScanLen [uPos];
							uScanNum=uPos-1;
							uPos-=3;
							break;
							}
						}

					if(uPos>=uScanNum)						// Benutze die erste Adresse
						{
						uPos=0;
						}
					}
				else{
					uPos=0;
					}
				
				uScanNum -= uPos;
						
				if(uScanNum>=2 && uScanPort[uPos]<0x400 &&  uScanPort[uPos+1]-uScanPort[uPos]!=0x400)
					{
					uScanNum=1;
					}
				
				if(uScanNum>=2)
					{
					if(iLptCount<iLptIndex)iLptCount=iLptIndex;

					pLptAddress  [iLptIndex-1] = uScanPort[uPos+0];
					pLptLength   [iLptIndex-1] = uScanLen [uPos+0];
					pLptAddressEx[iLptIndex-1] = uScanPort[uPos+1];
					pLptLengthEx [iLptIndex-1] = uScanLen [uPos+1];

					iFound = 1;
					}
				else if(uScanNum)
					{
					if(iLptCount<iLptIndex)iLptCount=iLptIndex;

					pLptAddress  [iLptIndex-1] = uScanPort[uPos];
					pLptLength   [iLptIndex-1] = uScanLen [uPos];
					pLptAddressEx[iLptIndex-1] = uScanPort[uPos]+0x400;
					pLptLengthEx [iLptIndex-1] = 0;

					iFound = 1;
					}

				delete pData;
				}

			dwValueCount++;
			}

		RegCloseKey(hKey);
															// Set Default-Values
		if(!iFound && iLptIndex<=3)
			{
			if(iLptIndex==1){uPort=0x0378;uLen=8;}
			if(iLptIndex==2){uPort=0x0278;uLen=8;}
			if(iLptIndex==3){uPort=0x038C;uLen=4;}

			if(iLptCount<iLptIndex)iLptCount=iLptIndex;

			pLptAddress  [iLptIndex-1] = uPort;
			pLptLength   [iLptIndex-1] = uLen;
			pLptAddressEx[iLptIndex-1] = uPort+0x400;
			pLptLengthEx [iLptIndex-1] = uLen;
			}
		}

	
	for(uIndex=0;uIndex<10;uIndex++)						// Destroy KeyList
		{
		if(pLptPortList[uIndex] != 0)
			{
			delete pLptPortList[uIndex];
			}
		}
}
 