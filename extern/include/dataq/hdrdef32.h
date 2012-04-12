#define ANHM 6 /* maximum chars for annotation in header */
#define VWIN 32 /* max # of windows in VGA CODAS header */
#define HB 4 /* data file byte index of hdrcmn structure */
#define THERM1 0x8000 /* special value of dacoff for thermocouple type 1 */
#define DIFFR 0x4000 /* bit in dacoff for differential in expanded header */
#define DGINP 0x2000 /* bit in dacoff for digital input in expanded header */
#define NONLN 0x1000 /* bit in dacoff for nonlinear nonthermocouple channel */
#define LSTPT 0x800 /* bit in dacoff for last point instead of combining */
#define CRULE 0x700 /* bits in dacoff for acquisition method */
#define CRFRQ 0x400 /* CRULE value for frequency */
#define CRRMS 0x300 /* CRULE value for RMS */
#define CRMIN 0x200 /* CRULE value for minimum */
#define CRMAX 0x100 /* CRULE value for maximum */
#define CRAVG 0x000 /* CRULE value for average */
#define TCTYP 0xE0 /* thermocouple bits if THERM1 and NONLN both set */
#define MVFSE 0x2 /* mvfscd field extension bit */
#define GAINE 0x1 /* gaincd field extension bit */

typedef struct winfmt { /* screen window format */
   unsigned short omax:8; /* max # of overlapping waveforms/window */
   unsigned short tmax:2; /* max # of time windows per data window */
   unsigned short gvsp:6; /* grid vertical spacing in pixels */
   unsigned char dmax; /* max # of data windows (affects scale) */
   unsigned char orient; /* modes message number to acquire for WinDaq/ECM,
      2 for DI-7002 16x16 channel selection grid, 3 for DI-1000TC */
} WINFMT;

typedef struct chinfo { /* information about each channel in file */
   float fscale; /* # of data windows from - to + full scale A/D */
   float foffset; /* part of - to + full scale A/D to add to data */
   double slope; /* slope converts 14-bit data to engr units */
   double intrcpt; /* engr units intercept for data value of 0 */
   char an[ANHM]; /* engineering unit strings, 6 chars max */
   unsigned char nxtchn; /* next logical channel to scan */
   unsigned char rdgppt; /* # readings averaged per file data pt */
   unsigned short pchn: 8; /* phys chn # (1-31), bit 6 = differential */
   unsigned short gaincd: 4; /* gain code for 1,2,5,10,50,100,500,1000,4,8,20,
      200,10000,100000,40,80. If .dacoff GAINE is set, 16,32,64,128,256,512
      and room for 10 more */
   unsigned short mvfscd: 4; /* mV full scale code for 5000,10000,2500,2048,
      1280,500000,1000000,%FS and MS bit set for unipolar. If .dacoff MVFSE is
      set, 64000, 20000 and room for 6 more. */
   unsigned short dacoff; /* D/A converter offset or bits defined above */
} CHINFO;

typedef struct hdrcmn {
   unsigned char chnbeg; /* file byte index of start of charray */
   unsigned char chnsiz; /* # of bytes per charray entry */
   short hdrsiz; /* header bytes = 1156 for WFS codas compatiblilty */
   unsigned long datbyt; /* # data bytes before trailer */
   unsigned long evtbyt; /* # event marker & time stamp bytes */
   unsigned short anfbyt; /* # of acquisition annotation function bytes */
   short fvres; /* height of full-screen window in lines */
   short fhres; /* width of full-screen window in pixels */
   short curpix; /* cursor position relative to center of screen */
   struct winfmt vidfmt; /* video window format */
   double intrvl; /* time between samples on a channel */
   unsigned long begtime; /* time acquistion opened file, sec since 1/1/70 */
   unsigned long endtime; /* time when file trailer was written */
   long compress; /* compression factor */
   long curpnt; /* cursor position */
   long mrkpnt; /* time marker position */
   unsigned short pretrig; /* pretrigger points */
   unsigned short postrig; /* postrigger points */
   short xycleft; /* left cursor limit for xy */
   short xycright; /* right cursor limit for xy */
   unsigned char fkeys; /*
      bit 0 F1 on
      bit 1 F2 on
      bit 2 F3 on
      bit 3 F4 on
      bit 4 F5 on
      bit 5 ALT-F2 baseline
      bit 6 scroll lock on
      bit 7 event markers on
      */
   unsigned char lmkeys; /*
      bit  0   ALT-G key grids enabled
      bit  1   U key user annotation enabled
      bits 2-3 L key, 0=none, 1=limits&frq, 2=acqasg&limits&frq, 3=acqasg
      bits 4-5 state of M key selection, 0=mid, 1=min, 2=max, 3=max/min
      bits 6-7 00=normal file,01=1st pass,02=wrapped circ,03=circ acq done
      */
   char chnnm; /* selected window number - 1, -1 for none */
   unsigned char pwkeys; /*
      bits 0-3 state of P key selection
      bits 4-5 state of T key, 0=filename,1=start time,2=end time,3=time/div
      bit 6 set if last scroll direction was reverse
      bit 7 W set for window-oriented scrolling
      */
   unsigned char vc[VWIN]; /* chan - 1 for window = o + omax*(t + tmax*d) */
   unsigned short reserved: 1; /* for future use */
   unsigned short data16: 1; /* file contains 16-bit data & all marks in trailer */
   unsigned short tctyp:2; /* thermocouple type, 0=K, 1=J, 2=T, 3=R */
   unsigned short trigmux: 4; /* multiplexer number for trigger channel */
   unsigned short autotrig: 1; /* oscilloscope mode auto triggers at screen end */
   unsigned short pchan0: 1; /* lowest physical channel is number 0 instead of 1 */
   unsigned short f3sel: 2; /* live display 0=TBF,1=DATA-TM,2=%CHANGE,3=SLOPETM */
   unsigned short f4sel: 2; /* marker display type 0=SEC, 1=SMP, 2=Hz, 3=C/M */
   unsigned short packed: 1; /* file is packed using .rdgppt fields */
   unsigned short f4mag: 1; /* set to display magnitude instead of decibels */
   unsigned short fftfmin: 13; /* lowest frequency index for fft display */
   unsigned short fftwpwr2: 1; /* set for windowed power-of-2 fft */
   unsigned short wintyp: 2; /* data window 0=none,1=hanning,2=Hamming,3=Blackman */
   unsigned short frqavp2: 4; /* fftfrqav + 2 or 4 - fftxzoom */
   unsigned short frqwin: 4; /* size - 1 of moving window for frequency averaging */
   unsigned short trigchan: 4; /* trigger channel source */
   unsigned short erasebar: 1; /* 1 for erase bar on */
   unsigned short dispmode: 2; /* 0=scroll,1=freeze,2=triggered sweep,3=freeze */
   unsigned short trgslope: 1; /* 1 for negative trigger slope */
   short trglevel: 16; /* trig level, bit0=1 trgrd mode, bit1=1 trgrd storage */
   unsigned short xystripe: 6; /* # of 1/16 screen stripes used for xy */
   unsigned short curnum: 2; /* 0=data cursror, 1=left limit, 2=right limit */
   unsigned short hysteres: 4; /* trigger hysteresis,bits 2-0 AT-CODSAS pen width */
   unsigned short remevenb: 1; /* remote events enabled */
   unsigned short remstenb: 1; /* remote storage enabled */
   unsigned short remevneg: 1; /* remote events triggered on - slope */
   unsigned short remstneg: 1; /* remote storage triggered on - slope */
} HDRCMN;
