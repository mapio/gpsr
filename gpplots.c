/*
    GPSR: Genetic Programming for Symbolic Regression
    Copyright (C) 2002 Massimo Santini

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

#define _GNU_SOURCE  /* per popen/pclose e mktemp */

#include "config.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "utils.h"
#include "gpplots.h"

gp_port gp_open( char *geom )
{
  char cmd[256];
  gp_port port;

#ifdef DEBUG
  strcpy( cmd, "cat 1>&2" );
#else
  if ( geom )
    sprintf( cmd, "gnuplot -geometry %s", geom );
  else
    strcpy( cmd, "gnuplot" );
#endif

  if ( ( port=popen( cmd, "w")) == NULL)
    yperrorf( "popening gnuplot" );
  setvbuf( port, NULL, _IONBF, 0 );

  return port;
}

void gp_close( gp_port port )
{
  if ( pclose( port ) < 0 )
    yperrorf( "pclosing gnuplot" );
}

void gp_title( gp_port port, char *msg )
{
  gp_tell( port, "set title '%s'\n replot\n", msg );
}

void gp_save( gp_port port, char *term, char *file )
{
  gp_tell( port, "set output '%s'\n set terminal %s\n", file, term );
  gp_tell( port, "replot\n set terminal x11\n replot\n" );
}

void gp_plotfile( gp_port port, int replot, char *style, char *filename )
{
  char cmd[256];

  if ( replot == GP_REPLOT )
	  sprintf( cmd, "replot '%s' %s\n", filename, style );
  else
	  sprintf( cmd, "plot '%s' %s\n", filename, style );

  gp_tell( port, cmd );
}

void gp_plotarray( gp_port port, int replot, char *style, plotv *x, plotv *y, int n )
{
	int i;
	char file[MAX_FILENAME_LEN + 1];
	FILE *out;

	ystrncpy( file, "/tmp/gpioXXXXXX", MAX_FILENAME_LEN );
	mktemp( file );
	if ( (out = fopen(file, "w")) == NULL )
		yperrorf( "can't open '%s'", file );
	if ( x == NULL )
		for ( i = 0; i < n; i++ )
			fprintf( out, "%e %e\n", (plotv)i, y[i] );
	else
		for ( i = 0; i < n; i++ )
			fprintf( out, "%e %e\n", x[i], y[i] );
	fclose( out );
	gp_plotfile( port, replot, style, file );
	/* unlink( file ); */
}

