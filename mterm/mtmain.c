/**
 ** Main scheduling loop
 **/
#include "MTPROTO.H"
#ifndef NULL
#   define NULL ((void*)0)
#endif

int maxmode, scrmode;

main( argc, argv )
    unsigned argc;
    char **argv;
{
    char *arg, ca;
    int hardflag = 0;      /* Use the COM port hardware */
    int comport, comirq;

    maxmode = 1;
    comport = -1;
    comirq = -1;
    while( (arg = *argv++) != NULL ){
        if( (ca = *arg++) == '/' || ca == '-' ){
	    while( (ca = *arg++) != 0 ){
		switch( ca ){
		case '1':    case '2':    case '3':    case '4':
		    comport = ca-'0'-1;
		    break;
		case 'S':    case 's':    /* Super-condensed mode */
		    maxmode = 3;
		    break;
		case 'C':    case 'c':    /* Condensed mode */
		    maxmode = 2;
		    break;
		case 'H':    case 'h':    /* Hardware communications */
		    hardflag = 1;
		    break;
		case 'I':    case 'i':
		    ca = *arg;
		    if( ca != 0 ){
			arg++;
			if( ca=='3' || ca=='4' || ca=='5' || ca=='7' ){
			    comirq = ca-'0';
			}
		    }
                    break;
		}
	    }
	}
    }

    dpbeg();
    SetInit( hardflag, comport, comirq );  /* Use Stop() after this */
    OutQue( X_ON );

     /* Testing order is significant */
    for(;;){
        Screen();   /* Display output */
        Line();     /* Line test for output */
        Input();    /* Line test for input */
        Keyb();     /* Keyboard test */
    }
}

void Stop( code )
    int code;
{

    SetEnd();
    dpend();
    exit( code );
}
