#include "nexusdefs.h"
#include "xnexus.h"

bool stri_equal::operator()(const nxsstring& x, const nxsstring& y) const
{
   nxsstring::const_iterator px = x.begin();
   nxsstring::const_iterator py = y.begin();
   while( px != x.end() && py != y.end() ) {
      if( toupper(*px) != toupper(*py) ) return false;
      ++px;
      ++py;
   }

   return ( x.size() == y.size() ) ? true : false;
}

/**
 * @class      XNexus
 * @file       xnexus.h
 * @file       xnexus.cpp
 * @author     Paul O. Lewis
 * @copyright  Copyright © 1999. All Rights Reserved.
 * @variable   col [long:public] column of current line
 * @variable   line [long:public] current line in file
 * @variable   msg [nxsstring&:public] nxsstring to hold message
 * @variable   pos [long:public] current file position (for Metrowerks compiler, type is streampos rather than long)
 * @see        NexusReader
 *
 * Exception class that conveys a message specific to the problem encountered.
 */

/**
 * @constructor
 *
 * Copies s to msg.  Note: for Metrowerks compiler, type of pos is streampos rather than long.
 */
XNexus::XNexus( nxsstring s, streampos fp, long fl /* = 0L */, long fc /* = 0L */ )
	: pos(fp), line(fl), col(fc)
{
   msg = s;
}
