// This code comes from the file squid.h in Sean Eddy's SQUID library.
// I've made a few minor changes to compile it under C++, and stiolen a
// couple of unctions from elsewhere in the SQUID package. The original copyright
// notice appears below.

/* SQUID - A C function library for biological sequence analysis
 * Copyright (C) 1992-1996 Sean R. Eddy	
 *
 *    This source code is distributed under terms of the
 *    GNU General Public License. See the files COPYING 
 *    and GNULICENSE for further details.
 *
 */

#ifndef GETOPTIONS_H
#define GETOPTIONS_H

/****************************************************
 * Support for a portable, flexible Getopt()
 ****************************************************/

/* Structure: opt_s
 * 
 * Structure for declaring options to a main().
 */
struct opt_s {
  char *name;			/* name of option, e.g. "--option1" or "-o" */
  bool  single;			/* TRUE if a single letter option           */
  int   argtype;		/* for typechecking, e.g. ARG_INT           */
};
				/* acceptable argtype's...           */
#define ARG_NONE   0		/* no argument                       */
#define ARG_INT    1		/* something that atoi() can grok    */
#define ARG_FLOAT  2		/* something that atof() can grok    */
#define ARG_CHAR   3		/* require single character or digit */
#define ARG_STRING 4		/* anything goes                     */

			/* someday, Sun Microsystems will conform to ANSI... */
#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1
#endif

int
Getopt(int argc, char **argv, struct opt_s *opt, int nopts, char *usage,
       int *ret_optind, char **ret_optname, char **ret_optarg);

#endif
