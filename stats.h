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

#ifndef H_STATS
#define H_STATS

#include "config.h"
#include "expr.h"
#include "genetic.h"

struct rstats {
	int usedmem;       /* memoria utilizzata */
	int mintok;        /* numero minimo di token */
	int minval;        /* numero minimo di valori */
	int avetok;        /* numero medio di token */
	int aveval;        /* numero medio di valori */
	int maxtok;        /* numero massimo di token */
	int maxval;        /* numero massimo di valori */
	fitnessv maxadjf;  /* massima adjf */
	fitnessv aveadjf;  /* adjf media */
	fitnessv varadjf;  /* varianza adjf */
	individual *besti; /* miglior individuo della popolazione */
};

extern struct rstats run[MAX_GENERATIONS];     /* le statistiche per generazione */
extern int gv_visual, gv_savepop, gv_saverun;

#define VISUAL_OFF 0
#define VISUAL_MIN 1
#define VISUAL_AVE 2
#define VISUAL_MAX 3

/* prototypes */

void freemisc(void);
void saver(char *file);
void restorer(char *file);
void printr(int i);
void displays(void);

#endif /* H_STATS */

