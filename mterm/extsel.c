/*
 * Selection a string from a set (Use extra line for display)
 */
#include <string.h>
#include "MTPROTO.H"
#include "DISPCTRL.H"
#ifndef NULL
# define NULL 0
#endif

int ExtSelect( sv, sx, spos )
    char **sv;          /* Values vector */
    int sx;             /* Index for 'sv' */
    int spos;           /* Index into extra line */
{
    int olen, omax;
    int i;
    char c;

    omax = 0;
    edpos( spos );
    for(;;){
	olen = strlen( sv[ sx ] );    edout( sv[ sx ] );
        for( i = omax - olen; --i >= 0; ) edout( " " );
        /** if( (i = omax) < olen ) i = olen; **/
        /** for( ; --i >= 0; ) dpo( CL );      **/
        edpos( spos );
        omax = olen;
        for(;;){
            c = dpi();
            if( c == ET || c == HO ){
                edout( sv[ sx ] ); /*Place cursor into terminal window*/
                return sx;
            }
            if( c == ' ' ){
                if( sv[ ++sx ] == NULL ) sx = 0;
                break;
            }
        }
    }
}
