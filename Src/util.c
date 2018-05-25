/*
 * util.c
 *
 *  Created on: 24 de mai de 2018
 *      Author: tpv
 */

#include "util.h"
#include <stdio.h>
#include <string.h>
int match( const char *text, const char *pattern ){
    size_t tamTexto = strlen( text );

    if( tamTexto > 0 ) {
        char *palavra = strstr( text, pattern );
        if( palavra != NULL ){
        	return ( int )( palavra - text );
        }
    }
    return -1;
}

