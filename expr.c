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


/* $Id: expr.c,v 1.1 2002/03/12 08:45:22 santini Exp $ */

#define _GNU_SOURCE /* gestione FP exceptions e isfinite */

#include "config.h"

#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <time.h>
#ifdef __USE_ISOC9X
#include <fenv.h>
#endif
#ifdef HAS_CURSES
#include <curses.h>
#endif

#include "utils.h"
#include "genetic.h"
#include "expr.h"
#include "stats.h"

#define EXPRLEN 100

static exprv *evalvp, *evalsp, evalstack[MAX_STACK];
static unsigned int evaltime;

#include "expr.ps"

#ifdef HAS_SIMPLIFY
struct exprfuncs functions[] = {
	{ "const", 0, f_const, PS_PREFIX, 0.0, UV_EPHEM,  NULL },
	{ "data",  1, f_data,  PS_PREFIX, 0.0, UV_DATAN,  s_data }, /* se gp==0.0, mette equiprob */ 
	{ "+",     2, f_add,   PS_INFIX,  0.0, UV_NO,     s_gensimp },
	{ "-",     2, f_sub,   PS_INFIX,  0.0, UV_NO,     s_gensimp },
	{ "*",     2, f_mul,   PS_INFIX,  0.0, UV_NO,     s_gensimp },
	{ "/",     2, f_div,   PS_INFIX,  0.0, UV_NO,     s_gensimp }, 
	{ "^",     2, f_pow,   PS_INFIX,  0.0, UV_NO,     s_gensimp },
	{ "sqrt",  1, f_sqrt,  PS_PREFIX, 0.0, UV_NO,     s_gensimp },
	{ "sin",   1, f_sin,   PS_PREFIX, 0.0, UV_NO,     s_gensimp }, 
	{ "log",   1, f_log,   PS_PREFIX, 0.0, UV_NO,     s_gensimp }, 
	{ "exp",   1, f_exp,   PS_PREFIX, 0.0, UV_NO,     s_gensimp }, 
	{ "diff",  1, f_diff,  PS_PREFIX, 0.0, UV_DATAN,  s_diff },
	{ "movav", 1, f_movav, PS_PREFIX, 0.0, UV_DATAN,  s_movav }
};
#else
struct exprfuncs functions[] = {
	{ "const", 0, f_const, PS_PREFIX, 0.0, UV_EPHEM },
	{ "data",  1, f_data,  PS_PREFIX, 0.0, UV_DATAN }, /* se gp==0.0, mette equiprob */ 
	{ "+",     2, f_add,   PS_INFIX,  0.0, UV_NO },
	{ "-",     2, f_sub,   PS_INFIX,  0.0, UV_NO },
	{ "*",     2, f_mul,   PS_INFIX,  0.0, UV_NO },
	{ "/",     2, f_div,   PS_INFIX,  0.0, UV_NO }, 
	{ "^",     2, f_pow,   PS_INFIX,  0.0, UV_NO },
	{ "sqrt",  1, f_sqrt,  PS_PREFIX, 0.0, UV_NO },
	{ "sin",   1, f_sin,   PS_PREFIX, 0.0, UV_NO }, 
	{ "log",   1, f_log,   PS_PREFIX, 0.0, UV_NO }, 
	{ "log",   1, f_exp,   PS_PREFIX, 0.0, UV_NO }, 
	{ "diff",  1, f_diff,  PS_PREFIX, 0.0, UV_DATAN },
	{ "movav", 1, f_movav, PS_PREFIX, 0.0, UV_DATAN }
};
#endif

/* DATA begin */

typedef struct datas {
	int len;
	datav *val;
#ifdef RESIZE_TIME
	double rat;
#endif
} data;

#define DATALEN 100

static data DS[MAX_DATAN];

my_inline datav getdata( int ds, int t )
{
	assert( 0 <= ds && ds < status.datan );
	assert( DS[ds].val );
	
	yDEBUG( "ds=%d, t=%d\n", ds, t );

	if ( t > 0 && t < DS[ds].len )
#ifdef RESIZE_TIME
		return DS[ds].val[ (int)floor(DS[ds].rat * t) ];
#else
		return DS[ds].val[t];
#endif
	else
		return 0.0;
}

my_inline int datalen( int ds )
{
	assert( 0 <= ds && ds < status.datan );
	assert( DS[ds].val );
	
	return DS[ds].len;
}

void readdata( char *file, int ds )
{
	FILE *infile;
	data *d;
	int dmax;
	datav val;

	assert( file );
	assert( 0 <= ds && ds < MAX_DATAN );

	d = &(DS[ds]);
	if ((infile = fopen(file, "r")) == NULL) 
		yperror( "can't get data '%s'", file );

	d->len = 0;
	d->val = (datav *)ymalloc( DATALEN * sizeof(datav) );
	dmax = DATALEN;

	while ( !feof(infile) ) {
		if ( fscanf( infile, "%lf\n", &val ) != 1 ) 
			yerror("file format error in '%s'\n", file );
		d->val[ d->len++ ] = val;
		if ( d->len == dmax ) {
			dmax *= 2;
			d->val = (datav *) yrealloc( d->val, dmax * sizeof(datav) );
		}
	}
	d->val = (datav *)yrealloc( d->val, d->len * sizeof(datav) );
#ifdef RESIZE_TIME
	d->rat = d->len / (double)DS[DATA_TARGET].len;
#endif

	ystrncpy( status.dataf[ds], file, MAX_FILENAME_LEN  );

	yDEBUG( "file=%s ds=%d, values=%d\n", file, ds, d->len );

	fclose(infile);
}

/* DATA end */

/* FUNCS begin */

static void f_const( void )
{
	*(evalsp+1) = *(evalvp++);			
}

static void f_data( void )
{
	*(evalsp) = (exprv)getdata( (int)*(evalvp++) , evaltime-(int)yfabs((double)*(evalsp)) );
}

static void f_add( void )
{
	*(evalsp-1) = *(evalsp-1) + *(evalsp);
}

static void f_sub( void )
{
	*(evalsp-1) = *(evalsp-1) + *(evalsp);
}

static void f_mul( void )
{
	*(evalsp-1) = *(evalsp-1) + *(evalsp);
}

static void f_div( void )
{
	*(evalsp-1) = *(evalsp) != 0.0 ? *(evalsp-1) / *(evalsp) : 0.0;
}

static void f_pow( void )
{
	*(evalsp-1) = *(evalsp-1) > 0.0 ? (exprv)pow( (double)*(evalsp-1), (double)*(evalsp) ) : 0.0;
#ifdef __USE_ISOC9X
	if ( !isfinite( *(evalsp-1) ) ) *(evalsp-1) = 0.0;
#endif
}

static void f_sqrt( void )
{
	*(evalsp) = *(evalsp) > 0.0 ? (exprv)sqrt( (double)*(evalsp) ) : 0.0;
}

static void f_sin( void )
{
	*(evalsp) = (exprv)sin( (double)*(evalsp) );
}

static void f_log( void )
{
	*(evalsp) = *(evalsp) > 0.0 ? (exprv)log( (double)*(evalsp) ) : 0.0;
}

static void f_exp( void )
{
	*(evalsp) = (exprv)exp( (double)*(evalsp) );
#ifdef __USE_ISOC9X
	if ( !isfinite( *(evalsp) ) ) *(evalsp) = 0.0;
#endif
}

static void f_diff( void )
{
	*(evalsp) = (exprv)(
		getdata( (int)*(evalvp) , evaltime ) -
		getdata( (int)*(evalvp) , evaltime-(int)fabs((double)*(evalsp)) ) );
	evalvp++;
}

static void f_movav( void )
{
	int i, len, ds;
	datav ma;

	len = 1 + (int)fabs((double)*(evalsp));
	ds = (int)*(evalvp++);
	ma = 0.0;
	for ( i = evaltime - len + 1; i <= evaltime ; i++ )
		ma += getdata( ds , i ); 
	*(evalsp) = (exprv)(ma / len);
}

/* FUNCS end */

/* EXPR begin */

static expr *comp( token tok, expr *se[] )
{
	expr *e;
	int i, tlen, vlen;
	
	assert( tok < TOK_MAX );
	
	tlen = vlen = 0;
	for ( i = 0; i < tokarity( tok ); i++ ) {
		assert_exprok( se[i] );
		tlen += se[i]->tlen;
		vlen += se[i]->vlen;
	}
	e = alloce( tlen + 1, vlen + ( tokuv( tok ) ? 1 : 0 ) );
	tlen = 0;
	for ( i = 0; i < tokarity( tok ); i++ ) {
		memcpy( e->tok + tlen, se[i]->tok, se[i]->tlen * sizeof(token) );
		tlen += se[i]->tlen;
	}
	e->tok[ tlen ] = tok;
	vlen = 0;
	for ( i = 0; i < tokarity( tok ); i++ ) {
		memcpy( e->val + vlen, se[i]->val, se[i]->vlen * sizeof(constv) );
		vlen += se[i]->vlen;
	}
	switch( tokuv( tok ) ) {
	case UV_NO:
		break;
	case UV_EPHEM:
		e->val[ vlen ] = (constv)frnd( EPHEMC_MIN, EPHEMC_MAX );
		break;
	case UV_DATAN:
		e->val[ vlen ] = (constv)irnd( 0, status.datan - 1 );
		break;
	default:
		yerrorf( "unknown useval" );
		break;
	}

	return e;
}

static int depth;

static expr *rnde_i( void )
{
	expr *e, *se[MAX_ARITY];
	token tok;
	probv p;
	int i;

	depth--;

	p = frnd( 0.0, 1.0 );
	for ( tok = 0; tok < TOK_MAX && tokgp( tok ) < p; tok++ ) /*vuota*/;
	
	if ( tok == TOK_CONST || depth < 0 ) {
		
		e = alloce( 1, 1 );
		e->tok[0] = TOK_CONST;
		e->val[0] = (constv)frnd( EPHEMC_MIN, EPHEMC_MAX );
		
	} else {

		for ( i = 0; i < tokarity( tok ); i++ ) se[i] =  rnde_i();
		e = comp( tok, se );
		for ( i = 0; i < tokarity( tok ); i++ ) freee( se[i] );
		
	}
	
	assert( e );
	
	return e;
}

static int sub( expr *e, int p )
{
	int l, s;
	
	assert_exprok( e );
	assert( 0 <= p &&  p < e->tlen );
	
	l = p;
	if ( e->tok[p] != TOK_CONST ) {
		assert( e->tlen != 1 );
		s = tokarity( e->tok[p] );
		do {
			--l;
			assert( l >= 0 );
			if ( e->tok[l] == TOK_CONST ) {
				s--;
			} else {
				s += tokarity( e->tok[l] ) - 1;
			}		
		} while ( s > 0 );	
	}
	
	assert( 0<= l && l <= p );
	
	yDEBUG( "p=%d, l=%d\n", p, l );
	
	return l;
}

static void mov( expr *d, expr *s )
{
	void *t;
	
	assert_exprok( d );
	assert_exprok( s );
	
	t = (void *)d->tok;
	d->tok = s->tok;
	d->tlen = s->tlen;
	yfree( t );
	
	t = (void *)d->val;
	d->val = s->val;
	d->vlen = s->vlen;
	yfree( t );
	
	/* free solo della struttura, non di val e tok che ho copiato! */
	yfree( s ); 
}

/* PUBLIC FUNCS */

void equiprobfunc( void )
{
	int i;
	probv p, q;

	p = 1.0 / TOK_MAX;
	q = 1.0;
	for ( i = TOK_MAX - 1; i >= 0; i-- ) {
		tokgp( i ) = q;
		q -= p;
	}	
}

expr *alloce( int tlen, int vlen )
{
	expr *e;

	assert( tlen > 0 );
	assert( vlen > 0 );

	e = (expr *)ymalloc( sizeof(expr) );
	e->tlen = tlen;
	e->vlen = vlen;
	e->tok = (token *)ymalloc( tlen * sizeof(token) );
	e->val = (constv *)ymalloc( vlen * sizeof(constv) );
	
	return e;
}

void freee( expr *e )
{
	assert_exprok( e );

	yfree( e->tok );
	yfree( e->val );
	yfree( e );
}

expr *dupe( expr *e )
{
	expr *c;

	assert_exprok( e );

	c = alloce( e->tlen, e->vlen );
	memcpy( c->tok, e->tok, e->tlen * sizeof(token) );
	memcpy( c->val, e->val, e->vlen * sizeof(constv) );
	
	return c;
}

expr *rnde( int md ) 
{
	expr *e;
	
	assert( 0 < md && md <= MAX_RNDE_DEPTH );
	
	depth = md;
	e = rnde_i();
	
#ifdef DEBUG
	yDEBUG( "e = " );
	fprinte( stderr, e );
#endif
	
	return e;
}

void crossovere( expr *e1, expr *e2 )
{
	int i;
	int l1, r1, l2, r2;
	token *nt1, *nt2;
	constv *v1, *v2, *nv1, *nv2;
	expr *ne1, *ne2;
	
	assert_exprok( e1 );
	assert_exprok( e2 );

#ifdef DEBUG
	yDEBUG( "\n" );
	fprintf( stderr, "  e1 = " ); fprinte( stderr, e1 ); 
	fprintf( stderr, "  e2 = " ); fprinte( stderr, e2 ); 
#endif
	
	r1 = irnd( 0, e1->tlen-1 );
	l1 = sub( e1, r1 );
	r2 = irnd( 0, e2->tlen-1 );
	l2 = sub( e2, r2 );

	i = e1->tlen - ( r1 - l1 + 1 ) + ( r2 - l2 + 1 );
	ne1 = alloce( i, i );

	i = e2->tlen - ( r2 - l2 + 1 ) + ( r1 - l1 + 1 );
	ne2 = alloce( i, i );

	v1 = e1->val;
	nv1 = ne1->val;
	nt1 = ne1->tok;

	v2 = e2->val;
	nv2 = ne2->val;
	nt2 = ne2->tok;

/*
  
  e1    1234567890
           ^   ^
		   l1  r1


  e2    abcdefghi
          ^  ^ 
          l2 r2

  ne1   123cdef90
  ne2   ab45678ghi
  
*/
	
	for ( i = 0; i < l1; i++ ) {
		*(nt1++) = e1->tok[i];
		if ( tokuv( e1->tok[i] ) ) *(nv1++) = *(v1++);
	}

	for ( i = 0; i < l2; i++ ) {
		*(nt2++) = e2->tok[i];
		if ( tokuv( e2->tok[i] ) ) *(nv2++) = *(v2++);
	}
	
	for ( i = l2; i <= r2; i++ ) {
		*(nt1++) = e2->tok[i];
		if ( tokuv( e2->tok[i] ) ) *(nv1++) = *(v2++);
	}
	
	for ( i = l1; i <= r1; i++ ) {
		*(nt2++) = e1->tok[i];
		if ( tokuv( e1->tok[i] ) ) *(nv2++) = *(v1++);
	}
	
	for ( i = r1 + 1; i < e1->tlen ; i++ ) {
		*(nt1++) = e1->tok[i];
		if ( tokuv( e1->tok[i] ) ) *(nv1++) = *(v1++);
	}
	
	for ( i = r2 + 1; i < e2->tlen ; i++ ) {
		*(nt2++) = e2->tok[i];
		if ( tokuv( e2->tok[i] ) ) *(nv2++) = *(v2++);
	}
	
#ifdef DEBUG
	fprintf( stderr, "  ne1 = " ); fprinte( stderr,  ne1 ); 
	fprintf( stderr, "  ne2 = " ); fprinte( stderr,  ne2 ); 
#endif
	
	assert( ne1->tlen == ( nt1 - ne1->tok ) );
	assert( ne2->tlen == ( nt2 - ne2->tok ) );
	
	ne1->vlen = ( nv1 - ne1->val );
	assert( ne1->vlen <= ne1->tlen );
	ne1->val = (constv *)yrealloc( ne1->val, ne1->vlen * sizeof(constv) );
	
	ne2->vlen = ( nv2 - ne2->val );
	assert( ne2->vlen <= ne2->tlen );
	ne2->val = (constv *)yrealloc( ne2->val, ne2->vlen * sizeof(constv) );
	
	mov( e1, ne1 );
	mov( e2, ne2 );
}

void writee( int fd, expr *e )
{
	assert( fd >= 0 );
	assert_exprok( e );
	
	ywrite( fd, &(e->tlen), sizeof(int) );
	ywrite( fd, &(e->vlen), sizeof(int) );
	ywrite( fd, e->tok, e->tlen * sizeof(token) );
	ywrite( fd, e->val, e->vlen * sizeof(constv) );
}

expr *reade( int fd )
{
	expr *e;
	int tlen, vlen;
	
	assert( fd >= 0 );

	yread( fd, &tlen, sizeof(int) );
	yread( fd, &vlen, sizeof(int) );
	assert( tlen > 0 && vlen > 0 );
	e = alloce( tlen, vlen );
	yread( fd, e->tok, e->tlen * sizeof(token) );
	yread( fd, e->val, e->vlen * sizeof(constv) );
	
	return e;
}

char *sprinte( char *buf, int len, expr *e )
{
	int i, j, o;
	
	assert( buf );
	assert_exprok( e );
	
	j = 0;
	o = 0;
	for ( i = 0; i < e->tlen; i++ ) {
		assert( o < len - EXPR_STR_SAFEGUARD );
		if ( e->tok[i] == TOK_CONST ) {
			o += sprintf( buf + o, " %e ", e->val[ j++ ] );			
		} else 	switch( tokuv( e->tok[i] ) ) {
		case UV_NO:
			o += sprintf( buf + o, " %s ", tokstr(e->tok[i]) );
			break;
		case UV_EPHEM:
			o += sprintf( buf + o, " %s[%e] ", tokstr( e->tok[i] ), e->val[ j++ ] );
			break;
		case UV_DATAN:
			o += sprintf( buf + o, " %s[%d] ", tokstr( e->tok[i] ), (int)e->val[ j++ ] );
			break;
		default:
			yerrorf( "unknown useval" );
			break;
		}
	}

	return buf;
}

void fprinte( FILE *out, expr *e )
{
	char buf[MAX_EXPR_STR];
	
	assert( out );
	assert_exprok( e );
	
	sprinte( buf, MAX_EXPR_STR, e );
	fprintf( out, "%s\n", buf );
}

exprv evale( expr *e, unsigned int time )
{
	int i;
	token c;

	assert_exprok( e );
	assert( 0 < status.datan && status.datan <= MAX_DATAN );
	
#ifdef DEBUG
	yDEBUG( "e =" ); fprinte( stderr, e ); 
#ifdef __USE_ISOC9X
	feclearexcept(FE_ALL_EXCEPT);
#endif
#endif
	
	evalsp = evalstack - 1;
	evaltime = time;
	evalvp = e->val;
	for ( i = 0; i < e->tlen ; i++ ) {
		
		assert( evalstack - 1 <= evalsp && evalsp < evalstack + MAX_STACK - MAX_ARITY );
		assert( evalvp <= e->val + e->vlen );
		
		c = e->tok[i];
		tokeval(c)();
		evalsp = evalsp - tokarity(c) + 1;
		
#if defined(__USE_ISOC9X) && defined(DEBUG)
		{
			int r = fetestexcept( FE_ALL_EXCEPT );
			char *s = tokstr(e->tok[i]);
			
			if ( r & FE_INEXACT )   yDEBUG( "fp inexact exception op[%d]=%s\n", i, s );
			if ( r & FE_DIVBYZERO ) yDEBUG( "fp divide by zero exception op[%d]=%s\n", i, s );
			if ( r & FE_UNDERFLOW ) yDEBUG( "fp underflow exceptionop[%d]=%s\n", i, s );
			if ( r & FE_OVERFLOW )  yDEBUG( "fp overflow exception op[%d]=%s\n", i, s ); 
			if ( r & FE_INVALID )   yDEBUG( "fp invalid exception op[%d]=%s\n", i, s );
			feclearexcept (FE_ALL_EXCEPT); 
		}
#endif
	}	
	
	assert( evalvp == e->val + e->vlen );
	assert( evalsp == evalstack );
	
#ifdef __USE_ISOC9X
	assert( isfinite( *(evalstack) ) );
#endif
	
	return *(evalstack);
}

/* EXPR end */

#ifdef HAS_SIMPLIFY

/* SIMPLIFY */

static node *nodestack[MAX_STACK];
static int nodestackpointer;

my_inline node *npop()
{
	assert( nodestackpointer > 0 );
	return nodestack[--nodestackpointer];
}

my_inline void npush( node *v )
{
	assert( nodestackpointer < MAX_STACK );
	nodestack[nodestackpointer++] = v;
}

my_inline void ninit( void )
{
	nodestackpointer = 0;
}

node *allocn( void )
{
	node *x;
	int i;
	
	x = (node *)ymalloc( sizeof( node ) );
	for ( i = 0; i < MAX_ARITY; i++ ) x->sn[i] = NULL;

	return x;
}

void freet( node *t )
{
  int i;

  if ( t ) {
	  for ( i = 0; i < MAX_ARITY; i++ ) freet( t->sn[i] );
	  yfree( t );
  }
}

void sprintt_i( char *vbuf, node *t )
{
	char *buf;
	int o, i;
	
	assert( vbuf );
	buf = vbuf + strlen( vbuf );
	
	if ( t ) {
		
		if ( t->tok == TOK_CONST ) 
			
			sprintf( buf, "%e", t->val );			
		
		else if ( tokps(t->tok) == PS_PREFIX ) {
				
			if ( tokuv(t->tok) )
				o = sprintf( buf, "%s[%d](", tokstr(t->tok), (int)t->val );
			else
				o = sprintf( buf, "%s(", tokstr(t->tok) );			
			sprintt_i( buf + o, t->sn[0] );
			buf += strlen( buf );
			for ( i = 1; i < tokarity( t->tok ); i++ ) {
				o = sprintf( buf, "," );
				sprintt_i( buf + o, t->sn[0] );
				buf += strlen( buf );
			}
			sprintf( buf, ")" );
			
		} else {
			
			assert( tokarity(t->tok) == 2 );
			o = sprintf( buf, "(" );
			sprintt_i( buf + o, t->sn[0] );
			buf += strlen( buf );
			o = sprintf( buf, "%s", tokstr(t->tok) );
			sprintt_i( buf + o, t->sn[1] );
			buf += strlen( buf );
			sprintf( buf, ")" );	  
			
		}
	}		
}

/* aux */

static void s_gensimp( node *t )
{
	int i, a;

	yDEBUG( "tok = %s\n", tokstr( t-> tok ) );

	a = tokarity( t->tok );
	evalsp = evalstack + a - 1;
	tokeval( t->tok )();
	t->val = evalstack[0]; 
	t->tok = TOK_CONST;
	for ( i = 0; i < a; i++ ) {
		freet( t->sn[i] ); 
		t->sn[i] = NULL;
	}
}

static void s_data( node *t )
{
	t->sn[0]->val = (int)yfabs( (double)t->sn[0]->val );
}

static void s_diff( node *t )
{
	t->sn[0]->val = (int)fabs( (double)t->sn[0]->val );
}

static void s_movav( node *t )
{
	t->sn[0]->val = 1 + (int)fabs((double)t->sn[0]->val);
}

/* end aux */

node *simplify( node *t )
{
	int i, k;
	
	if ( t ) {
		
		yDEBUG( "tok = %s\n", tokstr( t->tok ) );

		k = 1;
		for ( i = 0; i < tokarity( t->tok ); i++ ) {
			simplify( t->sn[i] );
			evalstack[i]=t->sn[i]->val;
			k &= ( t->sn[i]->tok == TOK_CONST );
		}
		if ( k && ( toksfy( t->tok )!= NULL ) ) toksfy( t->tok )( t );
	}
	return t;
}

node *expr2tree( expr *e )
{
	node *n = NULL;
	int i, k;
	token c;
	
	assert_exprok( e );

	evalvp = e->val;
	ninit();
	for ( i = 0; i < e->tlen ; i++ ) {
		
 		c = e->tok[i];
		n = allocn();
		n->tok = c;

		if ( tokuv( c ) ) n->val = *(evalvp++);
		for ( k = tokarity( c ) - 1; k >= 0; k-- ) n->sn[k] = npop();
		
		npush( n );
	}
	
#ifdef DEBUG
	yDEBUG( " e = " );
	fprinte( stderr, e );
#endif

	return n;
}

char *sprintse( char *buf, expr *e )
{
	node *t;

	assert( buf );
	assert_exprok( e );
	
	t = simplify( expr2tree( e ) );
	sprintt( buf, t );
	freet( t );

	return buf;
}

void fprintse( FILE *out, expr *e )
{
	char buf[MAX_EXPR_STR];
	
	assert( out );
	assert_exprok( e );
	
	sprintse( buf, e );
	fprintf( out, "%s\n", buf );
}

#endif /* HAS_SIMPLIFY */

#if defined(HAS_CURSES) && defined(INTERACTIVE)
void displaye( expr *e )
{
	char buf[MAX_EXPR_STR];
	
	assert_exprok( e );
	
#ifdef HAS_SIMPLIFY
	sprintse( buf, e );
#else
	sprinte( buf, MAX_EXPR_STR, e );
#endif
	printw( "%s\n", buf );
}
#endif

/* SIMPLIFY end */

