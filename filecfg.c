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
#include <search.h>

#define PARAM_MAX   100
#define PARAM_LEN    40
#define PARAM_FMT   "%40s = %40s\n"

#include "utils.h"

struct params {
	char name[PARAM_LEN], value[PARAM_LEN];
};

static struct params param[PARAM_MAX];
static size_t paramcount;

static int paramcompar(const void *s1, const void *s2)
{
	return strncmp( (char *)(((struct params *)s1)->name), (char *)(((struct params *)s2)->name), PARAM_LEN);
}

char *findparam( char *name )
{
	struct params *found, t;

	found = (struct params *)lfind( name, param, &paramcount, sizeof(struct params), paramcompar );
/*	found = (struct params *)qsearch( name, param, paramcount, sizeof(struct params), paramcompar ); */


	if ( found ) {
		memcpy( &t, found, sizeof(t) );
		memcpy( found, &(param[--paramcount]), sizeof(t) );
		memcpy( &(param[paramcount]), &t, sizeof(t) );
		return param[paramcount].value;
	} else return NULL;
}

void readparams( char *file )
{
	FILE *in;

	if ((in = fopen(file, "r")) == NULL)
		yperror( "can't open config file '%s'", file );

	paramcount = 0;
	while ( !feof(in) ) {
		if ( fscanf( in, PARAM_FMT, param[paramcount].name, param[paramcount].value ) != 2 )
			yerror("file format error in config file '%s'\n", file );
		paramcount++;
	}
/*	qsort( param, paramcount, sizeof(struct params), paramcompar ); */
	fclose(in);
}
