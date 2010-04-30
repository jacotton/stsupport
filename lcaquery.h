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

 // $Id: lcaquery.h,v 1.1 2002/03/14 14:11:42 rdmp1c Exp $
 
/**
 * @file lcaquery.h
 *
 * Classes to perform LCA queries on a tree
 *
 */

#ifndef LCAQUERYH
#define LCAQUERYH

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

#include "TreeLib.h"
#include "nodeiterator.h"

/**
 * @class LCAQuery
 * Base class for performing LCA queries. Descendants of this class must
 * override the abstract member function LCA.
 */
class LCAQuery
{
public:
	LCAQuery () { t = NULL; };
	LCAQuery (Tree *tree);
    virtual ~LCAQuery () {};
    virtual NodePtr LCA (NodePtr i, NodePtr j) = 0;
    virtual void SetTree (Tree *tree);
protected:
	Tree *t;
	virtual void Initialise () {};
};

/**
 * @class SimpleLCAQuery
 * Does naive LCA queries by going down the tree until we find the
 * LCA.
 */
class SimpleLCAQuery : public LCAQuery
{
public:
	SimpleLCAQuery () {};
	SimpleLCAQuery (Tree *tree) :  LCAQuery (tree) { Initialise (); };
	/**
	 * Finds LCA by going down tree twowards root until we reach node
     * with the same preorder number.
     * @return LCA of nodes i and j
	 */
    virtual NodePtr LCA (NodePtr i, NodePtr j);
protected:
	std::map<Node *, int, std::less<Node *> > depth;
	/**
	 * Number the nodes in preorder (root = 0).
	 */
	virtual void Initialise ();
};

#if __BORLANDC__
	// Redefine __MINMAX_DEFINED so Windows header files compile
	#ifndef __MINMAX_DEFINED
    		#define __MINMAX_DEFINED
	#endif
#endif

#endif
