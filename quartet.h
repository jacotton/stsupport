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

 // $Id: quartet.h,v 1.1 2002/03/14 14:11:42 rdmp1c Exp $
 
/**
 * @file quartet.h
 *
 * Compute quartet distance between two trees. Algorithm is based on
 *   Doucette, C. R. 1985. An efficient algorithm to
 *	  compute quartet dissimilarity measures. Unpubl.
 *	  BSc(Hons) dissertation, Dept. Computer Science,
 *	  Memorial University of Newfoundland.
 *
 *
 */

#ifndef QUARTETH
#define QUARTETH

#ifdef __BORLANDC__
	// Undefine __MINMAX_DEFINED so that min and max are correctly defined
	#ifdef __MINMAX_DEFINED
		#undef __MINMAX_DEFINED
	#endif
    // Ignore "Cannot create precompiled header: code in header" message
    // generated when compiling string.cc
    #pragma warn -pch
#endif

#include <vector>

#include "ntree.h"


// Values
typedef struct  {
	int u;		// unresolved in T1 and T2
	int d;      // resolved but different
	int s;      // resolved and same
	int r1;     // resolved in T1 but not T2
	int r2;     // resolved in T2 but not T1
	int x1;     // total resolved in T1
	int n;      // maximum no. of quartets/triplets
	float SD;   // symmetric difference
	float EA;   // explicitly agree
	float SJA;  // strict joint assertions
	float DC;   // do not conflict
}QTValues;


void SummaryStats (QTValues &QR);
void ShowHeader (ostream &s);
void ShowQTRecord (ostream &s, QTValues &QR);
void CompareQuartets (NTree &t1, NTree &t2, QTValues &QR);
void CompareTriplets (NTree &t1, NTree &t2, QTValues &QR);



#if __BORLANDC__
	// Redefine __MINMAX_DEFINED so Windows header files compile
	#ifndef __MINMAX_DEFINED
    		#define __MINMAX_DEFINED
	#endif
#endif

#endif
