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
 
// $Id: treereader.h,v 1.2 2001/07/24 10:50:40 rdmp1c Exp $

#ifndef TREEREADER_H
#define TREEREADER_H

#include "TreeLib.h"
#include "tokeniser.h"

class TreeReader
{
public:
	TreeReader ();
	TreeReader (Tokeniser &p);
	virtual ~TreeReader () {};

	virtual bool Read (TreePtr t);
	virtual bool MoreTrees() { return true; };

protected:
	Tokeniser &parser;
	 Tree *tree;
	std::string errormsg;


	virtual void 	doAdjust() = 0;

//	virtual void	GetTaxonName ();
	virtual bool 	LabelEdge ();
	virtual bool 	LabelLeaf (std::string s);
	virtual void 	LabelInternalNode (std::string s);

//	virtual int 	ReadName() { return 0; };
};

class PHYLIPReader : public TreeReader
{
public:
	PHYLIPReader ();
	PHYLIPReader (Tokeniser &p) : TreeReader (p) {};
protected:
	virtual void 	doAdjust();
};

#endif


