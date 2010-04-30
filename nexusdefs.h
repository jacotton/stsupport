#ifndef __NEXUSDEFS_H
#define __NEXUSDEFS_H

// maximum number of states that can be stored; the only limitation is that this
// number be less than the maximum size of an int (not likely to be a problem).
// A good number for this is 76, which is 96 (the number of distinct symbols
// able to be input from a standard keyboard) less 20 (the number of symbols
// symbols disallowed by the Nexus standard for use as state symbols)
//
#define NCL_MAX_STATES         76

#if __BORLANDC__ < 0x550 
	#ifdef __MINMAX_DEFINED
	    #undef __MINMAX_DEFINED
	#endif
    #pragma warn -pch
    #pragma warn .pch
#endif

#include <stdexcept>
#include <algorithm>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <iostream.h>
#include <fstream.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <iomanip.h>
#include <assert.h>

using namespace std;

// lines below needed for access function
#if defined( __BORLANDC__ )
#  include <io.h>
#else
#  include <unistd.h>
#endif

#if (defined __BORLANDC__ || defined __MWERKS__)
	using namespace std;
#endif




#include "nxsstring.h"

typedef std::vector<bool> BoolVect;
typedef std::vector<int> IntVect;
typedef std::vector<nxsstring> LabelList;
typedef std::set< int,less<int> > IntSet;
typedef std::map< int, LabelList, less<int> > LabelListBag;
typedef std::map< nxsstring, nxsstring, less<nxsstring> > AssocList;
typedef std::map< nxsstring, IntSet, less<nxsstring> > IntSetMap;
typedef std::vector<LabelList> AllelesVect;

struct stri_equal : public binary_function<nxsstring,nxsstring,bool> {
   bool operator()(const nxsstring& x, const nxsstring& y) const;
};

#if __BORLANDC__ < 0x550
	// Redefine __MINMAX_DEFINED so Windows header files compile
	#ifndef __MINMAX_DEFINED
    		#define __MINMAX_DEFINED
	#endif
#endif

#endif



