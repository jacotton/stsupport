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
 
// $Id: gtree.h,v 1.8 2001/07/24 10:50:37 rdmp1c Exp $

// GNode and GTree describe a tree that can plot itself on an instance of
// gport. This is a start towards platform and format independent graphical
// output of trees.


#ifndef GTREEH
#define GTREEH

#include "TreeLib.h"

#define USE_VRML 0

#if USE_WXWINDOWS
   	#include "wx/wx.h"
#else
	#include "gport.h"
#endif

// Tree styles
#define TS_LEFT     			0x0001
#define TS_RIGHT				0x0002
#define TS_SLANT				0x0004
#define TS_RECTANGLE			0x0008
#define TS_CLADOGRAM			0x0010
#define TS_PHYLOGRAM			0x0020
#define TS_USEINTERNALLABEL		0x0040
#define TS_RADIAL				0x0800
#define TS_ROTATE_LABELS 		0x1000

#define TS_DEFAULT				TS_LEFT | TS_SLANT | TS_CLADOGRAM

class GPlotInfo
{
public:
	int 			count;		// keep track of how many leaves have been drawn
	int 			leaves;		// number of leaves in tree
	double		maxheight;		// maximum height of tree
	int 			lasty;		// last y coordinate
	double		nodegap;		// gap between internals in x coordinates
	double		leafgap;		// gap betwene leaves in y coordinates
	unsigned int	style;		// style of tree drawing
	double		scale;		// scale
	double		leafangle;		// angle of leaf

#if USE_WXWINDOWS
	wxRect		r;
	wxFont		labelfont;
	wxFont		edgefont;
	wxDC			*dc;
#else
	GRect			r;			// device coordinates within which tree is plotted
	GFont 		labelfont;		// font for drawing leaf labels
	GFont 		edgefont;		// font for drawing internal labels
#endif
};


// A node that can draw itself on a GPort
class GNode : public Node
{
public:
	GNode ();
	virtual void Calc (GPlotInfo &plot);
	virtual void Draw (GPlotInfo &plot);
#if USE_WXWINDOWS
	virtual void GetXY (wxPoint &pt) { pt = xy; };
	virtual int  GetX () { return xy.x; };
	virtual int  GetY () { return xy.y; };
#else
	virtual void GetXY (GPoint &pt) { pt = xy; };
	virtual int  GetX () { return xy.GetX(); };
	virtual int  GetY () { return xy.GetY(); };
#endif
#if USE_WXWINDOWS
	virtual void SetXY (wxPoint pt) { xy = pt; };
#else
	virtual void SetXY (GPoint pt) { xy = pt; };
#endif

protected:
#if USE_WXWINDOWS
	wxPoint	xy;
#else
	GPoint xy;
#endif
};
typedef GNode *GNodePtr;



// A tree that can draw itself
class GTree : public Tree
{
public:
	GTree ();
#if USE_WXWINDOWS
	virtual void Plot (wxDC *dc, wxRect r, const wxFont &font,
	unsigned int style = TS_DEFAULT,
	int lineWidth = 1);
#else
	virtual void Plot (GRect r, const GFont &font,
	unsigned int style = TS_DEFAULT,
	int lineWidth = 1);
#endif

	virtual NodePtr NewNode () const { return new GNode; };
#if USE_VRML
	virtual void WriteVRML (ostream &f, GRect r, const GFont &font);
#endif

protected:
	GPlotInfo PlotInfo;

	virtual void calc (GNodePtr p);
	virtual void draw (GNodePtr p);
#if USE_VRML
	virtual void VRMLTraverse (GNodePtr p);
#endif
};
typedef GTree *GTreePtr;



#endif


