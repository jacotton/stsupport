// This code comes from the file getopt.c in Sean Eddy's SQUID library.
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <iostream.h>

#include "getoptions.h"

/* Function: IsInt()
 * 
 * Returns TRUE if s points to something that atoi() will parse
 * completely and convert to an integer.
 */
int
IsInt(char *s)
{
  int hex = 0;

  if (s == NULL) { return 0; }

				/* skip whitespace */
  while (isspace(*s)) s++;      
				/* skip leading sign */
  if (*s == '-' || *s == '+') s++;
				/* skip leading conversion signals */
  if ((strncmp(s, "0x", 2) == 0 && (int) strlen(s) > 2) ||
      (strncmp(s, "0X", 2) == 0 && (int) strlen(s) > 2))
    {
      s += 2;
      hex = 1;
    }
  else if (*s == '0' && (int) strlen(s) > 1)
    s++;
				/* examine remainder for garbage chars */
  if (!hex)
    while (*s != '\0')
      {
	if (!isdigit(*s)) return 0;
	s++;
      }
  else
    while (*s != '\0')
      {
	if (!isxdigit(*s)) return 0;
	s++;
      }

  return 1;
}


/* Function: IsReal()
 * 
 * Purpose:  Returns TRUE if s is a string representation
 *           of a valid floating point number.
 */
int
IsReal(char *s)
{
  int gotdecimal = 0;
  int gotexp     = 0;
  int gotreal    = 0;

  if (s == NULL) return 0;

  while (isspace(*s)) s++;         /* skip leading whitespace */
  if (*s == '-' || *s == '+') s++; /* skip leading sign */

  /* Examine remainder for garbage. Allowed one '.' and
   * one 'e' or 'E'; if both '.' and e/E occur, '.'
   * must be first.
   */
  while (*s != '\0')
    {
      if (isdigit(*s)) 
	gotreal++;
      else if (*s == '.')
	{
	  if (gotdecimal) return 0; /* can't have two */
	  if (gotexp) return 0;	/* e/E preceded . */
	  else gotdecimal++;
	}
      else if (*s == 'e' || *s == 'E')
	{
	  if (gotexp) return 0;	/* can't have two */
	  else gotexp++;
	}
      else if (isspace(*s))
	break;

      s++;
    }

  while (isspace(*s)) s++;         /* skip trailing whitespace */
  if (*s == '\0' && gotreal) return 1;
  else return 0;
}

/* Function: Getopt()
 * 
 * Purpose:  Portable command line option parsing with abbreviated
 *           option switches. Replaces UNIX getopt(). Using UNIX getopt()
 *           hinders portability to non-UNIX platforms, and getopt()
 *           is also limited to single letter options.
 *
 *           Getopt() implements a superset of UNIX getopt().
 *           All of getopt()'s single-character switch behavior
 *           is emulated, and "--" by itself terminates the options.
 *           Additionally, Getopt() provides extended switches
 *           like "--youroptionhere", and Getopt() type checks
 *           arguments.  
 * 
 *           Extended options must start with "--", as in "--option1".
 *           Normal options must start with "-", as in "-o".
 *           Normal options may be concatenated, as in "-a -b" == "-ab".
 *           
 *           See bottom of this .c file after #fdef GETOPT_TESTDRIVER
 *           for an example of calling Getopt().
 *           
 * Args:     argc  - from main(). number of elems in argv.
 *           argv  - from main(). argv[0] is the name of the command.
 *           opt   - array of opt_s structures, defining option switches
 *           nopts - number of switches in opt
 *           usage - a (possibly long) string to print if usage error.
 *           ret_optind - RETURN: the index in argv[] of the next 
 *                        valid command-line token.
 *           ret_optname- RETURN: ptr to the name of option switch 
 *                        seen, or NULL if no option was seen.
 *           ret_optarg - RETURN: ptr to the optional argument, if any;
 *                        NULL if option takes no argument.
 *                        
 * Return:   1 if a valid option was parsed.
 *           0 if no option was found, and command-line parsing is complete.
 *           Die()'s here if an error is detected.
 */
int
Getopt(int argc, char **argv, struct opt_s *opt, int nopts, char *usage,
       int *ret_optind, char **ret_optname, char **ret_optarg)
{
  int i;
  int arglen;
  int nmatch;
  static int optind   = 1;        /* init to 1 on first call  */
  static char *optptr = NULL;     /* ptr to next valid switch */
  int opti;

  /* Check to see if we've run out of options.
   */
  if (optind >= argc || argv[optind][0] != '-')
    { 
      *ret_optind  = optind; 
      *ret_optarg  = NULL; 
      *ret_optname = NULL; 
      return 0; 
    }

  /* Check to see if we're being told that this is the end
   * of the options.
   */
  if (strcmp(argv[optind], "--") == 0)
    { 
      optind++;
      *ret_optind  = optind; 
      *ret_optname = NULL;
      *ret_optarg  = NULL; 
      return 0; 
    }

  /* We have a real option. Find which one it is.
   * We handle single letter switches "-o" separately
   * from full switches "--option", based on the "-" vs. "--"
   * prefix -- single letter switches can be concatenated.
   */
				/* full option */
  if (optptr == NULL && strncmp(argv[optind], "--", 2) == 0)
    {
      optptr = NULL;		/* full options can't concantenate */
      arglen = strlen(argv[optind]);
      nmatch = 0;
      for (i = 0; i < nopts; i++)
	if (opt[i].single == false &&
	    strncmp(opt[i].name, argv[optind], arglen) == 0)
	  { 
	    nmatch++;
	    opti = i;
	  }
      if (nmatch > 1)
      {
      	cerr << "Option \"" << argv[optind] << "\" is ambiguous; please be more specific."
        	<< endl << usage << endl;
        exit (0);
		}
      if (nmatch == 0)
      {
      	cerr << "No such option \"" << argv[optind] << "\"."
        	<< endl << usage << endl;
        exit(0);
  	}

      *ret_optname = opt[opti].name;

      /* Set the argument, if there is one
       */
      if (opt[opti].argtype != ARG_NONE) 
	{
	  if (optind+1 >= argc)
      {
      	cerr << "Option " << opt[opti].name << "\" requires an argument."
        	<< endl << usage << endl;
        exit (0);
		}
	  *ret_optarg = argv[optind+1];
	  optind+=2;
	}
      else  /* ARG_NONE */
	{
	  *ret_optarg = NULL;
	  optind++;
	}
    }
  else				/* else, a single letter option "-o" */
    {
				/* find the option */
      if (optptr == NULL) 
	optptr = argv[optind]+1;
      for (opti = -1, i = 0; i < nopts; i++)
	if (opt[i].single == true && *optptr == opt[i].name[1])
	  { opti = i; break; }
      if (opti == -1)
      {
      	cerr << "No such option \"" << *optptr << "\"."
        	<< endl << usage << endl;
        exit(0);
  	}
      *ret_optname = opt[opti].name;

				/* set the argument, if there is one */
      if (opt[opti].argtype != ARG_NONE) 
	{
	  if (*(optptr+1) != '\0')   /* attached argument */
	    {
	      *ret_optarg = optptr+1;
	      optind++;
	    }
	  else if (optind+1 < argc) /* unattached argument */
	    {
	      *ret_optarg = argv[optind+1];
	      optind+=2;	      
	    }
	  else
      {
      	cerr << "Option " << opt[opti].name << "requires an argument."
        	<< endl << usage << endl;
        exit(0);
  	}

	  optptr = NULL;	/* can't concatenate after an argument */
	}
      else  /* ARG_NONE */
	{
	  *ret_optarg = NULL;
	  if (*(optptr+1) != '\0')   /* concatenation */
	    optptr++; 
	  else
	    {
	      optind++;                /* move to next field */
	      optptr = NULL;
	    }
	}

    }

  /* Type check the argument, if there is one
   */
  if (opt[opti].argtype != ARG_NONE) 
    {
      if (opt[opti].argtype == ARG_INT && ! IsInt(*ret_optarg))
      {
      	cerr << "Option " << opt[opti].name << "requires an integer argument."
        	<< endl << usage << endl;
        exit(0);
  	}
      else if (opt[opti].argtype == ARG_FLOAT && ! IsReal(*ret_optarg))
      {
      	cerr << "Option " << opt[opti].name << "requires a numerical argument."
        	<< endl << usage << endl;
        exit(0);
  	}
      else if (opt[opti].argtype == ARG_CHAR && strlen(*ret_optarg) != 1)
      {
      	cerr << "Option " << opt[opti].name << "requires single-character argument."
        	<< endl << usage << endl;
        exit(0);
  	}
      /* ARG_STRING is always ok, no type check necessary */
    }

  *ret_optind = optind;
  return 1;
}



#ifdef GETOPT_TESTDRIVER 

struct opt_s OPTIONS[] = {
  { "--test1", FALSE, ARG_INT    },
  { "--test2", FALSE, ARG_FLOAT  },
  { "--test3", FALSE, ARG_STRING },
  { "--test4", FALSE, ARG_CHAR   },
  { "-a",      TRUE,  ARG_NONE   },
  { "-b",      TRUE,  ARG_INT    },
};
#define NOPTIONS (sizeof(OPTIONS) / sizeof(struct opt_s))
    
int
main(int argc, char **argv)
{
  int   optind;
  char *optarg;
  char *optname;

  while (Getopt(argc, argv, OPTIONS, NOPTIONS, "Usage/help here",
		&optind, &optname, &optarg))
    {
      printf("index: %d name: %s argument: %s\n",
	     optind, optname, optarg);
    }
}

#endif /*GETOPT_TESTDRIVER*/
