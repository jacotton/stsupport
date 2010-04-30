// nxsdate.h
// Copyright (C) 1999 Paul O. Lewis.

#ifndef __NXSDATE_H
#define __NXSDATE_H

#if defined( __BORLANDC__ )
#	include <dos.h>
#endif

#include <time.h>
#include <iostream.h>

class NxsDate
{
	time_t secs;
	tm ts;
	public:
		NxsDate();
		NxsDate( int day, const char* month, int year
			, int hours = 0, int minutes = 0, int seconds = 1 );

		char* c_str();
		void GetCurrentDate();
		
		void operator=( const NxsDate& other_date );
		long operator-( const NxsDate& other_date ) const;
		int operator<( const NxsDate& other_date ) const;
		int operator>( const NxsDate& other_date ) const;
		friend ostream& operator<<( ostream& o, const NxsDate& d );

		class XTime {};
};

#endif

