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


/* $Id: config.h,v 1.2 2006/06/06 07:33:29 santini Exp $ */

#ifndef  H_CONFIG
#define  H_CONFIG

/* genetic */

#define MAX_POPULATION_SIZE 10000
#define MAX_GENERATIONS      1000
#define MAX_RNDE_DEPTH         10
#define MAX_LENGTH_SIZE        10

#define MAX_DATAN              10
#define MAX_STACK            1000
#define MAX_FILENAME_LEN      255

#define EPHEMC_MAX           10.0
#define EPHEMC_MIN    -EPHEMC_MAX

/* Conditional code... */

#define RESIZE_TIME
#define SAVE_ADJF
#define MUTATE_RNDICROSSOVER
#define CROSSOVER_OLD
#define HAS_SIMPLIFY

/* NOT DEFINED

#define MUTATE_RNDIREPLACE
#define PERMUTED_CROSSOVER
#define ERROR_MSE
#define FAST_STATS

#define ALLOW_FUTURE

*/

typedef double fitnessv; /* valore della fitness */
typedef double constv;   /* costanti nelle espressioni */
typedef double exprv;    /* valore calcolato nella valutazione espressioni */
typedef double stackv;   /* valori conservati nello stack (per la valutazione delle espressioni) */
typedef double datav;    /* valori delle serie storiche */
typedef double probv;    /* valori [0,1] della probabilita' */
typedef double plotv;    /* valori da plottare, deve essere uguale a sopra!!! */

/* porting */

#ifndef __sgi__
#define HAS_CURSES
// #define HAS_MALLINFO
// #define HAS_PROGSHORTNAME
#endif

/* cproto */

#ifdef NOINLINE
#define my_inline
#else
#define my_inline __inline__ static
#endif

#endif /* H_CONFIG */
