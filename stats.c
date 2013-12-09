/*
    GPSR: Genetic Programming for Symbolic Regression
    Copyright (C) 2012 Massimo Santini

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <string.h>
#ifdef HAS_CURSES
#include <curses.h>
#endif

#include "stats.h"
#include "utils.h"
#include "genetic.h"

struct rstats run[MAX_GENERATIONS];     /* le statistiche per generazione */
int gv_visual, gv_savepop, gv_saverun;

/*
  libera lo spazio allocato per gli individui migliori nelle statistiche
  della popolazione corrente e per i filename dei dati in status
*/
void freemisc( void )
{
	int i;

	assert( 0 < status.totgen && status.totgen <= MAX_GENERATIONS );
	assert( 0 < status.datan && status.datan <= MAX_DATAN );

	freei( status.bssfi );
	for ( i = 0; i < status.totgen; i++ )
		freei( run[i].besti );
	for ( i = 0; i < status.datan; i++ )
		yfree( status.dataf[i++] );
}

/*
  salva le statistiche (fino a curgen) nel file di nome file
*/
void saver( char *file )
{
	int fd, i;
	char filext[MAX_FILENAME_LEN + 4];

	assert( file );
	assert_statusok();

	ystrncpy( filext, file, MAX_FILENAME_LEN );
	strcat( filext, ".run" );

	if (  ( fd = open( filext, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR ) ) < 0 )
		yperror( "can't save runs to '%s'", filext );

	for ( i = 0; i < status.curgen; i++ ) {
		ywrite( fd, &(run[i]), sizeof(run[i]) );
		writei( fd, run[i].besti );
	}
	if ( close(fd) < 0 ) yperrorf( "closing file");
}

/*
  alloca e legge le statistiche (fino a curgen) dal file di nome file
  ( non libera in precedenza le statistiche )
*/
void restorer( char *file )
{
	int fd, i;
	char filext[MAX_FILENAME_LEN + 4];

	assert( file );
	assert_statusok();

	ystrncpy( filext, file, MAX_FILENAME_LEN );
	strcat( filext, ".run" );

	if (  ( fd = open( filext,  O_RDONLY ) ) < 0 )
		yperror( "can't restore runs from '%s'", filext );

	for ( i = 0; i < status.curgen; i++ ) {
		yread( fd, &(run[i]), sizeof(run[i]) );
		run[i].besti = readi( fd );
	}

	if ( close(fd) < 0 ) yperrorf( "closing file" );
}

/*
  stampa le statistiche di popolazione dell' i-esimo run
*/
void printr( int i )
{
	assert_statusok();
	assert( 0 <= i && i < status.curgen );

	printf( "\nStatistics:\n" );
	printf( "----------- \n" );
	printf( "usedmem   : %d\n", run[i].usedmem );
#ifndef FAST_STATS
	printf( "mintok    : %d\n", run[i].mintok );
	printf( "avetok    : %d\n", run[i].avetok );
	printf( "maxtok    : %d\n", run[i].maxtok );
	printf( "minval    : %d\n", run[i].minval );
	printf( "aveval    : %d\n", run[i].aveval );
	printf( "maxval    : %d\n", run[i].maxval );
	printf( "----------- \n\n" );
#endif
	printf( "maxadjf   : %e\n", run[i].maxadjf );
	printf( "aveadjf   : %e\n", run[i].aveadjf );
	printf( "varadjf   : %e\n", run[i].varadjf );
	printf( "----------- \n" );
}

#if defined(HAS_CURSES) && defined(INTERACTIVE)
void displays( void )
{
	int i;

    assert_statusok();
	assert( status.curgen > 0 );
	i = status.curgen - 1;
	clear();
	printw( "----------- \n" );
	printw( "curgen    : %d\n", status.curgen );
	printw( "----------- \n" );
	printw( "bssfg     : %d\n", status.bssfg );
	printw( "bssfadjf  : %e (%e)\n", status.bssfadjf, 1.0/status.bssfadjf-1.0 );
	if ( gv_visual == VISUAL_MAX ) {
		printw( "bssfi     : \n" );
		displayi( status.bssfi );
	}
	printw( "----------- \n" );
	printw( "maxadjf   : %e\n", run[i].maxadjf );
	printw( "aveadjf   : %e\n", run[i].aveadjf );
	printw( "varadjf   : %e\n", run[i].varadjf );
	if ( gv_visual == VISUAL_MAX ) {
		printw( "besti     : \n" );
		displayi( run[i].besti );
	}
	printw( "----------- \n" );
	printw( "usedmem   : %d\n", run[i].usedmem );
#ifndef FAST_STATS
	printw( "mintok    : %d\n", run[i].mintok );
	printw( "avetok    : %d\n", run[i].avetok );
	printw( "maxtok    : %d\n", run[i].maxtok );
	printw( "minval    : %d\n", run[i].minval );
	printw( "aveval    : %d\n", run[i].aveval );
	printw( "maxval    : %d\n", run[i].maxval );
#endif
	printw( "----------- \n" );
	refresh();
}
#endif



