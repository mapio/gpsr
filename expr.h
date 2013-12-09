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


/* $Id: expr.h,v 1.1 2002/03/12 08:45:22 santini Exp $ */

#ifndef H_EXPR
#define H_EXPR

#include "config.h"

typedef unsigned char token;

typedef struct exprs {
	int tlen, vlen;
	token *tok;
	constv *val;
} expr;

#define DATA_TARGET 0
#define DATA_NEXT   1

#define assert_exprok(e) { assert( e ); assert( e->tlen > 0 ); assert( e->vlen > 0 ); \
                          assert( e->tok ); assert( e->val ); }

#define frnd(min,max) (probv)((min)+((probv)((max)-(min))*rand()/RAND_MAX))
#define irnd(min,max) (int)((min)+((probv)((max)-(min)+1.0)*rand()/(RAND_MAX+1.0)))
#define rndc(p)       (((double)rand()/RAND_MAX) < p ? 1 : 0)

#define max(a,b)      ( (a) > (b) ? (a) : (b) )
#define min(a,b)      ( (a) < (b) ? (a) : (b) )

#define printe(e)     fprinte( stdout, (e) )

#define printse(e)    fprintse( stdout, (e) )
#define sprintt(b,t)  { (b)[0]='\0'; sprintt_i( (b), (t) ); }
	  
#ifdef ALLOW_FUTURE
#define yfabs
#else 
#define yfabs fabs
#endif
	  
#define MAX_EXPR_STR       10000
#define EXPR_STR_SAFEGUARD    20
	  
/* exprfuncs */
	  
#define TOK_CONST  0
#define TOK_DATA   1
#define TOK_MAX    13

#define PS_INFIX   0 /* solo se arity == 2 */
#define PS_PREFIX  1 

#define UV_NO      0
#define UV_EPHEM   1
#define UV_DATAN   2

#define MAX_ARITY  2
	  
#define tokarity(t)   functions[(int)(t)].arity
#define tokstr(t)     functions[(int)(t)].name
#define tokeval(t)    functions[(int)(t)].eval
#define tokps(t)      functions[(int)(t)].printstyle
#define tokgp(t)      functions[(int)(t)].genprob
#define tokuv(t)      functions[(int)(t)].useval
#define toksfy(t)     functions[(int)(t)].simplify

typedef struct nodes {
	token tok;
	constv val;
	struct nodes *sn[MAX_ARITY];
} node;

struct exprfuncs {
	char *name;                /* nome, per stampa postfissa */
	unsigned char  arity;      /* arieta' */
	void (*eval)( void );      /* come si valuta */
	unsigned char printstyle;  /* come va stampato */
	probv genprob;             /* prob. cumulativa di generazione */  
	unsigned char useval;      /* che tipo di valori usa in val */
	void (*simplify)(node *);  /* come si semplifica se i figli sono const */
};

extern struct exprfuncs functions[];

#include "expr.p"
	  
#endif /* H_EXPR */
