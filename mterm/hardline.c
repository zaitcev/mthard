/**
 ** Serial line (aka COM port) service task & Setup
 ** Usually it works via BIOS, but capable to due with PC hardware
 **/
#ifndef _MY_DOS_
#  include <DOS.H>
#  define _MY_DOS_
#endif
#include "MTPROTO.H"
#include "DISPCTRL.H"
#ifndef NULL
#  define NULL ((void*)0)
#endif
#ifndef __TURBOC__    /*e.g. Microsoft*/
#include <conio.h>   /* outp() */
#define getvect _dos_getvect
#define setvect _dos_setvect
#define inportb inp
#define outportb outp
#define enable _enable
#define disable _disable
#endif
#ifndef max
#  define max(a,b)   (((a)>(b))?(a):(b))
#endif

static int hardmode;

/*
 * Manifest constants for
 * BIOS serial communications (RS-232) support
 */
#define _COM_INIT       0   /* init serial port */
#define _COM_SEND       1   /* send character */
#define _COM_RECEIVE    2   /* receive character */
#define _COM_STATUS     3   /* get serial port status */

struct Baud_t {   int b_val;   char *b_name;  };
struct Form_t {   int f_val;   char *f_name;  };
struct Port_t {
    int p_base;     /* Base in I/O space */
    int p_vect;     /* Vector number */
    int p_itm;      /* Inerrupt Mask for 8259A */
    char *p_name;
};

static struct Baud_t BaudHard[] = {
    {    1538,    "75"  },
    {     385,   "300"  },
    {      96,  "1200"  },
    {      48,  "2400"  },
    {      24,  "4800"  },
    {      12,  "9600"  },
    {       6, "19200"  },
    {       2, "57600"  }
};
#define DFL_BAUD_H  5
static struct Baud_t BaudBIOS[] = {
    {    0x00,  "110"   },
    {    0x20,  "150"   },
    {    0x40,  "300"   },
    {    0x60,  "600"   },
    {    0x80,  "1200"  },
    {    0xA0,  "2400"  },
    {    0xC0,  "4800"  },
    {    0xE0,  "9600"  }
};
#define DFL_BAUD_B  7

static struct Form_t FormDesc[] = {
    {  2 +4 +0x18  , "7e2" },
    {  3           , "8n1" },
    {  3 +4        , "8n2" },
    {  3    +0x18  , "8e1" },
    {  3 +4 +0X18  , "8e2" }
};
#define DFL_FORM  1

static struct Port_t PortDesc[] = {
    {  0x3F8, 0x0C, 0x10, "COM1" },
    {  0x2F8, 0x0B, 0x08, "COM2" },
    {  0x3E8, 0x0C, 0x10, "COM3" },
    {  0x2E8, 0x0B, 0x08, "COM4" }
};
#define DFL_PORT  0
#define NPORTS_H  4
#define outcom(port,reg,data) outportb(PortDesc[port].p_base+reg,data)
#define incom(port,reg)       inportb( PortDesc[port].p_base+reg)

#define DLLB_DATA  0
#define DLHB_INTEN 1
#define INTIDENT   2
#define LINECTRL   3
#define MODEMCTRL  4
#define LINESTATE  5
#define MODEMSTATE 6

#define NBAUDS_B (sizeof(BaudBIOS)/sizeof(*BaudBIOS))
#define NBAUDS_H (sizeof(BaudHard)/sizeof(*BaudHard))
#define NFORMS (sizeof(FormDesc)/sizeof(*FormDesc))

#define IRX  0x01
#define ITX  0x02
#define ILS  0x04
#define IMS  0x08

static int CuPort;
static int CuBaud;
static int CuForm;

#define QSIZE 5          /* &95-Y-X + X_OFF + KeybSym */
static volatile char Oqueue[ QSIZE ], *pp = Oqueue, *gp = Oqueue;
static volatile int ccnt = 0;

#define IBSIZE  900
#define HIWATER 400
#define LOWATER  50
static volatile char Ibuff[ IBSIZE ], *ibget = Ibuff, *ibput = Ibuff;
static volatile int bfc = 0;

static volatile int Xflag = 0;

#define EMPTY  1
#define SAVED  0
static int OrdSaved = EMPTY;
static int OrdPort;
#ifdef __TURBOC__
static void interrupt (*OrdProc)( void );
#else /*Microsoft*/
void (interrupt far *OrdProc)( void );
#endif

static outbusy = 0;    /* Transmitter is busy */

 /* Set the hardware parameters */
void LineSet( int, int, int, int );
static void
LineSet( Port, Rate, Format, IRQ )
    int Port;
    int Rate, Format;
    int IRQ;
{
    extern void Linit();
    void interrupt PortEvent();
    short intnum;

    if( hardmode ){
	if( IRQ != -1 ){
	    PortDesc[Port].p_itm = 1<<IRQ;
	    PortDesc[Port].p_vect = 8+IRQ;
	}
         /* Restore old vector etc. */
        if( OrdSaved == SAVED ){
            outportb( 0x21, inportb( 0x21 ) | PortDesc[ OrdPort ].p_itm );
            outcom( OrdPort, DLHB_INTEN, 0 );  /* Disable all interrupts */
            setvect( PortDesc[ OrdPort ].p_vect, OrdProc );
            OrdSaved = EMPTY;
        }
         /* Save and set vector (may be different from previous one) */
        OrdPort = Port;
        OrdProc = getvect( PortDesc[ Port ].p_vect );
        OrdSaved = SAVED;    
        setvect( PortDesc[ Port ].p_vect, PortEvent );

	disable();
	if( Rate >= 0 ){
	    outcom( Port, LINECTRL, 0x80 );
	    outcom( Port, DLLB_DATA,  BaudHard[ Rate ].b_val );
	    outcom( Port, DLHB_INTEN, BaudHard[ Rate ].b_val >> 8 );
	    outcom( Port, LINECTRL, FormDesc[ Format ].f_val );
	}

        outcom( Port, MODEMCTRL, 0x0B );

        for(;;){
            intnum = incom( Port, INTIDENT );
            if( intnum & 0x01 ) break;
	    switch( (intnum >> 1) & 0x03 ){
	    case 3:         /* Receiver line status */
		incom( Port, LINESTATE );
                break;
            case 2:         /* Receiver data available */
                incom( Port, DLLB_DATA );
                break;
            case 1:         /* Transmitter buffer empty */
		outcom( Port, DLHB_INTEN, IRX|ILS|IMS );
		break;
	    case 0:         /* Modem status */
		incom( Port, MODEMSTATE );
		break;
	    }
	}
	outcom( Port, DLHB_INTEN, IRX|ILS|IMS );
	outportb( 0x21, inportb( 0x21 ) & ~(PortDesc[Port].p_itm) );
	enable();
    }else{
        Linit( Port, BaudBIOS[ Rate ].b_val | FormDesc[ Format ].f_val );
    }
}

/*
 * HARDWARE EVENT
 */
void interrupt PortEvent()
{
    void OutMasked( char );
    short intnum;
    char cdatum;

    for(;;){
        intnum = incom( CuPort, INTIDENT );
        if( intnum & 0x01 ) break;
	switch( (intnum >> 1) & 0x03 ){
	case 3:         /* Line status */
	    incom( CuPort, LINESTATE );
	    break;
	case 2:         /* Receiver data available */
            /*
             * Put a received char into a circular queue
             */
            cdatum = incom( CuPort, DLLB_DATA );
            if( bfc < IBSIZE ){
                bfc++;
                *ibput++ = cdatum;
                if( ibput == Ibuff+IBSIZE ) ibput = Ibuff;
                if( bfc >= HIWATER && !Xflag ){
                    Xflag = 1;
                    OutMasked( X_OFF );
		}
	    }
	    break;
	case 1:         /* Transmitter buffer empty */
	    /*
	     * Testing for output and start it if needed
	     */
	    if( ccnt != 0 ){
		outcom( CuPort, DLLB_DATA, *gp );
		--ccnt;
		if( ++gp == Oqueue+QSIZE ) gp = Oqueue;
	    }else{
		 /* Disable this interrupt */
		/**outcom( CuPort, DLHB_INTEN, IRX|ILS|IMS );**/
	    }
	    outbusy = 0;
	    break;
	case 0:         /* Modem status */
	    incom( CuPort, MODEMSTATE );
	    outcom( CuPort, MODEMCTRL, 0x0B );
	    break;
	}
    }
     /* Non-specific EOI (for full nesting ints.) */
    outportb( 0x20, 0x20 );
}

/*
 * INPUT SOFTWARE EVENT EMULATOR
 */
void
Input()
{
    auto   int tsret;
    extern int Ltest();

    if( hardmode ) return;

    for(;;){
        /*
         * Test an input port
         */
        tsret = Ltest( CuPort );
        if( tsret & 0x200 ){           /* Error ! */
	    LineSet( CuPort, CuBaud, CuForm, -1 );
        }
        if( tsret & 0x100 ) return;    /* Nothing */

        /*
         * Put a received char into a circular queue
         */
        if( bfc < IBSIZE ){
            bfc++;
            *ibput++ = tsret;
            if( ibput == Ibuff+IBSIZE ) ibput = Ibuff;
            if( bfc >= HIWATER && !Xflag ){
                Xflag = 1;
                OutQue( X_OFF );
            }
        }
    } /*loop*/
}

/*
 * OUTPUT SOFTWARE EVENT EMULATOR
 */
void
Line()
{
    extern int Ltryout();

    if( !hardmode ){
        /*
         * Testing for output and start it if needed and possible
         */
        if( ccnt != 0 && Ltryout( CuPort, *gp ) == 0 ){
            --ccnt;
            if( ++gp == Oqueue+QSIZE ) gp = Oqueue;
        }
    }
}

static void
OutMasked( c )
    char c;
{
    int enb_size;

    enb_size = QSIZE;
    if( c != X_ON && c != X_OFF ) --enb_size;

    if( hardmode ){
	if( !outbusy ){
	    outcom( CuPort, DLLB_DATA, c );
	     /* Enable TX interrupts */
	    outcom( CuPort, DLHB_INTEN, IRX|ITX|ILS|IMS );
	    outbusy = 1;
	}else{                                    /* Save byte in the queue */
	    if( incom( CuPort,LINESTATE )&0x20 ){ /* Holding reg. is empty  */
		outcom( CuPort, DLHB_INTEN, IRX|ILS|IMS );   /*Wakeup*/
	    }
	    outcom( CuPort, DLHB_INTEN, IRX|ITX|ILS|IMS );
	    if( ccnt < enb_size ){
		ccnt++;
		*pp++ = c;
		if( pp == Oqueue+QSIZE ) pp = Oqueue;
	    }else{
		edpos( 60 );   edout( "Clog" );
	    }
	}
    }else{
	if( ccnt < enb_size ){
	    ccnt++;
	    *pp++ = c;
	    if( pp == Oqueue+QSIZE ) pp = Oqueue;
	}
    }
}

void
OutQue( c )   char c;
{
    disable();    OutMasked( c );    enable();
}

int          /* ==0x100 if no characters in the input queue */
GetIn()
{
    register int cret;

    disable();       /* Input Queue & Xflag guardance */
    if( bfc == 0 ){
        cret = 0x100;
    }else{
        --bfc;
        cret = *ibget++ & 0xFF;
        if( ibget == Ibuff+IBSIZE ) ibget = Ibuff;
    }
    if( bfc < LOWATER && Xflag ){
        Xflag = 0;
	OutMasked( X_ON );
    }
    enable();
    return cret;
}

void
SetInit( hflag, port, irq )
    int hflag;        /* Use hardware instead of BIOS */
    int port;
    int irq;
{
    hardmode = hflag;

    OrdSaved = EMPTY;
    CuPort = DFL_PORT;
    CuBaud = (hardmode)? DFL_BAUD_H: DFL_BAUD_B;
    CuForm = DFL_FORM;
    if( hardmode && port >= 0 ){
	LineSet( CuPort = port, -1, -1, irq );
    }else{
	SetUp();
    }
}

void
SetEnd()
{

    if( hardmode ){
        if( OrdSaved == SAVED ){
            outportb( 0x21, inportb( 0x21 ) | PortDesc[ OrdPort ].p_itm );
            outcom( OrdPort, DLHB_INTEN, 0 );
            setvect( PortDesc[ OrdPort ].p_vect, OrdProc );
        }
        OrdSaved = EMPTY;
    }
}

 /* SetUp control program */
void
SetUp()
{
    static char *csp[ NPORTS_H+1 ];
    static char *bsp[ max(NBAUDS_B,NBAUDS_H)+1 ];
    static char *fsp[ NFORMS+1 ];
    int x;

    for( x = 0; x < NPORTS_H; x++ ) csp[ x ] = PortDesc[ x ].p_name;
    csp[ x ] = NULL;
    CuPort = ExtSelect( csp, CuPort, 24 );

    if( hardmode ){
        for( x = 0; x < NBAUDS_H; x++ ) bsp[ x ] = BaudHard[ x ].b_name;
    }else{
        for( x = 0; x < NBAUDS_B; x++ ) bsp[ x ] = BaudBIOS[ x ].b_name;
    }
    bsp[ x ] = NULL;
    CuBaud = ExtSelect( bsp, CuBaud, 29 );

    for( x = 0; x < NFORMS; x++ ) fsp[ x ] = FormDesc[ x ].f_name;
    fsp[ x ] = NULL;
    CuForm = ExtSelect( fsp, CuForm, 35 );

    LineSet( CuPort, CuBaud, CuForm, -1 );
}
