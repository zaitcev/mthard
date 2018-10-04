/**
 ** PC video driver (via BIOS)
 **/
#ifndef _MY_DOS_
#  include <DOS.H>
#  define _MY_DOS_
#endif
#include <IO.H>        /* write() */
#include <PROCESS.h>   /* _exit() */
#include "MTPROTO.H"
#include "DISPCTRL.H"

/* Video services */
#define _VIDEO_SETMODE     0x00 /* Set video mode */
#define _VIDEO_SETPOS      0x02 /* Set cursor position */
#define _VIDEO_ACTPAGE     0x05 /* Select active display page */
#define _VIDEO_UPSCROLL    0x06 /* Scroll window */
#define _VIDEO_DOWNSCROLL  0x07 /* Reverse scroll for a window */
#define _VIDEO_READ        0x08 /* read character/attribute at cursor*/
#define _VIDEO_WRITE       0x09 /* Write  - - - -  */

 /* video attributes */
#define _ATTR_HIGH  0x08
#define _ATTR_BLINK 0x80
#define _ATTR_RED   0x04
#define _ATTR_GREEN 0x02
#define _ATTR_BLUE  0x01
#define FLIP_ATTR(a) ((((a)&0x07)<<4) | (((a)&0x70)>>4) | ((a)&0x88))

#define DFL_ATTR (_ATTR_BLUE|_ATTR_GREEN|_ATTR_RED)

#define XDIM   80
#define YDIM   Y_dim
#define ER_Y_STEP  4
#define RL_STEP    3

#define DELC  0xDB          /* MISS "DEL" White rectangle */

static char MISS_to_PCalt[ 256 ] = {
    /* 00 */ 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
             0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
             0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
             0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
             0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
             0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
             0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
             0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
    /* 40 */ 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
             0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
             0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
             0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
             0x9E, 0x80, 0x81, 0x96, 0x84, 0x85, 0x94, 0x83,
             0x95, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E,
             0x8F, 0x9F, 0x90, 0x91, 0x92, 0x93, 0x86, 0x82,
             0x9C, 0x9B, 0x87, 0x98, 0x9D, 0x99, 0x97, DELC,
    /* 80 */ 0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7,
             0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
             0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,
             0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
             0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7,
             0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
             0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,
             0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF,
    /* C0 */ 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
             0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
             0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
             0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
             0xEE, 0xA0, 0xA1, 0xE6, 0xA4, 0xA5, 0xE4, 0xA3,
             0xE5, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE,
             0xAF, 0xEF, 0xE0, 0xE1, 0xE2, 0xE3, 0xA6, 0xA2,
             0xEC, 0xEB, 0xA7, 0xE8, 0xED, 0xE9, 0xE7, 0xEA
};

static int Ycurr, Xcurr;    /* Cursor position in the display window */
static int CuAttr;          /* PC video attributes */
#define CuPage  0     /*SCNDR.ASM requirement*/
/**static int CuPage;**/          /* Current video page */
static int Y_dim;           /* Hight of the screen */

static char SaveAttr;       /* DOS attributes */
static int Old_maxmode;     /* DOS maxmode */
static int SaveMode;        /* Fn 0x00,0x0F mode of video */

static int SimpleFlag;      /* Don't use 0x80 in 0x00 Fn of INT 10h */

void ExtPict()
{
    Serase( YDIM, YDIM, 0, XDIM-1, _ATTR_GREEN<<4 );
    edpos( 3 );
    edout( "MISS terminal" );
}


#define SetPos(y,x) Spos(y,x)
#if 0
static void SetPos( y, x )
    int y, x;
{
    union REGS rint;

    rint.h.ah = _VIDEO_SETPOS;
    rint.h.bh = CuPage;
    rint.h.dh = y;    rint.h.dl = x;
    int10( &rint );

    Input();
}
#endif

#if 0
static void Serase( yup, ydown, xleft, xright, attr )
    int yup, ydown, xleft, xright, attr;
{
    union REGS rint;

    rint.h.ah = _VIDEO_UPSCROLL;
    rint.h.ch = yup;      rint.h.cl = xleft;
    rint.h.dh = ydown;    rint.h.dl = xright;
    rint.h.al = 0;      /* Pseudo scroll for erasing */
    rint.h.bh = attr;
    int10( &rint );
}
#endif

void InsertChar( int ybase, int xbase );
static void InsertChar( ybase, xbase )
    int ybase, xbase;
{
    int i;
    unsigned cprev,c0;     /* With attribute */
    auto union REGS rint;

    cprev = (CuAttr<<8)+' ';
    for( i = xbase; i < XDIM; i++ ){
        SetPos( ybase, i );
        rint.h.ah = _VIDEO_READ;
	rint.h.bh = CuPage;
        int10( &rint );
        Input();
        if( (c0 = rint.x.ax) != cprev ){
            Swrite( cprev, cprev>>8 );
	}
        cprev = c0;
    }
}

void DeleteChar( int, int );
static void DeleteChar( ybase, xbase )
    int ybase, xbase;
{
    int i;
    unsigned cprev,c0;     /* With attribute */
    auto union REGS rint;

    cprev = (CuAttr<<8)+' ';
    for( i = XDIM; --i >= xbase; ){
        SetPos( ybase, i );
        rint.h.ah = _VIDEO_READ;
	rint.h.bh = CuPage;
        int10( &rint );
        Input();
        if( (c0 = rint.x.ax) != cprev ){
            Swrite( cprev, cprev>>8 );
	}
        cprev = c0;
    }
}

void DeleteLine( int ybase );
static void DeleteLine( ybase )
    int ybase;
{
    union REGS rint;
    register int i, slc;

    slc = 1;   /* Duummy satement (suppress warnings) */
    for( i = ybase; i < YDIM-1; i += slc ){
        slc = RL_STEP;    if( i+slc >= YDIM ) slc = YDIM-1-i;
        rint.h.ah = _VIDEO_UPSCROLL;
        rint.h.ch = i;    rint.h.cl = 0;
        rint.h.dh = i+slc;    rint.h.dl = XDIM-1;
        rint.h.al = 1;                    /* Scroll length */
        rint.h.bh = CuAttr & ~(_ATTR_BLINK);
        int10( &rint );
        Input();
    }
}

void InsertLine( int );
static void InsertLine( ybase )
    int ybase;
{
    union REGS rint;
    
    if( ybase == YDIM-1 ){
        rint.h.ah = _VIDEO_UPSCROLL;
        rint.h.ch = YDIM-1;    rint.h.cl = 0;
        rint.h.dh = YDIM-1;    rint.h.dl = XDIM-1;
        rint.h.al = 0;                /* Erasing */
        rint.h.bh = CuAttr & ~(_ATTR_BLINK);
        int10( &rint );
    }else{
        rint.h.ah = _VIDEO_DOWNSCROLL;
        rint.h.ch = ybase;    rint.h.cl = 0;
        rint.h.dh = YDIM-1;       rint.h.dl = XDIM-1;
        rint.h.al = 1;                    /* Scroll length */
        rint.h.bh = CuAttr & ~(_ATTR_BLINK);
        int10( &rint );
    }
}

void PrintChar( char c );
void PrintChar( c )
    char c;
{

    Swrite( MISS_to_PCalt[ (int)c & 0xFF ], CuAttr );
    Input();
    if( ++Xcurr >= XDIM ){
        Xcurr = 0;
        if( ++Ycurr >= YDIM ){
            Ycurr = YDIM-1;
            DeleteLine( 0 );
        }
    }
    SetPos( Ycurr, Xcurr );
    Input();
}

/*
 * Tick entry
 */
void
Screen()
{
    typedef enum {
        NEUTRAL,        /* Waiting for a chain */
        RYPOS,          /* Waiting for Y position in SP-chain */
        MISSXPOS,       /*             X position (MISSTERM) */
        OLDXPOS,        /*             X position (VDT52-108) */
        MCOUNT,         /*             Repeat count */
        MCHAR,          /*             Char to repeat */
        SATTR,          /* Waiting for screen attributes */
        SCOLOR,         /* Waiting for screen colour */
        RAWCHAR         /* Raw character to display */
    } InputStatus;

    static InputStatus istate = NEUTRAL;
    static int yto, xto;

    static int rpc;  static int rep = 0, hidrep;
    static char attr_ctl;

    /*auto union REGS rint;*/
    register int c0;
    register int i;

    if( rep == 0 ){     /* Nothing to repeat => ready to get next */
        if( (c0 = GetIn()) & 0x100 ) return;

         /* Check for a chain */
        switch( istate ){
        case NEUTRAL:
            if( c0 == SP ){
                istate = RYPOS;
            }else if( c0 == SM ){
                istate = MCOUNT;
            }else if( c0 == SC ){
                istate = SATTR;
            }else if( c0 == ESC ){ 
	        istate = RAWCHAR;
            }else{
                break;
            }
            return;
        case RYPOS:
            istate = MISSXPOS;
            if( (yto = c0 - 0x20) < 0 ){
                if( (yto = c0) >= 0x10 ) yto -= 4;
                istate = OLDXPOS;
            }
            return;
        case MISSXPOS:
            c0 -= 0x20;
        case OLDXPOS:
            xto = c0;
            if( yto >= 0 && yto < YDIM && xto >= 0 && xto < XDIM ){
                SetPos( Ycurr = yto, Xcurr = xto );
            }
            istate = NEUTRAL;
            return;
        case MCOUNT:
            hidrep = c0 & 0xFF;
            istate = MCHAR;
            return;
        case MCHAR:
            rep = hidrep;
            rpc = c0;
            istate = NEUTRAL;
            return;
        case SATTR:
            attr_ctl = c0;
            istate = SCOLOR;
            return;
        case SCOLOR:
            CuAttr = (c0 & 0x77)^0x07; 
            if( attr_ctl & FI_INVRS ) CuAttr = FLIP_ATTR( CuAttr );
            if( attr_ctl & FI_HIGHL ) CuAttr |= _ATTR_HIGH;
            if( attr_ctl & FI_BLINK ) CuAttr |= _ATTR_BLINK;
            istate = NEUTRAL;
            return;
        case RAWCHAR:
	    PrintChar( c0 );
            istate = NEUTRAL;
	    return;
        }
        istate = NEUTRAL;                     /* For any chance */

    }else{
        --rep;
        c0 = rpc;
    }

    /*
     * Output a character
     */
    for(;;){
        switch( c0 ){
        case ER:
	    { int slice;
		CuAttr = DFL_ATTR;
		for( i = 0; i < YDIM; i += ER_Y_STEP ){
		    if( i + (slice = ER_Y_STEP) > YDIM ) slice = YDIM-i;
		    Serase( i, i+slice-1, 0, XDIM-1, CuAttr & ~(_ATTR_BLINK) );
		    Input();
                }
                SetPos( Ycurr = 0, Xcurr = 0 );
            }
            return;
        case CL:
            if( --Xcurr < 0 ){
                Xcurr = XDIM-1;
                if( --Ycurr < 0 ) Ycurr = YDIM-1;
            }
            SetPos( Ycurr, Xcurr );
            return;
        case CR:
            if( ++Xcurr >= XDIM ){
                c0 = LF;
                break;
            }
            SetPos( Ycurr, Xcurr );
            return;
        case CU:
            if( --Ycurr < 0 ) Ycurr = YDIM-1;
            SetPos( Ycurr, Xcurr );
            return;
        case CD:
            if( ++Ycurr >= YDIM ){
                Ycurr = YDIM-1;
                DeleteLine( 0 );
            }
            SetPos( Ycurr, Xcurr );
            return;
        case IC:
            InsertChar( Ycurr, Xcurr );
            SetPos( Ycurr, Xcurr );
            return;
        case DC:
            DeleteChar( Ycurr, Xcurr );
            SetPos( Ycurr, Xcurr );
            return;
        case IL:
            InsertLine( Ycurr );
            return;
        case DL:
            DeleteLine( Ycurr );
            return;
        case LF:
            Xcurr = 0;
            c0 = CD;
            break;
        case HO:
            Ycurr = Xcurr = 0;
            SetPos( 0, 0 );
            return;
        case RN:
            SetPos( Ycurr, Xcurr = 0 );
            return;
        case EL:
            Serase( Ycurr, Ycurr, 0, XDIM-1, CuAttr & ~(_ATTR_BLINK) );
            return;
        case BL:
            return;
        default:
            if( c0 < 0x20 ) return;
	    PrintChar( c0 );
            return;
        }
    } /*loop*/

}

void NewSize( dpmode, saveenb )
    int dpmode;
    int saveenb;
{
    static int SizeStart = 1;
    union REGS rint;

    Serase( YDIM, YDIM, 0, XDIM-1, DFL_ATTR );
     /* Set the screen hight */
    switch( dpmode ){
    case 1:
	Y_dim = 43-1;
	rint.x.ax = 0x1201;
	rint.h.bl = 0x30;
	int10( &rint );
	rint.h.ah = 0;
	rint.h.al = SaveMode;
	int10( &rint );
	rint.x.ax = 0x1112;
	rint.h.bl = 0;
	int10( &rint );
	break;
    case 2:
	Y_dim = 50-1;
	rint.x.ax = 0x1202;
	rint.h.bl = 0x30;
	int10( &rint );
	rint.h.ah = 0;
	rint.h.al = SaveMode;
	int10( &rint );
	rint.x.ax = 0x1112;
	rint.h.bl = 0;
	int10( &rint );
	break;
    default:
	rint.h.ah = 0;
	rint.h.al = SaveMode;
	/**if( !SimpleFlag && saveenb ) rint.h.al |= 0x80;**/
	int10( &rint );
	Y_dim = 24;
    }
    if( SizeStart == 0 || YDIM != 24 ){
	OutQue(0x95);    OutQue(YDIM+0x20);    OutQue(XDIM+0x20);
    }
    SizeStart = 0;
}


/** Extra line server **/
static int ExtPos;

 /* Write into extra line */
void edout( s )
    char *s;
{
    union REGS rint;

    while( *s ){
        if( ExtPos >= XDIM ) ExtPos = 0;
        rint.h.ah = _VIDEO_SETPOS;
        rint.h.bh = CuPage;
        rint.h.dh = YDIM;    rint.h.dl = ExtPos++;
        int10( &rint );
        rint.h.ah = _VIDEO_WRITE;
        rint.h.bh = CuPage;
        rint.h.al = *s++;
        rint.x.cx = 1;
        rint.h.bl = _ATTR_GREEN<<4;
        int10( &rint );
    }
    rint.h.ah = _VIDEO_SETPOS;
    rint.h.bh = CuPage;
    rint.h.dh = Ycurr;    rint.h.dl = Xcurr;
    int10( &rint );
}
 /* Set write position and place cursor into extra line */
void edpos( x )
    int x;
{
    union REGS rint;

    if( (ExtPos = x) >= XDIM ) ExtPos = 0;
    rint.h.ah = _VIDEO_SETPOS;
    rint.h.bh = CuPage;
    rint.h.dh = YDIM;    rint.h.dl = ExtPos;
    int10( &rint );
}

 /* Display driver initialisation */
void dpbeg()
{
    union REGS rint;

    rint.h.ah = 0x0F;               /* Read a video mode */
    int10( &rint );
    SaveMode = rint.h.al & 0x7F;
    if( SaveMode != 2 && SaveMode != 3 && SaveMode != 7 ){
	write( 2, "Not a suitable video (It must be 80x25)\r\n", 41 );
	_exit( 1 );
    }
    SaveMode = rint.h.al;
    /**CuPage = rint.h.bh;**/
     /* Save a DOS cursor attributes */
    rint.h.ah = _VIDEO_READ;
    rint.h.bh = CuPage;
    int10( &rint );
    SaveAttr = rint.h.ah;
    CuAttr = DFL_ATTR;
     /* Save a DOS screen hight */
    SimpleFlag = 0;
    rint.x.ax = 0x1130;
    rint.h.bh = 0;       /* Dummy table pointer number */
    rint.h.dl = 0;       /*Hope that CGA BIOS should save registers*/
    int10( &rint );
    switch( rint.h.dl ){
    case 25-1:
	Old_maxmode = 1;
	break;
    case 43-1:
	Old_maxmode = 2;
	break;
    case 50-1:
	Old_maxmode = 3;
	break;
    default:
	Old_maxmode = 1;
	SimpleFlag = 1;
    }
    /**maxmode = (maxmode>Old_maxmode)? Old_maxmode: maxmode;**/
    scrmode = maxmode-1;
    NewSize( scrmode, 1 );
     /* Fill the screen with blank attributes */
    Serase( 0, YDIM-1, 0, XDIM-1, CuAttr & ~(_ATTR_BLINK) );
    SetPos( Ycurr = 0, Xcurr = 0 );

    ExtPict();
}
 /* Terminate actions */
void dpend()
{
    union REGS rint;

    NewSize( Old_maxmode-1, 0 );
    SetPos( YDIM, 0 );
    rint.x.cx = 1;
    rint.h.bl = SaveAttr;
    rint.h.bh = CuPage;
    rint.h.al = ' ';
    rint.h.ah = _VIDEO_WRITE;
    int10( &rint );
}
