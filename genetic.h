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


#ifndef H_GENETIC
#define H_GENETIC

#include "config.h"

#include <stdio.h>

#include "expr.h"

#define MODN 0
#define MODY 1

typedef struct individuals {
  expr *e[MAX_LENGTH_SIZE];
#ifdef SAVE_ADJF
  fitnessv adjf;
  char mod;
#endif
} individual;

typedef struct forecasts {
	exprv v[MAX_LENGTH_SIZE];
} forecast;

struct statuss {
	int seed;           /* il seed con cui e` stato inizializzato il generatore */
	int maxdepth;       /* massima profondita` */
	int popsize;        /* dimensione della popolazione */
	int horizon;        /* l'orizzonte della previsione */
	int length;         /* la lunghezza della previsione */
	int totgen;         /* il numero totale di generazioni */
	int curgen;         /* la generazione corrente */
	probv mutatep;      /* probabilita' di mutazione */
	probv crossoverp;   /* probabilita' di crossover */
	probv selectionrho; /* frazione di selezione */
	int elitism;        /* {0,1}, a seconda che {no, si} */
	int datan;          /* il numero e i nomi di datafile letti */
	char dataf[MAX_DATAN][MAX_FILENAME_LEN + 1];
	char configf[MAX_FILENAME_LEN + 1 ]; /* file delle conf */
	char savef[MAX_FILENAME_LEN + 1 ];   /* file dove salva tutto */
	int bssfg;          /* la generazione del bssf */
	fitnessv bssfadjf;  /* migliore adjf sinora vista */
	individual *bssfi;  /* miglior individuo sinora visto */
};

extern struct statuss status;

#define printi(i)   fprinti(stdout,(i))
#define printsi(i)  fprintsi(stdout,(i))

#define assert_statusok() \
{ assert( 0 < status.popsize && status.popsize <= MAX_POPULATION_SIZE ); \
  assert( 0 < status.length && status.length <= MAX_LENGTH_SIZE ); \
  assert( 0 < status.datan && status.datan <= MAX_DATAN ); \
  assert( 0 <= status.curgen && status.curgen <= MAX_GENERATIONS ); }

/* prototypes */

individual *alloci(void);
void freei(individual *i);
individual *dupi(individual *s);
void writei(int fd, individual *i);
individual *readi(int fd);
forecast evali(individual *i, int t);
individual *rndi(int md);
void freep(void);
void initp(void);
void rndp(void);
void savep(char *file);
void restorep(char *file);
void saves(char *file);
void restores(char *file);
void prints(void);
void displayi(individual *i);
void getgpsrcfg(char *file);
struct rstats statisticsp(void);
void fprinti(FILE *stream, individual *i);
void fprintsi(FILE *stream, individual *i);
fitnessv adjfitnessi(individual *i);
void crossoverp(void);
void mutatep(void);
void selectionp(void);
void generationp(void);

#endif /* H_GENETIC */
