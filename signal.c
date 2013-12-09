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

#ifdef __sgi__
#define _XOPEN_SOURCE 1
#include <sys/signal.h>
#endif

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef HAS_CURSES
#include <curses.h>
#endif

#include "stats.h"
#include "utils.h"
#include "genetic.h"

typedef void (*sh_t)(int);

void handler( int s )
{
	yDEBUG( "received signal %d\n", s );

	if ( s < 0 ) {
		if ( (int)signal( SIGINT,  (sh_t)handler ) <  0 ) yperrorf( "setting SIGINT" );
		if ( (int)signal( SIGABRT, (sh_t)handler ) <  0 ) yperrorf( "setting SIGABRT" );
		if ( (int)signal( SIGTERM, (sh_t)handler ) <  0 ) yperrorf( "setting SIGTERM" );
		if ( (int)signal( SIGHUP,  (sh_t)handler ) <  0 ) yperrorf( "setting SIGHUP" );
		return;
	}

	if ( s == SIGABRT || s == SIGTERM || s == SIGINT ) {

#if defined(HAS_CURSES) && defined(INTERACTIVE)
	  if ( gv_visual ) endwin();
#endif

	  savep( "emergency" );
	  ymessage( "Current status saved in file \"emergency.sts\"\n" );
	  saver( "emergency" );
	  ymessage( "Current population saved in file \"emergency.pop\"\n" );
	  saves( "emergency" );
	  ymessage( "Current runs stats saved in file \"emergency.run\"\n" );
	  prints();
	  exit( EXIT_SUCCESS );

	}

	if ( s == SIGHUP  ) {
		if ( status.curgen > 0 ) {
			savep( "dump" );
			saver( "dump" );
			saves( "dump" );
		}
		if ( (int)signal( SIGHUP, (sh_t)handler ) <  0 ) yperrorf( "resetting SIGHUP" );
	}
}
