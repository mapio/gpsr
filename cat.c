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

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

#include "genetic.h"
#include "expr.h"
#include "stats.h"
#include "signal.h"
#include "utils.h"

#define CAT_GEN 1
#define CAT_MAX 2
#define CAT_AVE 3
#define CAT_IND 4
#define CAT_FOR 5
#define CAT_BSS 6
#define CAT_DAT 7
#define MAX_CAT CAT_DAT

int what;

int main( int argc, char **argv )
{
	int gen = -1, i, option_index;
	char file[MAX_FILENAME_LEN + 1];

	static struct option long_options[] = {
		{"general", no_argument, &what, CAT_GEN},
		{"maxadjf", no_argument, &what, CAT_MAX},
		{"aveadjf", no_argument, &what, CAT_AVE},
		{"bssfi", no_argument, &what, CAT_IND},
		{"bssfp", required_argument, &what, CAT_BSS},
		{"data", no_argument, &what, CAT_DAT},
		{"forecast", required_argument, &what, CAT_FOR},
		{"help", no_argument, NULL, 0},
		{0, 0, 0, 0}};

	what = CAT_GEN;

	while ( getopt_long(argc, argv, "", long_options, &option_index) >= 0 ) {
		if ( !( 0<= option_index && option_index <= MAX_CAT) ) {
			fprintf( stderr, "Available options:\n\n" );
			fprintf( stderr, " --general/--maxadjf/--aveadjf/--bssfi/--forecast gen\n" );
			fprintf( stderr, " --help\n\n" );
			exit( EXIT_SUCCESS );
		}
		if ( option_index == CAT_IND || option_index== CAT_BSS ) {
		  gen = atoi( optarg );
		}
	}

	if (optind == argc - 1)
		ystrncpy( file, argv[optind], MAX_FILENAME_LEN );
	else
		yerror( "missing filename\n" );

	restores( file );

	switch( what ) {

	case CAT_GEN:
		prints();
		break;

	case CAT_MAX:
		restorer( file );
		for ( i = 0; i < status.curgen; i++ )
			printf( "%e\n", run[i].maxadjf );
		break;

	case CAT_AVE:
		restorer( file );
		for ( i = 0; i < status.curgen; i++ )
			printf( "%e\n", run[i].aveadjf );
		break;

	case CAT_IND:
#ifdef HAS_SIMPLIFY
		printsi( status.bssfi );
#else
		printi( status.bssfi );
#endif
		break;

	case CAT_FOR:
	  if ( gen < 1 || gen > status.curgen ) {
		yerror( "gen must be in [1,%d]\n", status.curgen );
	  } else {
		forecast f;
		for( i = 0; i < status.datan; i++ )
		  readdata( status.dataf[i], i );
		restorer( file );
		f = evali( run[gen-1].besti, datalen( DATA_TARGET ) - 1 );
		for ( i = 0; i < status.length; i++ )
		  printf( "%f\n", f.v[i] );
	  }
	  break;

	case CAT_BSS:

	  if ( gen < 1 || gen > status.curgen ) {
		yerror( "gen must be in [1,%d]\n", status.curgen );
	  } else {
		int h, t;
		forecast r;

		for( i = 0; i < status.datan; i++ )
		  readdata( status.dataf[i], i );
		restorer( file );
		for ( t = 0; t < datalen( DATA_TARGET ); t++ ) {
		  printf( "%d %f\n", t, getdata( DATA_TARGET, t ) );
		  r = evali( run[gen-1].besti, t );
		  for ( h = 0; h < status.length; h++ )
			printf( "%d %f\n", t + h + status.horizon, r.v[h] );
		}
	  }
	  break;

	case CAT_DAT:
	  {
		int t;

		for( i = 0; i < status.datan; i++ )
		  readdata( status.dataf[i], i );
		restorer( file );
		for ( t = 0; t < datalen( DATA_TARGET ); t++ )
		  printf( "%d %f\n", t, getdata( DATA_TARGET, t ) );

	  }
	  break;

	}



	return 0;
}
