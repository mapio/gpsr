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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#ifdef HAS_CURSES
#include <curses.h>
#endif

#include "expr.h"
#include "utils.h"
#include "filecfg.h"
#include "stats.h"
#include "genetic.h"

typedef individual *pind;

static unsigned int pop_start;
static pind *P, P1[MAX_POPULATION_SIZE], P2[MAX_POPULATION_SIZE];
struct statuss status;


#ifdef PERMUTED_CROSSOVER
static int perm[MAX_POPULATION_SIZE];
#endif

#ifdef DEBUG
static void checkp( void )
{
	int i, j;

	assert_statusok();

	assert( P );

	for ( i = 0; i < status.popsize; i++ )
		for ( j = 0; j < status.length; j++ )
			assert_exprok( P[i]->e[j] );
}
#endif

#ifdef PERMUTED_CROSSOVER
/*
  Mette in perm[0..status.popsize-1] una permutazione casuale di 0..status.popsize-1
*/
static void permute( void )
{
	int t, j, i;

	assert_statusok();

	for ( i = 0; i < status.popsize; i++ ) perm[i] = i;

	for ( i = status.popsize - 1; i >= 0 ; i-- ) {
		j = irnd( 0, i );
		t = perm[j];
		perm[j] = perm[i];
		perm[i] = t;
	}
}
#endif

/*
  Calcola l'errore dell'individuo i al tempo t
*/
static fitnessv err_it( individual *i, int t )
{
	int j;
	fitnessv s, f;

	assert( i );
	assert( 0 <= t && t < ( datalen( DATA_TARGET ) - status.length - status.horizon) );
	assert( 0 < status.length && status.length <= MAX_LENGTH_SIZE );

	s = 1.0 / status.length;
	f = 0.0;
	for ( j = 0; j < status.length; j++ ) {
#ifdef ERROR_MSE
		s = getdata( DATA_TARGET, t + j + status.horizon ) - evale( i->e[j], t );
		s *= s;
		f += s;
#else
		{
			double e, d;

			e = evale( i->e[j], t );
			d = getdata( DATA_TARGET, t + j + status.horizon );
			f += fabs( (e - d)/d ); /* * ( 1.0 - s * j ); */
			/* f += f * f; */

			yDEBUG( "forecast = %e, data = %e\n", e, d );
		}
#endif
	}
	f /= (fitnessv)status.length;

	yDEBUG( "t = %d, err_it = %e \n", t, f );

	return f;
}


/* PUBLIC */

/*
  alloca un nuovo individuo
*/
individual *alloci( void )
{
	individual *i;

	i = (individual *)ymalloc( sizeof(individual) );

#ifdef SAVE_ADJF
	i->mod = MODY;
	i->adjf = 0.0;
#endif

	return i;
}

/*
  libera lo spazio precedentemente allocato all'individuo i
*/
void freei( individual *i )
{
	int j;

	assert( i );
	assert( 0 < status.length && status.length <= MAX_LENGTH_SIZE );

	for ( j = 0; j < status.length; j++ )
		freee( i->e[j] );
	yfree( i );
}

/*
  alloca e ritorna un nuovo individuo uguale a s
*/
individual *dupi( individual *s )
{
	int j;
	individual *i;

	assert( s );
	assert( 0 < status.length && status.length <= MAX_LENGTH_SIZE );

	i = alloci();
	for ( j = 0; j < status.length; j++ )
		i->e[j] = dupe( s->e[j] );

#ifdef SAVE_ADJF
	i->mod = s->mod;
	i->adjf = s->adjf;
#endif

	return i;
}

/*
  scrive l'individuo i sul file avente descriptor fd
*/
void writei( int fd, individual *i )
{
	int j;

	assert( fd >= 0 );
	assert( i );
	assert( 0 < status.length && status.length <= MAX_LENGTH_SIZE );

	for ( j = 0; j < status.length; j++ )
		writee( fd, i->e[j] );
#ifdef SAVE_ADJF
	ywrite( fd, &(i->adjf), sizeof(fitnessv) );
	ywrite( fd, &(i->mod), sizeof(char) );
#endif
}

/*
  alloca e ritorna l'individuo letto dal file avente descriptr fd
*/
individual *readi( int fd )
{
	individual *i;
	int j;

	assert( fd >= 0 );
	assert( 0 < status.length && status.length <= MAX_LENGTH_SIZE );

	i = alloci();
	for ( j = 0; j < status.length; j++ )
		i->e[j] = reade( fd );
#ifdef SAVE_ADJF
	yread( fd, &(i->adjf), sizeof(fitnessv) );
	yread( fd, &(i->mod), sizeof(char) );
#endif

	return i;
}

/*
   ritorna la previsione dell'individuo i al tempo t
*/
forecast evali( individual *i, int t )
{
	forecast r;
	int j;

	assert( i );
	assert( 0 < status.length && status.length <= MAX_LENGTH_SIZE );

	for ( j = 0; j < status.length; j++ )
		r.v[j] = evale( i->e[j], t );

	return r;
}

/*
  alloca e ritorna un individuo casuale con espressioni di massima profondita` md
*/
individual *rndi( int md )
{
	int j;
	individual *i;

	assert( 0 < md && md <= MAX_RNDE_DEPTH );
	assert( 0 < status.length && status.length <= MAX_LENGTH_SIZE );

	i = alloci();
	for ( j = 0; j < status.length; j++ )
		i->e[j] = rnde( md );

	return i;
}

/*
  libera lo spazio occupato dalla poplazione corrente
*/
void freep( void )
{
	int i;

	assert( 0 < status.popsize && status.popsize <= MAX_POPULATION_SIZE );
	assert( P );

	for ( i = 0; i < status.popsize; i++ )
		freei( P[i] );
}

/*
  inizializza la popolazione corrente per una data dimensione e orizzonte
*/
void initp( void )
{
	P = P1;
}

/*
  alloca e genera a caso gli individui della popolazione corrente
*/
void rndp( void )
{
	int i;

	assert_statusok();

	if ( status.seed < 0 ) status.seed = (int)time(NULL);
	srand( status.seed );
	for ( i = 0; i < status.popsize; i++ )
		P[i] = rndi( status.maxdepth );

	yDEBUG( "seed = %d\n", status.seed );
}

/*
  salva la popolazione corrente su un file di nome file
*/
void savep( char *file )
{
	int fd, i;
	char filext[MAX_FILENAME_LEN + 4];

	assert( file );
	assert_statusok();
	assert( P );

	ystrncpy( filext, file, MAX_FILENAME_LEN );
	strcat( filext, ".pop" );

	if (  ( fd = open( filext, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR ) ) < 0 )
		yperror( "can't save population to '%s'", filext );

	ywrite( fd, &status, sizeof(status) );
	for ( i = 0; i < status.popsize; i++ )
		writei( fd, P[i] );
	if ( close(fd) < 0 ) yperrorf( "closing file" );
}

/*
  alloca e legge nella popolazione corrente la popolazione sul file di nome file
  ( non libera in precedenza la popolazione corrente )
*/
void restorep( char *file )
{
	int fd, i;
	char filext[MAX_FILENAME_LEN + 4];

	assert( file );

	ystrncpy( filext, file, MAX_FILENAME_LEN );
	strcat( filext, ".pop" );

	if (  ( fd = open( filext,  O_RDONLY ) ) < 0 )
		yperror( "can't restore population from '%s'", filext );

	yread( fd, &status, sizeof(status) );
	assert_statusok();
	P = P1;
	for ( i = 0; i < status.popsize; i++ )
		P[i] = readi( fd );
	if ( close(fd) < 0 ) yperrorf( "closing file" );
}

/*
  salva lo stato nel file di nome file
*/
void saves( char *file )
{
	int fd;
	char filext[MAX_FILENAME_LEN + 4];

	assert( file );
	assert_statusok();

	ystrncpy( filext, file, MAX_FILENAME_LEN );
	strcat( filext, ".sts" );

	if (  ( fd = open( filext, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR ) ) < 0 )
		yperror( "can't save status to '%s'", filext );

	ywrite( fd, &status, sizeof(status) );
	writei( fd, status.bssfi );

	if ( close(fd) < 0 ) yperrorf( "closing file");
}

/*
  alloca e legge lo stato dal file di nome file
*/
void restores( char *file )
{
	int fd;
	char filext[MAX_FILENAME_LEN + 4];

	assert( file );

	ystrncpy( filext, file, MAX_FILENAME_LEN );
	strcat( filext, ".sts" );

	if ( ( fd = open( filext,  O_RDONLY ) ) < 0 )
		yperror( "can't restore status from '%s'", filext );

	yread( fd, &status, sizeof(status) );
	status.bssfi = readi( fd );
	assert_statusok();

	if ( close(fd) < 0 ) yperrorf( "closing file" );
}

/*
  stampa lo status
*/
void prints( void )
{
	int i;

	assert_statusok();
	printf( "\nStatus:\n" );
	printf( "----------- \n" );
	if ( status.configf[0] ) printf( "configf   : %s\n", status.configf );
	printf( "seed      : %d\n", status.seed );
	printf( "maxdepth  : %d\n", status.maxdepth );
	printf( "popsize   : %d\n", status.popsize );
	printf( "length    : %d\n", status.length );
	printf( "horizon   : %d\n", status.horizon );
	printf( "totgen    : %d\n", status.totgen );
	printf( "mutate    : %f\n", status.mutatep );
	printf( "crossover : %f\n", status.crossoverp );
	printf( "selection : %f\n", status.selectionrho );
	printf( "elitism   : %s\n", status.elitism ? "yes" : "no" );
	printf( "----------- \n" );
	for ( i = 0; i < status.datan; i++ )
		printf( "data[%3d] : %s\n", i, status.dataf[i] );
	printf( "----------- \n" );
	printf( "curgen    : %d\n", status.curgen );
	if ( status.curgen > 0 ) {
		printf( "bssfg     : %d\n", status.bssfg );
		printf( "bssfadjf  : %e (%e)\n", status.bssfadjf, 1.0/status.bssfadjf-1.0 );
		printf( "bssfi     :\n" );
#ifdef HAS_SIMPLIFY
		printsi( status.bssfi );
#else
		printi( status.bssfi );
#endif
	}
	printf( "----------- \n\n" );
}

#if defined(HAS_CURSES) && defined(INTERACTIVE)
/*
  mostra l'individuo i (con le espresisoni semplificate)
*/
void displayi( individual *i )
{
	int j;

	assert( i );
	assert( 0 < status.length && status.length <= MAX_LENGTH_SIZE );

	for ( j = 0; j < status.length; j++ ) {
		printw( "data[0](t+%d) = ", j + status.horizon );
		displaye( i->e[j] );
	}
}
#endif

/*
  legge il file di configurazione, se esiste, e modifica lo stato di conseguenza
  ( mette anche il status.configf il nome del file se lo ha trovato )
*/
extern char *strdup(const char *);

void getgpsrcfg( char *file )
{
	int i;
	char *rv;
	readparams( file );

	yDEBUG( "reading config file '%s'\n", file );

	for ( i = 0; i < TOK_MAX; i++ )
		if ( ( rv = findparam( strcat( strdup( "gp_" ), tokstr( i ) ) ) ) ) {
			tokgp( i ) = atof( rv );
			yDEBUG( "setting gp of '%s' to %e\n", tokstr( i ), tokgp( i ) );
		}

	if ( ( rv = findparam( "popsize" ) ) ) {
		status.popsize = atoi( rv );
		if ( status.popsize <= 0 || status.popsize > MAX_POPULATION_SIZE )
			yerror( "popsize must be in [1,%d]\n", MAX_POPULATION_SIZE );
	}

	if ( ( rv = findparam( "horizon" ) ) ) {
		status.horizon = atoi( rv );
		if ( status.horizon <= 0  )
			yerror( "horizon must be >=1\n" );
	}

	if ( ( rv = findparam( "length" ) ) ) {
		status.length = atoi( rv );
		if ( status.length <= 0 || status.length > MAX_LENGTH_SIZE )
			yerror( "length must be in [1,%d]\n", MAX_LENGTH_SIZE );
	}

	if ( ( rv = findparam( "maxdepth" ) ) ) {
		status.maxdepth = atoi( rv );
		if ( status.maxdepth <= 0 || status.maxdepth > MAX_RNDE_DEPTH )
			yerror( "maxdepth must be in [1,%d]\n", MAX_RNDE_DEPTH );
	}

	if ( ( rv = findparam( "seed" ) ) ) {
		status.seed = atoi( rv );
		if ( status.seed <= 0 )
			yerror( "seed must be positive\n" );
	}

	if ( ( rv = findparam( "totgen" ) ) ) {
		status.totgen = atoi( rv );
		if ( status.totgen <= 0 || status.totgen > MAX_GENERATIONS )
			yerror( "generations must be in [1,%d]\n", MAX_GENERATIONS );
	}

	if ( ( rv = findparam( "mutatep" ) ) ) {
		status.mutatep = atof( rv );
		if ( status.mutatep < 0.0 || status.mutatep > 1.0 )
			yerror( "mutatep must be in [0,1]\n" );
	}

	if ( ( rv = findparam( "crossoverp" ) ) ) {
		status.crossoverp = atof( rv );
		if ( status.crossoverp < 0.0 || status.crossoverp > 1.0 )
			yerror( "crossoverp must be in [0,1]\n" );
	}

	if ( ( rv = findparam( "elitism" ) ) ) {
		status.elitism = ( atoi( rv ) == 1 );
	}

	if ( ( rv = findparam( "saveas" ) ) ) {
		ystrncpy( status.savef, rv, MAX_FILENAME_LEN );
	}

	while ( ( rv = findparam( "data" ) ) ) {
		if ( status.datan < MAX_DATAN )
			readdata( rv, status.datan++ );
		else
			yerror( "you can read at most %d datafile(s)\n", MAX_DATAN );
	}

	if ( ( rv = findparam( "selectionrho" ) ) ) {
		status.selectionrho = atof( rv );
		if ( status.selectionrho < 0.0 || status.selectionrho > 1.0 )
				yerror( "selectionrho must be in {0} (fitness proportional), or (0,1]\n" );
	}

	ystrncpy( status.configf, file, MAX_FILENAME_LEN );
}


/*
  calcola alcune statistice circa la popolazione corrente
  ( in particolare alloca il miglior individuo )
*/
struct rstats statisticsp( void )
{
	struct rstats s;
	fitnessv f = 0.0;
	int i;
#ifndef FAST_STATS
	int j, vlen, tlen, vtot, ttot;
#endif

	assert_statusok();
	assert( P );

#ifndef FAST_STATS
	vtot = ttot = 0;
	s.maxval = s.maxtok = -1;
	s.minval = s.mintok = INT_MAX;
#endif
	s.maxadjf = -1.0;
	s.aveadjf = 0.0;
	s.varadjf = 0.0;

	for ( i = 0; i < status.popsize; i++ ) {
		f = adjfitnessi( P[i] );
		s.aveadjf += f;
		s.varadjf += f * f;
		if ( s.maxadjf < f ) {
			s.maxadjf = f;
			s.besti = P[i];
		}
#ifndef FAST_STATS
		for ( j = 0; j < status.length; j++ ) {
			vlen = P[i]->e[j]->vlen;
			tlen = P[i]->e[j]->tlen;
			vtot += vlen;
			ttot += tlen;
			if ( s.maxval < vlen ) s.maxval = vlen;
			if ( s.minval > vlen ) s.minval = vlen;
			if ( s.maxtok < tlen ) s.maxtok = tlen;
			if ( s.mintok > tlen ) s.mintok = tlen;
		}
#endif
	}
	s.besti = dupi( s.besti );
	s.aveadjf = s.aveadjf / status.popsize;
	s.varadjf = ( s.varadjf - s.aveadjf * s.aveadjf * status.popsize ) / ( status.popsize - 1 );
#ifndef FAST_STATS
	s.avetok = ttot / ( status.popsize * status.length );
	s.aveval = vtot / ( status.popsize * status.length );
#endif
	s.usedmem = ymallocuse();

	return s;
}

/*
  stampa l'individuo i sullo stream
*/
void fprinti( FILE *stream, individual *i )
{
	int j;

	assert( i );
	assert( 0 < status.length && status.length <= MAX_LENGTH_SIZE );

	for ( j = 0; j < status.length; j++ ) {
		fprintf( stream, "e[%d] =", j );
		fprinte( stream, i->e[j] );
	}
}

#ifdef HAS_SIMPLIFY
/*
  stampa l'individuo i sullo stream (con le espressioni semplificate)
*/
void fprintsi( FILE *stream, individual *i )
{
	int j;

	assert( i );
	assert( 0 < status.length && status.length <= MAX_LENGTH_SIZE );

	for ( j = 0; j < status.length; j++ ) {
		fprintf( stream, "data[0](t+%d) = ", j + status.horizon );
		fprintse( stream, i->e[j] );
	}
}
#endif

/*
  calcola l'adjusted fitness dell'individuo i
*/
fitnessv adjfitnessi( individual *i )
{
	int t;
	fitnessv f;

	assert( i );
	assert( 0 < status.length && status.length <= MAX_LENGTH_SIZE );

#ifdef SAVE_ADJF
	if ( i->mod == MODN ) return i->adjf;
#endif

	f = 0.0;
	for ( t = 0; t < ( datalen( DATA_TARGET ) - status.length -  status.horizon ); t++ ) {
		f += err_it( i, t );
	}
	f /= (fitnessv)( datalen( DATA_TARGET ) - status.length -  status.horizon );

	f = 1.0 / ( 1.0 + f );
#ifdef SAVE_ADJF
	i->mod = MODN;
	i->adjf = f;
#endif

	yDEBUG( "(adjfitnessi): f = %e\n", f );

	return f;
}

/*
  effettua il crossover sulla popolazione corrente
*/
void crossoverp( void )
{
	int i;
#ifdef CROSSOVER_OLD
	expr *e1, *e2;
#else
	int j;
#endif

	assert_statusok();
	assert( P );

#ifdef PERMUTED_CROSSOVER
	permute();
#endif
	for ( i = pop_start; i < status.popsize-1; i+=2 )
		if ( rndc( status.crossoverp ) ) {
#ifdef CROSSOVER_OLD
			e1 = P[i]->e[ irnd( 0, status.length - 1 ) ];
			e2 = P[i+1]->e[ irnd( 0, status.length - 1 ) ];
			crossovere( e1, e2 );
#else
			for ( j = 0; j < status.length-1; j++ )
				crossovere( P[i]->e[j], P[i+1]->e[j] );
#endif
#ifdef SAVE_ADJF
			P[i]->mod = MODY;
			P[i+1]->mod = MODY;
#endif
		}

}


/*
  effettua la mutazione sulla popolazione corrente
*/
#if defined(MUTATE_RNDICROSSOVER) && defined(MUTATE_RNDIREPLACE)
#error MUTATE_RNDICROSSOVER and MUTATE_RNDIREPLACE both defined
#endif
void mutatep( void )
{
	int i;
	expr *e1, *e2;

	assert_statusok();
	assert( P );

	for ( i = pop_start; i < status.popsize; i++ )
		if ( rndc( status.mutatep ) ) {
#ifdef MUTATE_RNDICROSSOVER
			if ( rndc( 0.5 ) ) {
				e1 = P[i]->e[ irnd( 0, status.length - 1 ) ];
				e2 = rnde( status.maxdepth );
				crossovere( e1, e2 );
				freee( e2 );
			} else {
#else
#ifdef MUTATE_RNDIREPLACE
		    if ( rndc( 0.5 ) ) {
				e1 = P[i]->e[ irnd( 0, status.length - 1 ) ];
				e2 = rnde( status.maxdepth );
				e1 = e2 ;
				freee( e1 );
			} else {
#endif
#endif
				e1 = P[i]->e[ irnd( 0, status.length - 1 ) ];
				e2 = P[i]->e[ irnd( 0, status.length - 1 ) ];
				crossovere( e1, e2 );
#if defined(MUTATE_RNDICROSSOVER) || defined(MUTATE_RNDIREPLACE)
			}
#endif
#ifdef SAVE_ADJF
			P[i]->mod = MODY;
#endif
    }
}

/* funzioni di supporto per selectionp */

struct fits {
	fitnessv f;
	int p; };

static int compar( const void *a, const void *b )
{
	fitnessv fa, fb;

	fa = ((struct fits *)a)->f;
	fb = ((struct fits *)b)->f;

	if ( fa < fb )
		return 1;
	else if ( fa > fb )
		return -1;
	else
		return 0;
}


/*
  produce una nuova generazione;

  se status.selectionrho > 0.0, replicando una frazione
       degli individui migliori della generazione precedente
  altrimenti con un metodo "fitness proportional"
*/
void selectionp( void )
{
	int i, j;
	pind *Q;

	assert_statusok();
	assert( P );

	if ( status.selectionrho > 0.0 ) {

		struct fits f[MAX_POPULATION_SIZE];
		int selsize;

		for ( i = 0; i < status.popsize; i++ ) {
			f[i].f = adjfitnessi( P[i] );
			f[i].p = i;
		}

		qsort( f, status.popsize, sizeof(struct fits), compar );

		Q = NULL;
		if ( P == P1 )
			Q = P2;
		if ( P == P2 )
			Q = P1;
		assert( Q );

		i = pop_start;
		selsize =  max( 1, status.popsize * status.selectionrho );
		while ( i < status.popsize )
			for ( j = 0; j < selsize && i < status.popsize; j++ )
				Q[i++] = dupi( P[ f[j].p ] );

		if ( status.elitism ) Q[0] = dupi( P[0] );

	} else /* status.selectionrho == 0.0 */ {

		fitnessv s, cf, cnf[MAX_POPULATION_SIZE];
		probv p;
		int bi = -1;
		fitnessv bf = -1.0;

		s = 0.0;
		for ( i = 0; i < status.popsize; i++ ) {
			cnf[i] = adjfitnessi( P[i] );
			if ( status.elitism && cnf[i] > bf ) {
				bf = cnf[i];
				bi = i;
			}
			s += cnf[i];
		}
		cf = 0.0;
		for ( i = 0; i < status.popsize; i++ ) {
			cf += cnf[i]/s;
			cnf[i] = cf;
		}

		yDEBUG( "cf = %e\n", cf );

		assert( 0.0 <= cf && cf <= 1.0 + FLT_EPSILON );

		Q = NULL;
		if ( P == P1 )
			Q = P2;
		if ( P == P2 )
			Q = P1;
		assert( Q );

		for ( i = pop_start; i  < status.popsize; i++ ) {
			p = frnd( 0.0, 1.0 );
			yDEBUG( "p = %f\n", p );

			for ( j = 0; j < status.popsize && cnf[j] < p; j++ )
				;
			yDEBUG( "choosen %i\n", j );
			Q[i] = dupi( P[j] );
		}

		if ( status.elitism ) Q[0] = dupi( P[bi] );
	}

#ifdef DEBUG
	checkp();
#endif
	for ( i = 0; i < status.popsize; i++ ) freei( P[i] );
	P = Q;
#ifdef DEBUG
	checkp();
#endif

}

/*
  esegue cicli di selectionp, crossoverp e mutatep
  alloca e calcola le statistiche
  ( ma non le libera in precedenza, tranne che il bssfi
    di cui conserva una sola copia )
*/
void generationp( void )
{
	int i;

	assert_statusok();

	status.bssfadjf = -1.0;
	status.bssfi = NULL;
	status.bssfg = -1;
	status.curgen = 0;

	if ( status.elitism ) pop_start = 1; else pop_start = 0;

	for ( i = 0; i < status.totgen; i++ ) {
		selectionp();
		if ( status.crossoverp > 0.0 ) crossoverp();
		if ( status.mutatep > 0.0 ) mutatep();
		run[i] = statisticsp();
		if ( run[i].maxadjf > status.bssfadjf ) {
			status.bssfadjf = run[i].maxadjf;
			if ( status.bssfi ) freei( status.bssfi );
			status.bssfi = dupi( run[i].besti );
			status.bssfg = i + 1;
		}
		status.curgen = i + 1;
		if ( gv_visual == VISUAL_MIN )
			printf( "%d %e\n", status.curgen, status.bssfadjf );
#ifdef INTERACTIVE
		if ( gv_visual >= VISUAL_AVE )
			displays();
#endif
	}
}

