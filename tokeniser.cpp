/*
 * TreeLib
 * A library for manipulating phylogenetic trees.
 * Copyright (C) 2001 Roderic D. M. Page <r.page@bio.gla.ac.uk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307, USA.
 */
 
// $Id: tokeniser.cpp,v 1.9 2002/03/05 17:23:24 rdmp1c Exp $

#include "tokeniser.h"
#include <ctype.h>
#include <string.h>


//------------------------------------------------------------------------------
#if defined __BORLANDC__ && (__BORLANDC__ < 0x0550)
Tokeniser::Tokeniser (istream& i ) : in(i)
#else
Tokeniser::Tokeniser (std::istream& i ) : in(i)
#endif
{
	curChar 	= '\0';
	filecol    	= 1L;
	fileline    	= 1L;
	filepos     	= 0L;
	atEOF		= false;
	atEOL		= false;
	token		= "";
	
#ifdef __MWERKS__
	putBuffer = '\0';
#endif

}

//------------------------------------------------------------------------------
bool Tokeniser::IsPunctuation (char ch)
{
	char punctuation[22];
	punctuation[0]  = '(';
	punctuation[1]  = ')';
	punctuation[2]  = '[';
	punctuation[3]  = ']';
	punctuation[4]  = '{';
	punctuation[5]  = '}';
	punctuation[6]  = '/';
	punctuation[7]  = '\\';
	punctuation[8]  = ',';
	punctuation[9]  = ';';
	punctuation[10] = ':';
	punctuation[11] = '=';
	punctuation[12] = '*';
	punctuation[13] = '\'';
	punctuation[14] = '"';
	punctuation[15] = '`';
	punctuation[16] = '+';
	punctuation[17] = '-';
	punctuation[18] = '<';
	punctuation[19] = '>';
	punctuation[20] = '!';
	punctuation[21] = '#';
	punctuation[22] = '\0';


	return (bool)(strchr (punctuation, ch) != NULL);
}


//------------------------------------------------------------------------------
bool Tokeniser::IsWhiteSpace (char ch)
{
	char whitespace[4];
	whitespace[0]  = ' ';
	whitespace[1]  = '\t';
	whitespace[2]  = '\n';
	whitespace[3]  = '\0';

	return (bool)(strchr (whitespace, ch) != NULL);
}

//------------------------------------------------------------------------------
char Tokeniser::GetNextChar ()
{
	int ch;
#ifdef __MWERKS__
	if (putBuffer != '\0')
	{
		ch = putBuffer;
		putBuffer = '\0';
	}
	else
		ch = in.get();	
#else
	ch = in.get();
#endif
	int failed = in.bad();
	if( failed )
		throw XTokeniser ( "Unknown error reading data file (check to make sure file exists)" );

	//	cout << "[" << (char)ch << "]" << endl;

	if( ch == 13 || ch == 10 )
	{
		fileline++;
		filecol = 1L;

		if( ch == 13 && (int)in.peek() == 10 )
			ch = in.get();

		atEOL = true;
	}
	else if( ch == EOF )
	{
		atEOF = true;
	}
	else
	{
		filecol++;
		atEOL = false;
	}

	filepos = in.tellg();



	if (atEOF )
		return '\0';
	else if (atEOL )
		return '\n';
	else
   	return (char)ch;
}



//------------------------------------------------------------------------------
Tokeniser::tokentype Tokeniser::GetNextToken ()
{
	tokentype TokenType = EMPTY;

	while ((TokenType == EMPTY) && !in.bad() && !atEOF)
	{
		curChar = GetNextChar ();

		if (IsWhiteSpace (curChar))
		{
		// skip white space
		}
		else
		{
			if (IsPunctuation (curChar))
			{
 				// classify punctuation token
				switch (curChar)
				{
					case '[': ParseComment (); break;
					case '\'':
						if (ParseString ())
							TokenType = STRING;
						else TokenType = BAD;
						break;
					case '(':
						TokenType = LPAR;
						break;
					case ')':
						TokenType = RPAR;
						break;
					case '{':
						TokenType = LPAR;
						break;
					case '}':
						TokenType = RPAR;
						break;
					case '!':
						TokenType = BANG;
						break;
					case '#':
						TokenType = HASH;
						break;
					case '=':
						TokenType = EQUALS;
						break;
					case ';':
						TokenType = SEMICOLON;
						break;
					case ',':
						TokenType = COMMA;
						break;
					case '*':
						TokenType = ASTERIX;
						break;
					case ':':
						TokenType = COLON;
						break;
					case '-':
						TokenType = MINUS;
						break;
					case '"':
						TokenType = DOUBLEQUOTE;
						break;
					case '/':
						TokenType = BACKSLASH;
						break;
					default:
						TokenType = OTHER;
						break;
				}
			}
			else
			{
            			// It's either a number, or a string
				if (isdigit (curChar))
				{
					TokenType = ParseNumber();
/*					if (ParseNumber ())
                    				TokenType = NUMBER;
                   			else
                    				TokenType = BAD;
*/
				}
				else
                			{
					if (ParseToken ())
						TokenType = STRING;
					else TokenType = BAD;
				}
			}
		}
	}

	if ((TokenType != STRING) && (TokenType != NUMBER))
	{
		token = "";
		token += curChar;
	}
	return TokenType;
}

//------------------------------------------------------------------------------
bool Tokeniser::ParseString ()
{
	bool done = false;
	 char lastChar = '\0';
	token = "";

  	while (!done && !atEOF)
	{
		curChar = GetNextChar ();

		if (curChar=='\'')
		{
			if (lastChar == '\0')		// first time we've encountered a quote
				lastChar = '\'';
			else if (lastChar == '\'')	// second single quote
			{
				token += curChar;
				lastChar = '\0';
			}
	        }
	        else
	        {
		        	if (lastChar == '\'')
		          {
			// end of quoted string indicated by single quote that doesn't
		                // follow another single quote
		                done = true;
			}
			else
			{
				lastChar = '\0';
				if (curChar == '_')
					token += ' ';
				else
					token += curChar;
			}
		}
	}
#ifdef __MWERKS__
	putBuffer = curChar;
#else
	in.putback (curChar);
#endif
 	filecol--;
	return (done);
}


//------------------------------------------------------------------------------
// Parse a number (integer or real).
Tokeniser::tokentype Tokeniser::ParseNumber ()
{
	enum {
		start		= 0x0001, // 0
		sign		= 0x0002, // 1
		digit		= 0x0004, // 2
		fraction	= 0x0008, // 3
		expsymbol	= 0x0010, // 4
		expsign	= 0x0020, // 5
		exponent 	= 0x0040, // 6
		bad		= 0x0080,
		done		= 0x0100
    } state;

    tokentype result = BAD;

	token = "";
	state = start;

	while (!IsWhiteSpace (curChar)
		&& !(IsPunctuation (curChar) && (curChar != '-'))
		&& (state != bad)
		&& (state != done))
	{
		if (isdigit (curChar))
		{
			switch (state)
			{
				case start:
				case sign:
					state = digit;
					break;
				case expsymbol:
				case expsign:
					state = exponent;
					break;
				default:
					break;
			}
		}
		else if ((curChar == '-') || (curChar == '+'))
		{
			switch (state)
			{
				case start:
					state = sign;		// sign of number
					break;
				case digit:
					state = done;		// minus sign is punctuation, such as 6-10
					break;
				case expsymbol:
					state = expsign;		// sign of exponent
					break;
				default:
					state = bad;		// syntax error
					break;
			}
		}
		else if ((curChar == '.') && (state == digit))
        			state = fraction;
		else if (((curChar == 'E') || (curChar == 'e')) && (state & (digit | fraction)))
			state = expsymbol;
		else
			state = bad;

		if ((state != bad) && (state != done))
		{
			token += curChar;
			curChar = GetNextChar ();
		}
	}

	int isNumber =  state & (digit | fraction | exponent | done);
	if (isNumber)
	{
		// We have a number
		result = NUMBER;

		if (IsPunctuation (curChar))
		{
#ifdef __MWERKS__
			putBuffer = curChar;
#else
			in.putback (curChar);
#endif
			if (!atEOL)
				filecol--;
		}
	}
	else
    {
		// Not a number, but a string that starts with numbers, such as "00BW0762.1"
			do {
				if (curChar == '_')
					token += ' ';
				else
					token += curChar;
            	curChar = GetNextChar ();
 			} while (isalnum (curChar) || (curChar == '_') || (curChar == '.'));
			if (IsPunctuation (curChar))
			{
#ifdef __MWERKS__
				putBuffer = curChar;
#else
				in.putback (curChar);
#endif
				if (!atEOL)
					filecol--;
			}

			result = STRING; //classify the token

    }


	return  result;
}

//------------------------------------------------------------------------------
bool Tokeniser::ParseToken ()
{
	token = "";
	while ((curChar != '\0') && !IsWhiteSpace(curChar) && !IsPunctuation(curChar))
	{
		if (curChar == '_')
			token += ' ';
		else
			token += curChar;
		curChar = GetNextChar ();
	}
	if (!atEOL)
	{
#ifdef __MWERKS__
		putBuffer = curChar;
#else
		in.putback (curChar);
#endif
		filecol--;
	}
	return true;
}

//------------------------------------------------------------------------------
// Parse a NEXUS-style comment
bool Tokeniser::ParseComment ()
{
	bool echo = false;
	comment = "";

	curChar = GetNextChar ();
	echo = (curChar == '!');
	if (echo)
		curChar =  GetNextChar ();

	while ((curChar != '\0') && (curChar != ']'))
	{
		comment += curChar;
		curChar = GetNextChar();
	}

	if (echo)
#if defined __BORLANDC__ && (__BORLANDC__ < 0x0550)
		cout << comment;
#else
		std::cout << comment;
#endif

	return true;
}

/*
//------------------------------------------------------------------------------
char Tokeniser::PeekNextChar ()
{
	int ch;
	ch = in.peek();
	int failed = in.bad();
 	if( failed )
		throw XTokeniser ( "Unknown error reading data file (check to make sure file exists)" );

	if( ch == 13 || ch == 10 )
	{
		atEOL = true;
	}
	else if( ch == EOF )
    {
		atEOF = true;
    }
	else
    {
		atEOL = false;
	}
   if (atEOF )
      return '\0';
   else if (atEOL )
      return '\n';
   else
   	return (char)ch;
}
*/





