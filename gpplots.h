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


/* $Id: gpplots.h,v 1.1 2002/03/12 08:45:22 santini Exp $ */

typedef FILE *gp_port;

#include <stdlib.h>

#define GP_PLOT   0
#define GP_REPLOT 1

#define GP_DOTS   " with point ps 0.5 "
#define GP_LINES  " with lines "
#define GP_TITLE  " title "

#include "gpplots.p"

#define gp_tell fprintf

