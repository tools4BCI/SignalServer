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
//	Diese Datei enth�lt Funktionen f�r den Zugriff auf die paralelle
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
//		LptPortOut(0,2,i&~0x01);	// Strobe auf LPT1 l�schen
//	  i=LptPortIn (0,1);			// Diverse Bits einlesen
//		...
//
//		LptExit();					// Treiber deinitialisieren
//

#include <sys/perm.h>
#include <stdio.h>
#include "LptTools.h"

#define		TRUE			1
#define		FALSE			0
#define 	MAX_LPT_PORTS	4


inline void outp( unsigned short port , unsigned char value )
	{
	__asm__ __volatile__ ( "outb %b0,%w1" : : "a" (value), "d" (port) );
	}

	inline unsigned char inp(unsigned short port)
	{
	unsigned char  vv;

	__asm__ __volatile__ ( "inb %w1,%b0" : "=a" (vv) : "d" (port) );

	return vv;
	}

static	unsigned short	wPortAddr[MAX_LPT_PORTS]={0x378,0x278,0x38C};
static	unsigned		bIsInit=FALSE;


//*****************************************************************************
//*
//*		LptDriverInstall
//*
//*****************************************************************************
//	Installiert den Treiber
//	Unter Linux ist das nur ein Dummy Funktion
//	Ergibt TRUE wenn der Treiber installiert wurde.
int LptDriverInstall()
{
	return TRUE;
}

//*****************************************************************************
//*
//*		LptDriverRemove
//*
//*****************************************************************************
//	Erntfernt ein Service und den Treiber
//	Unter Linux ist das nur ein Dummy Funktion
//	Ergibt TRUE wenn der Treiber entfernt wurde.
int LptDriverRemove()
{
	return TRUE;
}

//*****************************************************************************
//*
//*		LptUnmapPorts
//*
//*****************************************************************************/
//	Ausblenden von IO-Ports in den User-Process
//	uPorts	: Erste Port-Adresse
//	uSize	: Anzahl der einzublendenten Ports
//	Ergibt TRUE wenn die Ports eingeblentet wurden.
int LptUnmapPorts(unsigned  uPort,unsigned uSize)
{
int	bOk;

	bOk=ioperm(uPort,uSize,0);

return (bOk==-1)? FALSE:TRUE;
}


//*****************************************************************************
//*
//*		LptMapPorts
//*
//*****************************************************************************/
//	Einblenden von IO-Ports in den User-Process
//	uPorts	: Erste Port-Adresse
//	uSize	: Anzahl der einzublendenten Ports
//	Ergibt TRUE wenn die Ports eingeblentet wurden.
// int LptUnmapPorts(unsigned  uPort,unsigned uSize)
// {
// 
// 	bOk=ioperm(uPort,uSize,1);
// 
// return (bOk==-1)? FALSE:TRUE;
// }


//*****************************************************************************
//*
//*		LptInit
//*
//*****************************************************************************
//	Initialisiert den LPT-Port Treiber
int	LptInit()
{
int i;


	if(bIsInit)return FALSE;

	bIsInit=1;

   	for(i=0;i<MAX_LPT_PORTS;i++)
		{
		if(!wPortAddr[i])continue;
		//if(ioperm(wPortAddr[i]      ,8,1))<0)return FALSE;
		//if(ioperm(wPortAddr[i]+0x400,8,1))<0)return FALSE;

		if(iopl(3)<0)return FALSE;
		}




return TRUE;
}


//*****************************************************************************
//*
//*		LptExit
//*
//*****************************************************************************
int	LptExit()
{
int i;


	if(!bIsInit)return FALSE;

   	for(i=0;i<MAX_LPT_PORTS;i++)
		{
		if(!wPortAddr[i])continue;
		ioperm(wPortAddr[i]      ,8,0);
		ioperm(wPortAddr[i]+0x400,8,0);
		}


   bIsInit=0;

return TRUE;
}


//*****************************************************************************
//*
//*		LptPort
//*
//*****************************************************************************
//	Ergibt die Port-Adresse einer LPT-Schnittstelle bzw. 0 wenn die
//	Schnittstelle nicht vorhanden oder nicht initaliesiert ist.
//	Nr	: Ist die Nummer der LPT-Schnittstelle (0=LPT1,1=LPT2,...)
int	LptPort(unsigned Nr)
{

	if(Nr>=MAX_LPT_PORTS)return 0;


return 	wPortAddr[Nr];
}

//*****************************************************************************
//*
//*		LptPortIn
//*
//*****************************************************************************
//	Ergibt den Wert eines Ports bzw. -1 wenn die
//	Schnittstelle nicht vorhanden oder nicht initaliesiert ist.
//	Nr		: Ist die Nummer der LPT-Schnittstelle (0=LPT1,1=LPT2,...)
//	Port	: Ist der Port-Offset (0-4)
int	LptPortIn(unsigned Nr,unsigned Port)
{
int iVal,iPort;

	if(Nr>=MAX_LPT_PORTS)return -1;
	if(Port>=4)
		{
		if(Port< 0x400)return -1;
		if(Port>=0x404)return -1;
		}

		iPort=wPortAddr[Nr];
	if(!iPort)return -1;
		iPort+=Port;


	iVal=inp(iPort);


return iVal;
}

//*****************************************************************************
//*
//*		LptPortOut
//*
//*****************************************************************************
//	Nr		: Ist die Nummer der LPT-Schnittstelle (0=LPT1,1=LPT2,...)
//	Port	: Ist der Port-Offset (0-4)
//	Ergibt 0 wenn der Wert des Ports gespeichert wurde bzw. -1 wenn die
//	Schnittstelle nicht vorhanden oder nicht initaliesiert ist.
int	LptPortOut(unsigned Nr,unsigned Port,unsigned Data)
{
int iPort;


	if(Nr>=MAX_LPT_PORTS)return -1;
	if(Port>=4)
		{
		if(Port< 0x400)return -1;
		if(Port>=0x404)return -1;
		}

		iPort=wPortAddr[Nr];
	if(!iPort)return -1;
		iPort+=Port;

	outp(iPort,Data);


return 0;
}

//*****************************************************************************
//*
//*		LptDetectPorts
//*
//*****************************************************************************
//	Speichert in FLPTCount die Anzahl der gefundenen LPT Ports
//	In FLPTAddress werden die Portadressen gespeichert.
//		FLPTAddress[0] f�r LPT1
//		FLPTAddress[1] f�r LPT2
//		...
//	FLPTMaxPorts ist die Anzahl der Ports die gescannt werden.
void  LptDetectPorts(int &FLPTCount,unsigned short *FLPTAddress,int FLPTMaxPorts)
{


	FLPTCount=3;
	if(FLPTMaxPorts>=1)FLPTAddress[0]=wPortAddr[0];
	if(FLPTMaxPorts>=2)FLPTAddress[1]=wPortAddr[1];
	if(FLPTMaxPorts>=3)FLPTAddress[2]=wPortAddr[2];

	if(FLPTCount>FLPTMaxPorts)FLPTCount=FLPTMaxPorts;

}






