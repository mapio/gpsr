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


/* $Id: stats.h,v 1.1 2002/03/12 08:45:22 santini Exp $ */

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

#include "stats.p"

#endif /* H_STATS */
