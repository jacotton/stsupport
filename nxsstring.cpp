#ifdef __BORLANDC__
	// Undefine __MINMAX_DEFINED so that min and max are correctly defined
	#ifdef __MINMAX_DEFINED
	    #undef __MINMAX_DEFINED
	#endif
#endif

#include <vector>
#include <string>

#ifdef __BORLANDC__
	// Redefine __MINMAX_DEFINED so Windows header files compile
	#ifndef __MINMAX_DEFINED
    		#define __MINMAX_DEFINED
	#endif
#endif

using namespace std;

#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "nxsstring.h"

nxsstring& nxsstring::operator+=( const double d )
{
   char tmp[81];
   sprintf( tmp, "%#3.6f", d );
   int tmplen = strlen(tmp);
   for(;;) {
      if( tmplen < 3 || tmp[tmplen-1] != '0' || tmp[tmplen-2] == '.' )
         break;
      tmp[tmplen-1] = '\0';
      tmplen--;
   }
   append(tmp);
   return *this;
}

/**
 * @method RightJustifyLong [void:public]
 * @param x [long] long value to right justify
 * @param w [int] width of field
 * @param clear_first [bool] if true, initialize string first to empty string
 *
 * Right-justifies x in a field w characters wide, using blank spaces
 * to fill in unused portions on the left-hand side of the field.
 * Specify third argument true to first empty the string.
 */
void nxsstring::RightJustifyLong( long x, int w, bool clear_first /* = false */ )
{
	if( clear_first )
		*this = "";
		
	int num_spaces = w - 1;
	if( x > 0L ) 
		num_spaces -= (int)log10( (double)x );
	if( num_spaces < 0 )
		num_spaces = 0;
		
	for( int k = 0; k < num_spaces; k++ )
		*this += ' ';
		
	*this += x;
}

/**
 * @method RightJustifyDbl [void:public]
 * @param x [long] long value to right justify
 * @param w [int] width of field
 * @param p [int] precision to use
 * @param clear_first [bool] if true, initialize string first to empty string
 *
 * Right-justifies x in a field w characters wide with precision p, using blank spaces
 * to fill in unused portions on the left-hand side of the field.
 * Specify fourth argument true to first empty the string.
 */
void nxsstring::RightJustifyDbl( double x, int w, int p, bool clear_first /* = false */ )
{
	if( clear_first )
		*this = "";

	char tmpstr[81];
	char fmtstr[81];
	sprintf( fmtstr, "%%.%df", p );
	sprintf( tmpstr, fmtstr, x );
	
	int num_spaces = w - strlen(tmpstr);
	if( num_spaces < 0 )
		num_spaces = 0;
	for( int k = 0; k < num_spaces; k++ )
		*this += ' ';
		
	*this += tmpstr;
}

/**
 * @method ShortenTo [void:public]
 * @param n [int] maximum number of characters
 *
 * Shortens string to n-3 characters, making the last
 * three characters "...".  If string is already less
 * than n character in length, has no effect.  The 
 * parameter n should be at least 4.
 */
void nxsstring::ShortenTo( int n )
{
	if( length() <= n )
		return;
		
	assert( n > 3 );
		
	char* s = new char[n+1];
	strncpy( s, this->c_str(), n-3 );
	s[n-3] = '.';
	s[n-2] = '.';
	s[n-1] = '.';
	s[n] = '\0';
	*this = s;
}
