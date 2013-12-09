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
#include <getopt.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>
#ifdef HAS_CURSES
#include <curses.h>
#endif

#include "genetic.h"
#include "expr.h"
#include "stats.h"
#include "signal.h"
#include "utils.h"

void help( void )
{
	fprintf( stderr, "Available options:\n\n" );
#ifdef HAS_CURSES
	fprintf( stderr, " --batch/--visual/--extravisual\n" );
#endif
	fprintf( stderr, " --popsize [0,%d]\n", MAX_POPULATION_SIZE );
	fprintf( stderr, " --length [1,%d]\n", MAX_LENGTH_SIZE );
	fprintf( stderr, " --horizon n>=1\n" );
	fprintf( stderr, " --maxdepth  [0,%d]\n", MAX_RNDE_DEPTH );
	fprintf( stderr, " --seed  n>0\n" );
	fprintf( stderr, " --generations [0,%d] \n", MAX_GENERATIONS );
	fprintf( stderr, " --mutatep [0,1]\n" );
	fprintf( stderr, " --crossoverp [0,1]\n" );
	fprintf( stderr, " --selectionrho [0,1]\n" );
	fprintf( stderr, " --elitism\n" );
	fprintf( stderr, " --saveas basefile\n" );
	fprintf( stderr, " --nopopsave\n" );
	fprintf( stderr, " --norunsave\n" );
	fprintf( stderr, " --data datafile\n" );
	fprintf( stderr, " --config configfile\n" );
	fprintf( stderr, " --help\n\n" );
	exit( EXIT_SUCCESS );
}

void getargs( int argc, char **argv )
{
	int rv, option_index;

	static struct option long_options[] = {
/* 0 */	{"popsize", required_argument, NULL, 0},
        {"horizon", required_argument, NULL, 0},
        {"length", required_argument, NULL, 0},
		{"maxdepth", required_argument, NULL, 0},
		{"seed", required_argument, NULL, 0},
		{"generations", required_argument, NULL, 0},
		{"mutatep", required_argument, NULL, 0},
		{"crossoverp", required_argument, NULL, 0},
		{"saveas", required_argument, NULL, 0},
		{"data", required_argument, NULL, 0},
		{"help", no_argument, NULL, 0},
		{"nopopsave", no_argument, &gv_savepop, 0},
        {"norunsave", no_argument, &gv_saverun, 0},
        {"selectionrho", required_argument, NULL, 0},
        {"config", required_argument, NULL, 0},
/* 15 */{"elitism", no_argument, &status.elitism, 1},
#ifdef HAS_CURSES
		{"extravisual", no_argument, &gv_visual, VISUAL_MAX},
		{"visual", no_argument, &gv_visual, VISUAL_AVE},
		{"batch", no_argument, &gv_visual, VISUAL_OFF},
#endif
		{0, 0, 0, 0}};

	gv_visual = VISUAL_MIN;
	gv_savepop = 1;
	gv_saverun = 1;
	status.popsize = 500;
	status.horizon = 1;
	status.length = 5;
	status.maxdepth = 5;
	status.seed = -1;
	status.totgen = 100;
	status.mutatep = 0.01;
	status.selectionrho = 0.1;
	status.crossoverp = 0.2;
	status.configf[0] = '\0';
	status.elitism = 0;
	if ( yexists( "gpsr.config" ) ) getgpsrcfg( "gpsr.config" );

	ystrncpy( status.savef, "saved", MAX_FILENAME_LEN );

	while ( (rv = getopt_long(argc, argv, "", long_options, &option_index)) == 0 ) {
		switch ( option_index ) {
		case 0:
			status.popsize = atoi( optarg );
			if ( status.popsize <= 0 || status.popsize > MAX_POPULATION_SIZE )
				yerror( "popsize must be in [1,%d]\n", MAX_POPULATION_SIZE );
			break;
		case 1:
			status.horizon = atoi( optarg );
			if ( status.horizon <= 0  )
				yerror( "horizon must be >=1\n" );
			break;
		case 2:
			status.length = atoi( optarg );
			if ( status.length <= 0 || status.length > MAX_LENGTH_SIZE )
				yerror( "length must be in [1,%d]\n", MAX_LENGTH_SIZE );
			break;
		case 3:
			status.maxdepth = atoi( optarg );
			if ( status.maxdepth <= 0 || status.maxdepth > MAX_RNDE_DEPTH )
				yerror( "maxdepth must be in [1,%d]\n", MAX_RNDE_DEPTH );
			break;
		case 4:
			status.seed = atoi( optarg );
			if ( status.seed <= 0 )
				yerror( "seed must be positive\n" );
			break;
		case 5:
			status.totgen = atoi( optarg );
			if ( status.totgen <= 0 || status.totgen > MAX_GENERATIONS )
				yerror( "generations must be in [1,%d]\n", MAX_GENERATIONS );
			break;
		case 6:
			status.mutatep = atof( optarg );
			if ( status.mutatep < 0.0 || status.mutatep > 1.0 )
				yerror( "mutatep must be in [0,1]\n" );
			break;
		case 7:
			status.crossoverp = atof( optarg );
			if ( status.crossoverp < 0.0 || status.crossoverp > 1.0 )
				yerror( "crossoverp must be in [0,1]\n" );
			break;
		case 8:
			ystrncpy( status.savef, optarg, MAX_FILENAME_LEN );
			break;
		case 9:
			if ( status.datan < MAX_DATAN )
				readdata( optarg, status.datan++ );
			else
				yerror( "you can read at most %d datafile(s)\n", MAX_DATAN );
			break;
		case 10:
			help();
			break;
		case 11: case 12:
			/* opzioni con argomento autosettato */
			break;
		case 13:
			status.selectionrho = atof( optarg );
			if ( status.selectionrho < 0.0 || status.selectionrho > 1.0 )
				yerror( "selectionrho must be in {0} (fitness proportional), or (0,1]\n" );
			break;
		case 14:
			getgpsrcfg( optarg );
			break;
		case 15:
			/* opzione con argomento autosettato */
			break;
#ifdef HAS_CURSES
		case 16: case 17: case 18:
			/* opzioni con argomento autosettato */
			break;
#endif
		default:
			help();
			break;
		}
	}
	if ( rv != -1 ) help();
	if ( status.datan == 0 ) yerror( "no datafile specified!\n" );
}

void saveall( void )
{
	saves( status.savef );
	ymessage( "status saved in %s.sts\n", status.savef );
	if ( gv_saverun ) {
		saver( status.savef );
		ymessage( "runs stats saved in %s.run\n", status.savef );
	}
	if ( gv_savepop ) {
		savep( status.savef );
		ymessage( "population saved in %s.pop\n", status.savef );
	}
}

int restoreemergency( void )
{
	if ( yexists( "emergency.sts" ) ) {
		restores( "emergency" );
		restorer( "emergency" );
		restorep( "emergency" );
		return 1;
	} else return 0;
}

int main( int argc, char **argv )
{
	if ( 1 || !restoreemergency() ) {
		getargs( argc, argv );
		if ( tokgp( TOK_DATA ) == 0.0 ) equiprobfunc();
		initp();
		rndp();
	}

	if ( gv_visual >= VISUAL_AVE ) {
#ifdef HAS_CURSES
		initscr();
#endif
	} else handler(-1);

	generationp();

#ifdef HAS_CURSES
	if ( gv_visual >= VISUAL_AVE ) endwin();
#endif

	saveall();

	return 0;
}
