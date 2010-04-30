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

 // $Id: Parse.cpp,v 1.9 2002/02/23 12:22:32 rdmp1c Exp $
 
#include <ctype.h>
#include "Parse.h"

// Return the next token in the string
tokentype Parser::NextToken ()
{
	tokentype t;
	token = "";

	if (pos >= text.length())
	{
		t = ENDOFSTRING;
	}
	else
	{
		char ch = text[pos];

		if (ch == '\'')
		{
			// Single quoted token
			bool done = false;
			pos++;
			while (!done)
			{
				ch = text[pos++];
				// Look ahead for double quote
				if (ch == '\'')
				{
					ch = text[pos];
					done = (ch != '\'');
				}
				if (!done && (ch != '\n') && (ch != '\r'))
				{
					token += ch;
				}
			}
			t = STRING; //classify the token
		}
		else if (isalpha (ch))
		{
			while (isalnum (ch) || (ch == '_') || (ch == '.') && (pos < text.length()))
			{
				if (ch == '_')
					token += ' ';
				else
					token += ch;
				pos++;
				ch = text[pos];
			}
			t = STRING; //classify the token
		}
		else
		{
			if (isdigit(ch) || (ch == '-'))
			{
            	t = ParseNumber();
//				int stop = text.find_first_not_of ("01234567890-.Ee", pos);
//				token = text.substr (pos, stop-pos);
// 				pos = stop;
//				t = NUMBER;
			}
			else
			{
				token += ch;
				pos++;
				switch (ch)
				{
					case '(': t = LPAR; 	 	break;
					case ')': t = RPAR; 	 	break;
					case ' ': t = SPACE; 	 	break;
					case ',': t = COMMA; 	 	break;
					case ';': t = SEMICOLON; 	break;
					case ':': t = COLON;		break;
					case '\t': t = TAB;		break;
					case '\r':
					case '\n': t = NEWLINE;	break;
					default:  t = BAD; 		break;
				}
			}
		}
	}

//    cout << "token = " << token << endl;

	return (t);
}

//------------------------------------------------------------------------------
// Parse a number (integer or real).
tokentype Parser::ParseNumber ()
{
	enum {
		start		= 0x0001, // 0
		sign		= 0x0002, // 1
        digit		= 0x0004, // 2
		fraction	= 0x0008, // 3
		expsymbol	= 0x0010, // 4
		expsign		= 0x0020, // 5
        exponent 	= 0x0040, // 6
        bad			= 0x0080,
        done		= 0x0100
    } state;

    tokentype result = BAD;

	token = "";
    state = start;

    char curChar = text[pos]; 

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
                	state = expsign;	// sign of exponent
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
//        	PutBack (curChar);
//            if (!atEOL)
//            	filecol--;
        }
	}        

	else
    {
		// Not a number, but a string that starts with numbers, such as "00BW0762.1"
//        cout << "Do something!" << endl;
			do {
				if (curChar == '_')
					token += ' ';
				else
					token += curChar;
            	curChar = GetNextChar ();
 			} while (isalnum (curChar) || (curChar == '_') || (curChar == '.') && (pos < text.length()));
			result = STRING; //classify the token

    }

	return result;
}


//------------------------------------------------------------------------------
bool Parser::IsPunctuation (char ch)
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
bool Parser::IsWhiteSpace (char ch)
{
	char whitespace[4];
	whitespace[0]  = ' ';
	whitespace[1]  = '\t';
	whitespace[2]  = '\n';
	whitespace[3]  = '\0';

    return (bool)(strchr (whitespace, ch) != NULL);
}
