//*****************************************************************************
//*
//*		
//*		LptTools.h
//*
//*
//*****************************************************************************
#ifndef 	__LPT_TOOLS_H__
#define 	__LPT_TOOLS_H__

#ifndef	__cplusplus
#error	Die Datei "LptTools.h" darf nur in C++ verwendet werden !!!
#endif 	


	void	LptDetectPorts(int &iCount,unsigned short *pAddress,int iMaxPorts,unsigned short *pAddressEx=0,unsigned *pLength=0,unsigned *pLengthEx=0);
	int		LptInit();												// Treiber initialisieren
	int		LptExit();												// Treiber deinitialisieren
	int		LptPort   (unsigned Nr);								// Portadresse abfragen
	int		LptPortIn (unsigned Nr,unsigned Port);					// Port-Eingabe
	int		LptPortOut(unsigned Nr,unsigned Port,unsigned Data);	// Port-Ausgabe
	
	int		LptDriverRemove();										// Treiber deinstallieren
	int		LptDriverInstall();										// Treiber installieren

	int		LptMapPorts  (unsigned  uPort,unsigned uSize);
	int		LptUnmapPorts(unsigned  uPort,unsigned uSize);



#endif 	


