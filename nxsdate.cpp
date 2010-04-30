// nxsdate.cpp
// Copyright (C) 1999 Paul O. Lewis.
//
// This file defines a class useful for obtaining and displaying the current date
//
// This file is part of SimpleNxs. SimpleNxs is free software; you can 
// redistribute it and/or modify it under the terms of the GNU General 
// Public License as published by the Free Software Foundation;
// either version 2 of the License, or (at your option) any later version.
//
// SimpleNxs is distributed in the hope that it will be useful, but WITHOUT ANY 
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
// FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more 
// details.
//
// You should have received a copy of the GNU General Public License along with
// SimpleNxs; see the file COPYING.  If not, write to the Free Software Foundation,
// Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
// Author contact information:
//
// Paul O. Lewis, Ph.D.
// Assistant Professor
// Department of Ecology and Evolutionary Biology (U-43)
// University of Connecticut
// 75 North Eagleville Road
// Storrs, CT 06269-3043
//
// Office: 166A Torrey Life Sciences
// Phone: (860) 486-2069
// FAX: (860) 486-6364 (not private)
// email: paul.lewis@uconn.edu

/**
 * @class      NxsDate
 * @file       nxsdate.h
 * @file       nxsdate.cpp
 * @author     Paul O. Lewis
 * @copyright  Copyright © 1999. All Rights Reserved.
 * @variable   secs [time_t] current time
 * @variable   ts [tm] time structure
 *
 */

#include <assert.h>
#include <ctype.h>
#if (defined __MWERKS__) || (defined __GNUC__)
	#include <string.h>
#endif

#include "nxsdate.h"

/**
 * @constructor
 *
 * Calls GetCurrentDate method.
 */
NxsDate::NxsDate()
{
	GetCurrentDate();
}

/**
 * @constructor
 * @param day [int] the day
 * @param month [const char*] the month (no abbreviation allowed)
 * @param year [int] the year (supply all four digits)
 * @param hours [int] default is 0
 * @param minutes [int] default is 0
 * @param seconds [int] default is 1
 * @throws XTime
 *
 * Sets secs and fills ts based on day, month, year, hours, minutes,
 * and seconds provided.  Throws XTime exception if parameters
 * supplied do not represent a valid date/time.
 */
NxsDate::NxsDate( int day, const char* month, int year
	, int hours /* = 0 */, int minutes /* = 0 */
	, int seconds /* = 1 */ )
{
	assert( month != NULL );
	char mstr[10];
	int n = strlen(month);
	for( int i = 0; i < n; i++ )
		mstr[i] = toupper( month[i] );
	mstr[n] = '\0';
	
	ts.tm_mday = day;
	if( strcmp( month, "JANUARY" ) == 0 )
		ts.tm_mon = 0;
	else if( strcmp( month, "FEBRUARY" ) == 0 )
		ts.tm_mon = 1;
	else if( strcmp( month, "MARCH" ) == 0 )
		ts.tm_mon = 2;
	else if( strcmp( month, "APRIL" ) == 0 )
		ts.tm_mon = 3;
	else if( strcmp( month, "MAY" ) == 0 )
		ts.tm_mon = 4;
	else if( strcmp( month, "JUNE" ) == 0 )
		ts.tm_mon = 5;
	else if( strcmp( month, "JULY" ) == 0 )
		ts.tm_mon = 6;
	else if( strcmp( month, "AUGUST" ) == 0 )
		ts.tm_mon = 7;
	else if( strcmp( month, "SEPTEMBER" ) == 0 )
		ts.tm_mon = 8;
	else if( strcmp( month, "OCTOBER" ) == 0 )
		ts.tm_mon = 9;
	else if( strcmp( month, "NOVEMBER" ) == 0 )
		ts.tm_mon = 10;
	else if( strcmp( month, "DECEMBER" ) == 0 )
		ts.tm_mon = 11;
	else
		ts.tm_mon = -1;
	ts.tm_year = year - 1900;

	ts.tm_hour = hours;
	ts.tm_min = minutes;
	ts.tm_sec = seconds;

	ts.tm_wday = -1;
	ts.tm_yday = -1;
	ts.tm_isdst = -1;

	// ensure that the date is correct (and compute tm_wday and tm_yday)
	secs = mktime(&ts);
	if( secs == -1 ) throw XTime();
}

/**
 * @method c_str [char*:public]
 *
 * Calls asctime to convert currently stored date and time to
 * a string.
 */
char* NxsDate::c_str()
{
	return asctime(&ts);
}

/**
 * @method GetCurrentDate [void:public]
 *
 * Sets secs and fills ts. Called by the NxsDate default constructor.
 */
void NxsDate::GetCurrentDate()
{
	secs = time(NULL);
	tm* time_structure;
	time_structure = localtime(&secs);
#if 1
	ts = *time_structure;
#else
	ts.tm_mday = time_structure->tm_mday;	// day of the month (1 to 31)
	ts.tm_mon = time_structure->tm_mon;	// month (0 to 11)
	ts.tm_year = time_structure->tm_year;	// year

	ts.tm_hour = time_structure->tm_hour;	// hours (24 hour clock)
	ts.tm_min = time_structure->tm_min;	// minutes
	ts.tm_sec = time_structure->tm_sec;	// seconds

	ts.tm_wday = time_structure->tm_wday;	// day of the week (0 is Sunday)
	ts.tm_yday = time_structure->tm_yday;	// day of the year (0 to 365)
	ts.tm_isdst = time_structure->tm_isdst;	// is daylight savings time
#endif
}
		
/**
 * @operator = [void:public]
 * @param other_date [const NxsDate&] the object being copied
 *
 * Copies secs and ts from other_date object.
 */
void NxsDate::operator=( const NxsDate& other_date )
{
	secs = other_date.secs; 
	ts = other_date.ts;
}

/**
 * @operator - [long:public]
 * @param other_date [const NxsDate&] the object being copied
 *
 * Returns secs minus other_date.secs.
 */
long NxsDate::operator-( const NxsDate& other_date ) const
{
	return ( secs - other_date.secs );
}

/**
 * @operator < [int:public]
 * @param other_date [const NxsDate&] the other NxsDate object
 *
 * Returns 1 if secs is less than other_date.secs; otherwise,
 * returns 0.
 */
int NxsDate::operator<( const NxsDate& other_date ) const
{
	return ( secs < other_date.secs ? 1 : 0 );
}

/**
 * @operator > [int:public]
 * @param other_date [const NxsDate&] the other NxsDate object
 *
 * Returns 1 if secs is greater than other_date.secs; otherwise,
 * returns 0.
 */
int NxsDate::operator>( const NxsDate& other_date ) const
{
	return ( secs > other_date.secs ? 1 : 0 );
}

/**
 * @operator << [ostream&:public]
 * @param o [ostream&] the output stream
 * @param d [const NxsDate&] the NxsDate object
 *
 * Outputs the current date and time in the form of a string.
 * This function is a friend, not a member function.
 */
ostream& operator<<( ostream& o, const NxsDate& d )
{
	o << asctime(&d.ts);
	return o;
}

