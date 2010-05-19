//*****************************************************************************
//*
//*		
//*		LptDemo
//*		
//*
//*****************************************************************************
//
//	Das ist ein Demo-Program für die LptTools
//
//

#include	<conio.h>
#include	<stdio.h>
#include	<string.h>
#include	"LptTools.h"

char *names[]={ "D0" ,"D1","D2","D3","D4","D5","D6","D7",
				"???","???","???","Error","Select","Paper out","Acknowlge","Busy",
				"Strobe","Autofeed","Initial","Sel.Input","IRQ free","???","???","???"
			  };

//*****************************************************************************
//*
//*		main
//*
//*****************************************************************************
int main()
{
int				i,j,key,bit,port,value[3];
unsigned short 	addr[16];


	LptDetectPorts(i,addr,3);

	printf("\n\n  Gefundene LPT-Ports:\n\n");


	for(j=0;j<i;j++)
		{
		if(!addr[j])continue;
		printf("\tLPT%i at %04Xh\n",j+1,addr[j]);
		}
	

	printf("\n\n  LPT Port waehlen '1' bis '9'  ESC=Abbruch");

	for(;;) 
		{
		   key=getch();
		if(key==27)return -1;
		if(key>='1' && key<='9')
			{
			port=key-'1';
			break;
			}
		}

	if(addr[port]==0)
		{
		printf("\n\n  Das Port ist nicht vorhanden !\n");
		getch();
		return -1;
		}

	if(!LptInit())
		{
		printf("\n\n  Der Treiber ist nicht installiert !\n");
		getch();
		return -1;
		}

	printf("\n\n  Weiter mit einem Tastendruck.....\n\n");
	getch();	


	i=LptPortIn (port,2);					// EEP abschalten
	  LptPortOut(port,2,i&0x1F);		
	  

	for(;;) 
		{
		printf("\n\n\n\n");
		printf("\n  Umschalten der Leitungen:\n");
		printf("\n    Tasten '0' bis '7' fuer die Leitungen D0 bis D7");
		printf("\n    Taste  'S' fuer die Leitung Strobe");
		printf("\n    Taste  'A' fuer die Leitung Autofeed");
		printf("\n    Taste  'I' fuer die Leitung Inital");
		printf("\n    Taste  'L' fuer die Leitung Sel.Input");

		printf("\n\n       Bit\t");    

		for(i=0;i<3;i++)
			{
			printf("     %03Xh\t",addr[port]+i);
			value[i]=LptPortIn(port,i);
			}

		printf("\n");    

		for(bit=0;bit<8;bit++)
			{
			printf("\n\t%i ",bit);

			for(i=0;i<3;i++)
				{
				printf("\t%-9s  %i",names[bit+i*8],(value[i]>>bit)&1);
				}
			}

		printf("\n\n  Abbruch mit ESC\n");

		key=getch();
		if(key==27)break;
		if(key>='0' && key<='7')
			{
			value[0]^=1<<(key-'0');
			LptPortOut(port,0,value[0]);
			}
		if(key=='S' || key=='s'){value[2]^=0x01;LptPortOut(port,2,value[2]);}
		if(key=='A' || key=='a'){value[2]^=0x02;LptPortOut(port,2,value[2]);}
		if(key=='I' || key=='i'){value[2]^=0x04;LptPortOut(port,2,value[2]);}
		if(key=='L' || key=='l'){value[2]^=0x08;LptPortOut(port,2,value[2]);}
		}

	LptExit();

return 0;
}
