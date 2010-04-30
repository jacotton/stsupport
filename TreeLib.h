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

// $Id: TreeLib.h,v 1.23 2002/03/12 14:54:40 rdmp1c Exp $


#ifndef TREELIB_H
#define TREELIB_H

#ifdef __BORLANDC__
	// Undefine __MINMAX_DEFINED so that min and max are correctly defined
	#ifdef __MINMAX_DEFINED
		#undef __MINMAX_DEFINED
	#endif
    // Ignore "Cannot create precompiled header: code in header" message
    // generated when compiling string.cc
    #pragma warn -pch
#endif


#include <fstream>
#include <string>
#include <vector>
#include <stack>
#include <map>


#ifdef __BORLANDC__
    #pragma warn .pch
#endif



//using namespace std;

// Graphics characters for drawing trees

#ifdef __WIN32__
	// OEM character set
	//Compressed tree
	#define HBAR 196  // Ä
	#define VBAR 179  // ³
	#define SIB  195  // Ã
	#define BOT  192  // À
	#define ROOT 218  // Ú
	#define DOWN 194  // Â

	// Regular tree
	#define TEE  180  // ´
	#define LEFT ROOT
	#define RIGHT BOT

#else
    // ANSI character set
	#define HBAR 45
	#define VBAR 124
	#define SIB  43
	#define BOT  92
	#define ROOT 43
	#define DOWN 43
	#define TEE  124
//	#define LEFT 47  
//	#define RIGHT 92
	#define LEFT 43
	#define RIGHT 43

#endif

std::string NEXUSString (const std::string s);


class Tree;


class Node
{
friend class Tree;
public:
	Node ();
	virtual ~Node () {}; 
	
	virtual void 	AddWeight (int w) { Weight += w; };
	virtual void	AppendLabel (char *s) { Label += s; };
	virtual void	AppendLabel (std::string s) { Label += s; };

	virtual void 	Copy (Node* theCopy);

	virtual Node 	*GetAnc () { return Anc; };
	virtual Node 	*GetChild () { return Child; };
	virtual int 		GetDegree () { return Degree; };
	virtual int 		GetDepth () { return Depth; };
	virtual float	GetEdgeLength () { return Length; };
	virtual int		GetHeight () { return Height; };
	 virtual int		GetIndex () { return Index; };
	virtual std::string 	GetLabel () { return Label; };
	virtual int		GetLabelNumber () { return LabelNumber; };
	virtual int		GetLeafNumber () { return LeafNumber; };
	virtual float	GetPathLength () { return PathLength; };
	virtual Node 	*GetRightMostSibling ();
	virtual Node 	*GetSibling () { return Sib; };
	virtual int 		GetWeight() { return Weight; };
	
	
	virtual void 	IncrementDegree () { Degree++; };
	virtual bool 	IsLeaf () { return Leaf; };
	virtual bool 	IsALeftDescendantOf (Node *q);
  	virtual bool 	IsMarked () { return Marked; };
	virtual bool	IsTheChild () { return  (Anc->Child == this); };
	
	virtual Node	*LeftSiblingOf ();

	virtual void 	SetAnc (Node *p) { Anc = p; };
	virtual void 	SetChild (Node *p) { Child = p; };
	virtual void 	SetDegree (int d) { Degree = d; };
	virtual void 	SetDepth (int d) { Depth = d; };
	virtual void	SetEdgeLength (float e) { Length = e; };
	virtual void 	SetHeight (int h) { Height = h; };
	virtual void 	SetIndex (int i) { Index = i;};
	virtual void 	SetLeaf (bool on) { Leaf = on; };
	virtual void 	SetLeafNumber (int i) { LeafNumber = i;};
	virtual void 	SetLabel (std::string s) { Label = s; };
	virtual void 	SetLabel (char *s) { Label = s; };
	virtual void 	SetLabelNumber (int i) { LabelNumber = i;};
	virtual void	SetMarked (bool on) { Marked = on; };
	virtual void 	SetPathLength (float l) { PathLength = l;};
	virtual void 	SetSibling (Node *p) { Sib = p; };
	virtual void 	SetWeight (int w) { Weight = w; };



protected:
	Node 			*Child;
	Node 			*Sib;
	Node 			*Anc;
	int 			Weight;
	std::string		Label;
	float			Length;
	bool			Leaf;
	int			Height;
	bool			Marked;
	int 			Degree;
	int			LeafNumber;
	int			Depth;
	float			PathLength;
	int			Index;
	int			LabelNumber;
};
typedef Node *NodePtr;


#define errSYNTAX 			1
#define errENDOFSTRING		2
#define errMISSINGLPAR		3
#define errUNBALANCED		4
#define errSTACKNOTEMPTY	5
#define errSEMICOLON		6


class Tree
{
public:
	Tree ();
	Tree (const Tree &t);
	virtual ~Tree ();

	virtual void 	AddNodeBelow (NodePtr Node, NodePtr Below);

	virtual NodePtr 	CopyOfSubtree (NodePtr RootedAt);

#if defined __BORLANDC__ && (__BORLANDC__ < 0x0550)
	virtual void 	Draw (ostream &f);
#else
 	virtual void 	Draw (std::ostream &f);
#endif

	virtual NodePtr 	GetCurNode() { return CurNode; };
	virtual int  	GetError () { return Error; };
	virtual std::string	GetErrorMsg ();
	virtual bool	GetHasEdgeLengths () const { return EdgeLengths; };
	virtual bool 	GetHasInternalLabels () const { return InternalLabels; };
	virtual NodePtr GetLeafWithLabel (std::string s);
	virtual std::string  	GetName () const { return Name; };
	virtual void 	GetNodeDepths ();
	virtual int		GetNumInternals () const { return Internals; };
	virtual int 	GetNumLeaves () const { return Leaves; };
	virtual int		GetNumNodes () const { return Leaves + Internals; };
	virtual NodePtr	GetRoot () const { return Root; };
	virtual double	GetWeight() const { return Weight; };

	virtual bool	IsRooted () const { return Rooted; };


	virtual void	MakeChild ();
	virtual void	MakeCurNodeALeaf (int i);
	virtual void	MakeRoot ();
	virtual void	MakeSibling ();
   	virtual void 	MakeNodeList ();

	virtual void	MarkNodes (bool on);
	virtual NodePtr NewNode () const { return new Node; };

	virtual int 	Parse (const char *TreeDescr);

#if defined __BORLANDC__ && (__BORLANDC__ < 0x0550)
	virtual int		Read (istream &f);
#else
	virtual int		Read (std::istream &f);
#endif

	virtual NodePtr 	RemoveNode (NodePtr Node);

	virtual void	SetCurNode (NodePtr p) { CurNode = p; };
	virtual void	SetEdgeLengths (bool on) { EdgeLengths = on; };
	virtual void 	SetInternalLabels (bool on) { InternalLabels = on; };
	virtual void	SetName (const std::string s) { Name = s; };
	virtual void	SetRooted (bool on) { Rooted = on; };
	virtual void	SetWeight (const double w) { Weight = w; };

	virtual void	Update ();


#if defined __BORLANDC__ && (__BORLANDC__ < 0x0550)
	virtual void	Write (ostream &f);
#else
	virtual void	Write (std::ostream &f);
#endif

	NodePtr		operator[] (const int i) { return Nodes[i]; };

protected:
	NodePtr			Root;						// Root of tree
	NodePtr			CurNode;					// Current node
	int				Leaves;					// Number of leaves
	int				Internals;					// Number of internal nodes
	int				Error;
	std::string			Name;					// Name of tree (e.g., for NEXUS trees)
	NodePtr			*Nodes;					// Array of nodes
	std::map<std::string, int, std::less<std::string> > LeafList;	// Quick lookup of leaves by label

	bool				InternalLabels;				// Flag for displaying internal node labels
	bool				EdgeLengths;				// Flag for displaying edge lengths
	bool				Rooted;					// Flag for rooted/unrooted

	std::string			Line;						// Buffer for drawing trees to text streams
	int				MaxHeight;
	float				MaxPathLength;
	
	double			Weight;

#if defined __BORLANDC__ && (__BORLANDC__ < 0x0550)
	ostream				*treeStream;
#else
	std::ostream		*treeStream;				// Pointer to stream used to draw trees to
#endif

	int				count;

	virtual void 		traverse (NodePtr p);

	virtual void 		buildtraverse (NodePtr p);
   	virtual void 		copyTraverse (NodePtr p1, NodePtr &p2) const;
   	virtual void 		deletetraverse (NodePtr p);
	virtual void 		drawAsTextTraverse (NodePtr p);
	virtual void 		drawLine (NodePtr p, bool isChild = false);
	virtual void 		fillInAncestors (NodePtr p);
	virtual void 		drawPendantEdge (NodePtr p);
	virtual void 		drawInteriorEdge (NodePtr p);
	virtual void 		getNodeDepth(NodePtr p);
	virtual void 		getNodeHeights (NodePtr p);
	virtual void 		getPathLengths (NodePtr p);
	virtual void		markNodes (NodePtr p, bool on);
	virtual void 		makeNodeList (NodePtr p);

};
typedef Tree *TreePtr;

#if defined __BORLANDC__ && (__BORLANDC__ < 0x0550)
inline istream& operator >> (istream& is, Tree &tree)
{
	tree.Read (is);
	return is;
}

inline ostream& operator << (ostream& os, Tree &tree)
{
	tree.Write (os);
	return os;
}
#else
inline std::istream& operator >> (std::istream& is, Tree &tree)
{
	tree.Read (is);
	return is;
}

inline std::ostream& operator << (std::ostream& os, Tree &tree)
{
	tree.Write (os);
	return os;
}
#endif


#ifdef __BORLANDC__
	// Redefine __MINMAX_DEFINED so Windows header files compile
	#ifndef __MINMAX_DEFINED
    		#define __MINMAX_DEFINED
	#endif
#endif


#endif // TREELIB_H

