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


/* $Id: utils.h,v 1.1 2002/03/12 08:45:22 santini Exp $ */

#ifndef H_MEMORY
#define H_MEMORY

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "utils.p"

#define ymessagef( args... )   ymessage( args )

#define ywarning( args... )    ymessage( "Warning: "  args )
#define ywarningf( args... )   ywarning( args )

#define yerror( args... )    { ymessage( "Error: "  args );\
                               exit( EXIT_FAILURE ); }
#define yerrorf( args... )	   yerror( args )

#define yperror( args... )   { ymessage( "Error: "  args );\
 	                           fprintf(stderr, ": %s\n", strerror(errno) );\
                               exit( EXIT_FAILURE ); }
#define yperrorf( args... )	   yperror( args )

#ifdef DEBUG
#define yDEBUG( args... )      ymessagef( "Debug: "  args )
#else
#define yDEBUG( args... )
#endif

#endif /* H_MEMORY */
