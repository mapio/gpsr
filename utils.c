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

#include "config.h"

#include <stdarg.h>
#include <errno.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>

#include "utils.h"

int ymallocuse( void )
{
#ifdef HAS_MALLINFO
	struct mallinfo i;

	i=mallinfo();

	return i.uordblks;
#else
	return 0;
#endif
}

void *ymalloc(size_t size)
{
	void *x;

	assert( size >= 0 );
	x = malloc(size);
	assert( x );

	yDEBUG( "used=%d\n", ymallocuse() );

	return x;
}

void yfree( void *ptr )
{
	assert( ptr );

	free( ptr );

	yDEBUG( "used=%d\n", ymallocuse() );
}

void *yrealloc(void *ptr, size_t size)
{
	void *x;

	assert( ptr );
	assert( size >= 0 );

	x = realloc( ptr, size );
	assert( x );

	yDEBUG( "used=%d\n", ymallocuse() );

	return x;
}

ssize_t yread(int fd, void *buf, size_t count)
{
	int r;

	r = read( fd, buf, count );

	if ( r < 0 ) yperrorf( "" );
	if ( r < count ) ywarningf( "count=%d, read=%d\n", count, r );

	return r;
}

ssize_t ywrite(int fd, const void *buf, size_t count)
{
	int r;

	r = write( fd, buf, count );

	if ( r < 0 ) yperrorf( "" );
	if ( r < count ) ywarningf( "count=%d, read=%d\n", count, r );

	return r;
}

char *ystrncpy(char *dest, const char *src, size_t n)
{
	assert( dest );
	assert( src );

	strncpy( dest, src, n );
	dest[n] = '\0';

	return dest;
}

void ymessage(const char *template, ...)
{
	va_list ap;
#ifdef HAS_PROGSHORTNAME
	extern char *program_invocation_short_name;

	fprintf(stderr, "%s: ", program_invocation_short_name);
#endif
	va_start(ap, template);
	vfprintf(stderr, template, ap);
	va_end(ap);
}

int yexists( char *file )
{
	int r;
	struct stat buf;

	r = stat( file, &buf );
	if ( r == 0 )
		return 1;
	else if ( errno == ENOENT )
		return 0;
	else yperrorf("stat'ing %s", file );
}
