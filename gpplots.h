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

typedef FILE *gp_port;

#include <stdlib.h>

#define GP_PLOT   0
#define GP_REPLOT 1

#define GP_DOTS   " with point ps 0.5 "
#define GP_LINES  " with lines "
#define GP_TITLE  " title "

/* prototypes */

gp_port gp_open(char *geom);
void gp_close(gp_port port);
void gp_title(gp_port port, char *msg);
void gp_save(gp_port port, char *term, char *file);
void gp_plotfile(gp_port port, int replot, char *style, char *filename);
void gp_plotarray(gp_port port, int replot, char *style, plotv *x, plotv *y, int n);

#define gp_tell fprintf

