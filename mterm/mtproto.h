/**
 ** MISSTERM globals
 **/

#define X_OFF  ((char)0x93)
#define X_ON   ((char)0x91)

extern int maxmode;           /* 1 - 80x25, 2 - 80x43, 4 - 80x50 */
extern int scrmode;           /* 0 <= curr_scr_mode <= maxmode-1 */

 /* Video interface */
void ExtPict( void );         /* Refresh the extra line */
void dpbeg( void );           /*Using extern maxmode */
void dpend( void );
void Screen( void );          /* Display task tick entry */
void NewSize( int, int );
void edout( char* );          /* Extra line services   */
void edpos( int );            /*   for SetUp()         */

 /* Line & setup (COMM port shell) */
void SetInit( int, int,int ); /* Line & setup initiator */
void SetUp( void );           /* Manual setup */
void Line( void );            /* Line task tick entry */
void Input( void );           /* Super-priority input task */
void OutQue( char );          /* Send character to output */
int  GetIn( void );           /* Test for an input character */

int ExtSelect( char** strs, int pos, int extpos );

 /* Keyboard services */
void Keyb( void );            /* Keyboard task entry */
char dpi( void );             /* dpi() for Setup */

void Stop( int );

#ifndef _MY_DOS_    /*instead of compiler-dependent __DOS_H or __DOS_H__ */
#  include <dos.h>   /* union REGS */
#  define _MY_DOS_
#endif
 /* Assembly procedures.  Check before usage. */
void Linit(  int port, int mode );
int Ltest(   int port );
int Ltryout( int port, short char );
int Kdpm( void );
void Sinit( int mode );
void Serase( int top, int bot, int lpos, int rpos, short attr );
void Spos( int Y, int X );
void Swrite( short char, short attr );
void int10( union REGS * );
