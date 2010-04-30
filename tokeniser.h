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
 
 // $Id: tokeniser.h,v 1.12 2002/03/05 17:23:25 rdmp1c Exp $
 
#ifndef TOKENISER_H
#define TOKENISER_H

#include <iostream>
#include <fstream>


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


class XTokeniser
{
public:
	std::string msg;
#if (defined( __MWERKS__ ) || (defined __BORLANDC__  && (__BORLANDC__ < 0x0550)))
	long       pos;
#else
	std::streampos  pos;
#endif
	long line;
	long col;

#if (defined( __MWERKS__ ) || (defined __BORLANDC__  && (__BORLANDC__ < 0x0550)))
	XTokeniser( std::string s, long fp = 0, long fl = 0L, long fc = 0L )
#else
	XTokeniser( std::string s, std::streampos fp = 0, long fl = 0L, long fc = 0L )
#endif
	{
		msg = s;
		pos = fp;
		line = fl;
		col = fc;
	};
};


class Tokeniser
{
#if defined __BORLANDC__ && (__BORLANDC__ < 0x0550)
	istream&		in;
#else
	std::istream&		in;
#endif
    char 			curChar;
	std::string		token;
	std::string		comment;
#if (defined( __MWERKS__ ) || (defined __BORLANDC__  && (__BORLANDC__ < 0x0550)))
	long       filepos;
#else
	std::streampos  filepos;
#endif

	long       fileline;
	long       filecol;
	bool       atEOF;
	bool       atEOL;
	
#ifdef __MWERKS__
	char	putBuffer;
#endif


public:
	enum tokentype
	{
		EMPTY,
		STRING,			// a text token
		NUMBER,			// a real or integer number
		OTHER,
		BAD,
		SPACE,			//
		LPAR,			// (
		RPAR,			// )
		COMMA,			// ,
		SEMICOLON,		// ;
		EQUALS,			// =
		MINUS,			// -
		ASTERIX,			// *
		BACKSLASH,		// /
		LCURLY,			// {
		RCURLY,			// }
		DOUBLEQUOTE,		// "
		BANG,			// !
		HASH,			// #
		COLON			// :
	};

	Tokeniser ();
#if defined __BORLANDC__ && (__BORLANDC__ < 0x0550)
	Tokeniser(istream& i);
#else
	Tokeniser(std::istream& i);
#endif    
	virtual ~Tokeniser() {};
	bool       	AtEOF() { return atEOF; };
	bool      	AtEOL() { return atEOL; };
	long       	GetFileColumn() { return filecol; };
#if (defined( __MWERKS__ ) || (defined __BORLANDC__  && (__BORLANDC__ < 0x0550)))
	long     	GetFilePosition() { return filepos; };
#else
	std::streampos  GetFilePosition() { return filepos; };
#endif
	long       	GetFileLine() { return fileline; };
	char 	  	GetNextChar ();
	tokentype 	GetNextToken ();
	std::string GetToken () { return token; };
	virtual bool IsPunctuation (char ch);
	virtual bool IsWhiteSpace (char ch);
	bool 		ParseComment ();
	tokentype		ParseNumber ();
	bool 		ParseString ();
	bool		ParseToken ();
	// don't use these functions as they will bugger up keeping track of
	// file positions.
	//	char PeekNextChar ();
	//    void PutBack (char ch) { in.putback (ch); };
#if (__BORLANDC__ < 0x0550)
	bool TokenEquals (std::string s) { return s.compare (token); };
#else
	bool TokenEquals (std::string s) { return (token == s); };
#endif
};

#if __BORLANDC__ 
	// Redefine __MINMAX_DEFINED so Windows header files compile
	#ifndef __MINMAX_DEFINED
    		#define __MINMAX_DEFINED
	#endif
#endif

#endif

