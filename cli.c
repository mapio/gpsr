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
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "genetic.h"
#include "utils.h"
#include "expr.h"
#include "stats.h"
#include "signal.h"
#include "gpplots.h"

#define CMDARGSIZE 255
#define HISTORYFILE ".graphics_history"

static char historyfile[CMDARGSIZE];

static gp_port port;

static void help(const char *line);
static void quit(const char *line);
static void sndcmd(const char *line);
static void ploti(const char *line);
static void plotr(const char *line);
static void pprinti(const char *line);
static void padjfi(const char *line);
static void pprintsi(const char *line);
static void plotd(const char *line);
static void plotma(const char *line);
static void plotva(const char *line);
static void plotava(const char *line);
static void parser(char *line);

static struct command {
	char *cmd, *scmd, *help;
	void (*function)(const char *);
} commands[] = {
	{ "help",            "h",   "this help", help },
	{ "quit",            "q",   "quit the program", quit },
 	{ "ploti",           "pi",  "plot individual", ploti },
 	{ "plotr",           "pr",  "plot residuals", plotr },
	{ "printsi",         "ps",  "print simplyfied individual", pprintsi },
	{ "printi",          "pri", "print individual", pprinti },
	{ "adjfi",           "a",   "print adjusted fitness of an individual", padjfi },
	{ "plotd",           "pd",  "plot data", plotd },
	{ "plotmaxadjf",     "pm",  "plot max adjusted fitness over runs", plotma },
	{ "plotaveadjf",     "pa",  "plot adjusted fitness average over runs", plotva },
	{ "plotvaraveadjf",  "pva", "plot adjusted fitness average and variance over runs", plotava },
	{ "sndcmd",          "s",   "send a command directly to gplot engine", sndcmd },
	{ NULL, NULL, NULL, NULL }
};

static struct {
	char current_cmd[CMDARGSIZE];
	char structure_present;
	char plot_initialized;
} parserstatus = { "", 0, 0 };

/* EXTERN FUNCS */

void p_error( const char *str )
{
	fprintf( stderr, "error: %s: %s\n", parserstatus.current_cmd, str );
}

void p_perror( const char *str )
{
	char buf[CMDARGSIZE];

	sprintf( buf, "%s: %s", parserstatus.current_cmd, str );
	perror( buf );
}

/* STATIC FUNCS */

static void help(const char *line)
{
	int i;
	char arg[CMDARGSIZE], buf[CMDARGSIZE];

	if ( sscanf( line, "%*s %s", arg ) == 1 ) {
		for ( i=0; commands[i].cmd; i++ )
			if ( !strcmp( arg, commands[i].scmd ) || !strcmp( arg, commands[i].cmd ) ) {
				printf( "%s (%s): %s\n", commands[i].cmd,  commands[i].scmd,  commands[i].help );
				break; }
		if ( !commands[i].cmd ) {
			sprintf( buf, "there is no \"%s\" command", arg );
			p_error(buf);
		}
	} else {
		printf( "available commands:\n" );
		for ( i=0; commands[i].cmd; i++ )
			printf( "%s (%s): %s\n",  commands[i].cmd,  commands[i].scmd,  commands[i].help );
	}

}

static void quit(const char *line)
{
	gp_close( port );
	if ( write_history(historyfile) )
		perror( "writing history" );
	exit(EXIT_SUCCESS);
}

static void sndcmd(const char *line )
{
	char *buf;

	if ( (buf=strstr( line, " " )) ) {
		gp_tell( port, "%s\n", buf+1 );
	} else
		p_error( "no command given" );
}

/* DOMAIN SPECIFIC */

static void ploti(const char *line )
{
	int t, tt, g, n, h, S, E;
	plotv *y, *x, *dx, *dy;
	forecast r;

	if ( (n=sscanf( line, "%*s %d %d %d", &g, &S, &E ))!=EOF ) {
		if ( n == 1 ) {
			S = 0;
			E = datalen( DATA_TARGET );
		} else {
			S = max( 0, S );
			E = min( E, datalen( DATA_TARGET ) );
		}
		if ( 0 < g && g <= status.curgen ) {
			g--;
			n = E - S;
			x = ymalloc( status.length * n * sizeof(plotv) );
			y = ymalloc( status.length * n * sizeof(plotv) );
			dx = ymalloc( n * sizeof(plotv) );
			dy = ymalloc( n * sizeof(plotv) );
			tt = 0;
			for ( t = S; t < E; t++ ) {
				dy[t-S] = getdata( DATA_TARGET, t );
				dx[t-S] = t;
				r = evali( run[g].besti, t );
				for ( h = 0; h < status.length; h++ ) {
					x[tt] = t + h + status.horizon;
					y[tt] = r.v[h];
					tt++;
				}
			}
			gp_plotarray( port, GP_PLOT, GP_TITLE "\"data\"" GP_DOTS, dx, dy, n );
			gp_plotarray( port, GP_REPLOT, GP_TITLE "\"forecast\"" GP_DOTS, x, y, n * status.length );
			yfree( x );
			yfree( y );
			yfree( dx );
			yfree( dy );
		} else
			fprintf( stderr, "individual must lie in [1,%d]\n", status.curgen );
	} else
		p_error( "no individual" );
}

static void plotr(const char *line )
{
	int t, tt, g, n, h, S, E;
	plotv *y, *x;
	forecast  r;
	datav d;

	if ( (n=sscanf( line, "%*s %d %d %d", &g, &S, &E ))!=EOF ) {
		if ( n == 1 ) {
			S = 0;
			E = datalen( DATA_TARGET ) - status.length - status.horizon;
		} else {
			S = max( 0, S );
			E = min( E, datalen( DATA_TARGET ) - status.length - status.horizon );
		}
		if ( 0 < g && g <= status.curgen ) {
			g--;
			n = E - S;
			x = ymalloc( status.length * n * sizeof(plotv) );
			y = ymalloc( status.length * n * sizeof(plotv) );
			tt = 0;
			for ( t = S; t < E; t++ ) {
				r = evali( run[g].besti, t );
				for ( h = 0; h < status.length; h++ ) {
					d = getdata( DATA_TARGET, t + h + status.horizon );
					x[tt] = t + h + status.horizon;
					y[tt] = ( d - r.v[h] ) / d ;
					tt++;
				}
			}
			gp_plotarray( port, GP_PLOT, GP_TITLE "\"residual\"" GP_DOTS, x, y, n * status.length );
			yfree( x );
			yfree( y );
		} else
			fprintf( stderr, "individual must lie in [1,%d]\n", status.curgen );
	} else
		p_error( "no individual" );
}

static void pprinti(const char *line )
{
	int g;

	if ( sscanf( line, "%*s %d", &g )!=EOF ) {
		if ( 0 < g && g <= status.curgen ) {
			printi( run[g-1].besti );
		} else
			fprintf( stderr, "individual must lie in [1,%d]\n", status.curgen );
	} else
		p_error( "no individual" );
}

static void padjfi(const char *line )
{
	int g;

	if ( sscanf( line, "%*s %d", &g )!=EOF ) {
		if ( 0 < g && g <= status.curgen ) {
			printf( "adjf[i] = %e\n", adjfitnessi( run[g-1].besti ) );
		} else
			fprintf( stderr, "individual must lie in [1,%d]\n", status.curgen );
	} else
		p_error( "no individual" );
}

static void pprintsi(const char *line )
{
	int g;

	if ( sscanf( line, "%*s %d", &g )!=EOF ) {
		if ( 0 < g && g <= status.curgen ) {
#ifdef HAS_SIMPLIFY
			printsi( run[g-1].besti );
#else
			printi( run[g-1].besti );
#endif
		} else
			fprintf( stderr, "individual must lie in [1,%d]\n", status.curgen );
	} else
		p_error( "no individual" );
}

static void plotd(const char *line )
{
	gp_plotfile( port, GP_PLOT, GP_LINES, status.dataf[DATA_TARGET] );
}

static void plotma(const char *line )
{
	int i;
	plotv *y;

	y = ymalloc( status.curgen * sizeof(plotv) );
	for ( i = 0; i < status.curgen; i++ ) y[i] = run[i].maxadjf;
	gp_plotarray( port, GP_PLOT, GP_TITLE "\"max adjf\"" GP_DOTS, NULL, y, status.curgen );
	yfree( y );
}

static void plotva(const char *line )
{
	int i;
	plotv *y1, *y2;

	y1 = ymalloc( status.curgen * sizeof(plotv) );
	y2 = ymalloc( status.curgen * sizeof(plotv) );
	for ( i = 0; i < status.curgen; i++ ) y1[i] = run[i].aveadjf;
	gp_plotarray( port, GP_PLOT, GP_TITLE "\"average adjf\"" GP_DOTS, NULL, y1, status.curgen );
	yfree( y1 );
}

static void plotava(const char *line )
{
	int i;
	plotv *y1, *y2;

	y1 = ymalloc( status.curgen * sizeof(plotv) );
	y2 = ymalloc( status.curgen * sizeof(plotv) );
	for ( i = 0; i < status.curgen; i++ ) {
		y1[i] = run[i].aveadjf;
		y2[i] = run[i].varadjf;
	}
	gp_plotarray( port, GP_PLOT, GP_TITLE "\"average adjf\"" GP_DOTS, NULL, y1, status.curgen );
	gp_plotarray( port, GP_REPLOT, GP_TITLE "\"variance adjf\"" GP_DOTS, NULL, y2, status.curgen );
	yfree( y1 );
	yfree( y2 );
}

/* PARSER */

static void parser( char *line )
{
	char cmd[CMDARGSIZE];
	int i;

	if ( line != NULL ) {
		sscanf( line, "%s", cmd );
		for ( i=0; commands[i].cmd; i++ ) {
			if ( !strcmp( cmd, commands[i].scmd ) || !strcmp( cmd, commands[i].cmd ) ) {
				strcpy( parserstatus.current_cmd, commands[i].cmd );
				commands[i].function( line );
				break; }
		}
		if ( !commands[i].cmd && line[0] )
			printf( "error: there is no \"%s\" command\n", cmd );
	}
}

int main( int argc, char **argv )
{
	char file[MAX_FILENAME_LEN + 1];
	char *line, *prompt="> ";
	int i;

	if ( argc == 2 )
		ystrncpy( file, argv[1], MAX_FILENAME_LEN );
	else
		yerror( "missing filename\n" );

	restores( file );
	prints();
	restorer( file );

	for( i = 0; i < status.datan; i++ )
		readdata( status.dataf[i], i );

	port = gp_open( "400x400" );
	using_history();
	stifle_history(1000);
	sprintf( historyfile, "%s/%s", getenv("HOME"), HISTORYFILE );
	if ( !access (historyfile, R_OK) )
		if ( read_history(historyfile) )
			perror( "reading history" );
	while (1) {
		line=readline( prompt );
		if ( line ) {
			add_history( line );
			parser( line );

		} else printf( "\n" );
	}
	return 0;
}
