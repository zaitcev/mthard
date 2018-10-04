/*
 * Conversions
 */

#define loop for(;;)
#define MAXDIG 8

static char symtab[] = "0123456789ABCDEF";

char *
conv( val, len, rad )
    unsigned int val;
    int len;
    int rad;
{
    static char nbuff[ MAXDIG+1 ];
    int nx;
    char filler;

    filler = (len & 0x80)? '0': ' ';
    if( (len &= 0x80-1) >= MAXDIG ) len = MAXDIG;
    for( nx = 0; nx < len; nx++ ) nbuff[ nx ] = filler;
    nbuff[ nx ] = '\0';    nbuff[ nx-1 ] = '0';
    for( ; --nx >= 0 && val != 0; ){
        nbuff[ nx ] = symtab[ val % rad & 0x0F ];    val /= rad;
    }
    return nbuff;
}

int
convi( ps, rad )
    char *ps;
    int rad;
{
    unsigned int retnum;
    register char *pn;

    retnum = 0;
    loop{
        pn = symtab;
        loop{
            if( *pn == '\0' ) return (int) retnum;
            if( *pn == *ps ) break;
            pn++;
        }
        retnum = retnum*rad + (pn - symtab);
        ps++;
    }
}
