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
 
 // $Id: Parse.h,v 1.6 2001/08/03 10:45:23 rdmp1c Exp $
 
//
// Simple string parser
//


#ifndef PARSE_H
#define PARSE_H

#ifdef __BORLANDC__
	// Undefine __MINMAX_DEFINED so that min and max are correctly defined
	#ifdef __MINMAX_DEFINED
		#undef __MINMAX_DEFINED
	#endif
    // Ignore "Cannot create precompiled header: code in header" message
    // generated when compiling string.cc
    #pragma warn -pch
#endif

#include <string>

#ifdef __BORLANDC__
    #pragma warn .pch
#endif


using namespace std;

enum tokentype {STRING, NUMBER, OTHER, ENDOFSTRING, BAD, SPACE, LPAR, RPAR,
	COMMA, SEMICOLON, COLON, TAB, NEWLINE};

class Parser
{
public:
	Parser () { text = ""; token = ""; pos = 0; };
	Parser (string s) { text = s; token = ""; pos = 0; };
	Parser (char *s) { text = s; token = ""; pos = 0; };	
	virtual tokentype NextToken ();
	virtual string GetToken () { return token; };
	virtual const char *GetTokenAsCstr () { return token.c_str(); };
	virtual int GetPos () { return pos; };

	virtual bool IsPunctuation (char ch);
	virtual bool IsWhiteSpace (char ch);
	virtual char GetNextChar () { return text[++pos]; };
    virtual void PutBack (char ch) { pos--; };
	virtual tokentype ParseNumber ();    
protected:
	string	text;
	string	token;
	int 	pos;
};


#ifdef __BORLANDC__
	// Redefine __MINMAX_DEFINED so Windows header files compile
	#ifndef __MINMAX_DEFINED
			#define __MINMAX_DEFINED
	#endif
#endif


#endif  // PARSE_H
