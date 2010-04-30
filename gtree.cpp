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
  
// $Id: gtree.cpp,v 1.7 2001/07/24 10:50:37 rdmp1c Exp $


#include "gtree.h"
#include <stdio.h>
#include <math.h>


//------------------------------------------------------------------------------
GNode::GNode ()
{
}

//------------------------------------------------------------------------------
void GNode::Calc (GPlotInfo &plot)
{
	if (IsLeaf()) //----- leaf 
	{
#if USE_WXWINDOWS
		xy.y = plot.r.GetTop() + plot.count * plot.leafgap;
		plot.lasty = xy.y;
#else
		xy.SetY ((int)(plot.r.GetTop() + plot.count * plot.leafgap));
		plot.lasty = xy.GetY();
#endif
		plot.count++;

		// Cladogram
		if ((plot.style & TS_CLADOGRAM) == TS_CLADOGRAM)
		{
#if USE_WXWINDOWS
			xy.x = plot.r.GetRight();
#else
			xy.SetX (plot.r.GetRight());
#endif
		}
		// Phylogram
		if ((plot.style & TS_PHYLOGRAM) == TS_PHYLOGRAM)
		{
			if ((plot.style & TS_LEFT) == TS_LEFT)
			{
#if USE_WXWINDOWS
				xy.x = plot.r.GetLeft() + ((PathLength / plot.maxheight) * plot.r.GetWidth());
#else
				xy.SetX ((int)(plot.r.GetLeft() + ((PathLength / plot.maxheight) * plot.r.GetWidth())));
#endif
			}
			else
			{
#if USE_WXWINDOWS
				xy.x = plot.r.GetRight() + ((PathLength / plot.maxheight) * plot.r.GetWidth());
#else
				xy.SetX ((int)(plot.r.GetRight() + ((PathLength / plot.maxheight) * plot.r.GetWidth())));
#endif
			}
		}
	}
	else // ------- internal node
	{
		//  Cladogram (no branch lengths)
		if ((plot.style & TS_CLADOGRAM) == TS_CLADOGRAM)
		{
			if ((plot.style & TS_LEFT) == TS_LEFT)
			{
#if USE_WXWINDOWS
				xy.x = plot.r.GetLeft() + plot.nodegap * (plot.leaves - Weight);
#else
				xy.SetX ((int)(plot.r.GetLeft() + plot.nodegap * (plot.leaves - Weight)));
#endif
			}
			else
			{
#if USE_WXWINDOWS
				xy.x = plot.r.GetRight() - plot.nodegap * (plot.leaves - Weight);
#else
				xy.SetX ((int)(plot.r.GetRight() - plot.nodegap * (plot.leaves - Weight)));
#endif
			}
			if ((plot.style & TS_SLANT) == TS_SLANT) // -- slant style tree
			{			
#if USE_WXWINDOWS
				xy.y = plot.lasty - (((Weight - 1) * plot.leafgap) / 2);
#else
 				xy.SetY ((int)(plot.lasty - (((Weight - 1) * plot.leafgap) / 2)));
#endif
			}
			else // -- rectangular style tree
			{
#if USE_WXWINDOWS
				wxPoint leftPt, rightPt;
				((GNode *)Child)->GetXY (leftPt);
				((GNode *)Child->GetRightMostSibling())->GetXY (rightPt);
				xy.y = leftPt.y + (rightPt.y - leftPt.y) / 2;
#else
				GPoint leftPt, rightPt;
				((GNode *)Child)->GetXY (leftPt);
				((GNode *)Child->GetRightMostSibling())->GetXY (rightPt);
				xy.SetY (leftPt.GetY() + (rightPt.GetY() - leftPt.GetY()) / 2);
#endif
			}
		}
		else // -- Phylogram (branch lengths)
		{
			if ((plot.style & TS_LEFT) == TS_LEFT)
			{
#if USE_WXWINDOWS
				xy.x = plot.r.GetLeft() + ((PathLength / plot.maxheight) * plot.r.GetWidth());
#else
				xy.SetX ((int)(plot.r.GetLeft() + ((PathLength / plot.maxheight) * plot.r.GetWidth())));
#endif
			}
			else
			{
#if USE_WXWINDOWS
				xy.x = plot.r.GetRight() + ((PathLength / plot.maxheight) * plot.r.GetWidth());
#else
				xy.SetX ((int)(plot.r.GetRight() + ((PathLength / plot.maxheight) * plot.r.GetWidth())));
#endif
			}
#if USE_WXWINDOWS
			wxPoint leftPt, rightPt;
			((GNode *)Child)->GetXY (leftPt);
			((GNode *)Child->GetRightMostSibling())->GetXY (rightPt);
			xy.y = leftPt.y + (rightPt.y - leftPt.y) / 2;
#else
			GPoint leftPt, rightPt;
			((GNode *)Child)->GetXY (leftPt);
			((GNode *)Child->GetRightMostSibling())->GetXY (rightPt);
			xy.SetY (leftPt.GetY() + (rightPt.GetY() - leftPt.GetY()) / 2);
#endif
		}
	}
}

//-----------------------------------------------------------------------------
void GNode::Draw (GPlotInfo &plot)
{
#if USE_WXWINDOWS
	if (IsLeaf ())
		plot.dc->SetFont(plot.labelfont);
	else
		plot.dc->SetFont(plot.edgefont);

	if ((plot.style & TS_SLANT) == TS_SLANT)
	{
		// Draw line
		if (Anc)
		{
			wxPoint  ancXY;
			((GNodePtr)Anc)->GetXY (ancXY);
			plot.dc->DrawLine (xy.x, xy.y, ancXY.x, ancXY.y);
		}
	}
    	// rectangular tree
	if ((plot.style & TS_RECTANGLE) == TS_RECTANGLE)
	{
		if (Anc)
		{
			wxPoint  pt;
			((GNodePtr)Anc)->GetXY (pt);
			pt.y = xy.y;
			plot.dc->DrawLine (pt.x, pt.y, xy.x, xy.y);
		}
		if (!IsLeaf())
		{
			plot.dc->DrawLine (xy.x, ((GNodePtr)Child)->GetY(),
			xy.x, ((GNodePtr)Child->GetRightMostSibling())->GetY());
		}
	}


	// Draw leaf label
	if (IsLeaf() || (plot.style & TS_USEINTERNALLABEL))
	{
		if (Label != "")
		{
			int labelX = xy.x;
			int labelY = xy.y;
			wxCoord w, h, descent;
			wxString s (Label.c_str(), wxSTRING_MAXLEN);
			//            plot.dc->GetTextExtent (s, &w, &h, &descent);
			labelX += plot.dc->GetCharWidth();
			labelY -= plot.dc->GetCharHeight()/2;
			plot.dc->DrawText (s, labelX, labelY);
		}
	}
#else
	if (IsLeaf ())
		Port.SetCurrentFont (plot.labelfont);
	else
		Port.SetCurrentFont (plot.edgefont);

	if ((plot.style & TS_SLANT) == TS_SLANT)
	{
		// Draw line
		if (Anc)
		{
			GPoint  ancXY;
			((GNodePtr)Anc)->GetXY (ancXY);
			Port.DrawLinePts (xy, ancXY);
		}
	}
	// rectangular tree
	if ((plot.style & TS_RECTANGLE) == TS_RECTANGLE)
	{
		if (Anc)
		{
			GPoint  pt;
			((GNodePtr)Anc)->GetXY (pt);
			pt.SetY (xy.GetY());
			if (Port.GetCurrentDevice () == devPostscript)
			{
				// This is a hack so that the two lines join nicely
				pt.SetX (pt.GetX() - Port.GetPenWidth()/2);
			}
			Port.DrawLinePts (pt, xy);
		}
		if (!IsLeaf())
		{
			GPoint pt1 (xy.GetX(), ((GNodePtr)Child)->GetY());
			GPoint pt2 (xy.GetX(), ((GNodePtr)Child->GetRightMostSibling())->GetY());
			Port.DrawLinePts (pt1, pt2);
		}
	}


	// Draw leaf label
	if (IsLeaf() || (plot.style & TS_USEINTERNALLABEL))
	{
		if (Label != "")
		{
			int labelY = xy.GetY();
			(IsLeaf() ? labelY += plot.labelfont.GetSize()/3 : labelY += plot.edgefont.GetSize()/3 );
			Port.DrawText (xy.GetX() + plot.labelfont.GetSize()/2, labelY, Label.c_str());
		}
	}
#endif
}


//------------------------------------------------------------------------------
GTree::GTree ()
{
}

//------------------------------------------------------------------------------
void GTree::calc (GNodePtr p)
{
	if (p)
	{
		calc ((GNodePtr)p->GetChild());
		p->Calc (PlotInfo);
		calc ((GNodePtr)p->GetSibling());
	}
}

//--------------------------------------------------------------------------
// Draw the tree
void GTree::draw (GNodePtr p)
{
	if (p)
	{
		draw ((GNodePtr)p->GetChild());
		p->Draw (PlotInfo);
		draw((GNodePtr)p->GetSibling());
	}
}

#if USE_WXWINDOWS
//------------------------------------------------------------------------------
void GTree::Plot (wxDC *dc, wxRect r, const wxFont &font, unsigned int style,
	int lineWidth)
{
	PlotInfo.dc = dc;
	PlotInfo.r = r;

	wxPen pen (*wxBLACK, lineWidth, wxSOLID);
	PlotInfo.dc->SetPen (pen);

	PlotInfo.count	= 0;
	PlotInfo.leaves	= Leaves;
	PlotInfo.lasty	= 0;
	PlotInfo.maxheight   = 0.0;
	PlotInfo.style		= style;
	PlotInfo.labelfont	= font;
	PlotInfo.edgefont	= font;

	// Space for a scale bar
	int scalebar_space = dc->GetCharHeight() * 2;
	if ((style & TS_PHYLOGRAM) == TS_PHYLOGRAM)
	{
		PlotInfo.r.SetBottom (PlotInfo.r.GetBottom() - scalebar_space);
	}

	double w = PlotInfo.r.GetWidth();
	double h = PlotInfo.r.GetHeight();
	double l = Leaves - 1.0;

	PlotInfo.nodegap = w/l;
	PlotInfo.leafgap = h/l;


	if ((style & TS_PHYLOGRAM) == TS_PHYLOGRAM)
	{
		Root->SetPathLength (0.0);
		MaxPathLength = 0.0;
		getPathLengths (Root);
		PlotInfo.maxheight = MaxPathLength;

		// Draw scale bar using "log" scale
		float m = log10 (PlotInfo.maxheight);
		int i = (int) m - 1;
		float bar = pow (10.0, i);
		int scalebar = (int)((bar / PlotInfo.maxheight) * PlotInfo.r.GetWidth());

		wxPoint pt1, pt2;
		if (PlotInfo.style & TS_LEFT)
		{
			pt1.x = PlotInfo.r.GetLeft();
			pt1.y = PlotInfo.r.GetBottom() + scalebar_space/2;
			pt2 = pt1;
			pt2.x += scalebar;
		}
		else
		{
			pt2.x = PlotInfo.r.GetRight();
			pt2.y = PlotInfo.r.GetBottom() + scalebar_space/2;
			pt1 = pt2;
			pt1.x -= scalebar;
		}

		// Pen to draw scale line
		wxPen scale_pen (*wxBLACK, 1, wxSOLID);
		dc->SetPen (scale_pen);
		dc->DrawLine (pt1.x, pt1.y, pt2.x, pt2.y);

		// Restore tree for the pen
		dc->SetPen (pen);

		// Label the bar
		wxString scale_label;
		if (i >= 0)
		{
			scale_label.Printf ("%d", int (bar));
		}
		else
		{
			int j = abs (i);
			scale_label.Printf ("%.*f", j, bar);
		}

		dc->SetFont (PlotInfo.edgefont);
		if (PlotInfo.style & TS_LEFT)
		{
			dc->DrawText (scale_label,
			pt2.x + dc->GetCharWidth(),
			pt2.y - dc->GetCharHeight()/2);
		} 
	}

	// Compute coordinates then draw
	calc ((GNodePtr)Root);
	draw ((GNodePtr)Root);
}

#else
//------------------------------------------------------------------------------
void GTree::Plot (GRect r, const GFont &font, unsigned int style,
	int lineWidth)
{
	PlotInfo.r = r;

    Port.SetPenWidth (lineWidth);
//	plot.r.Inset (0, Port.FontHeight () / 2);

	PlotInfo.count	= 0;
	PlotInfo.leaves	= Leaves;
	PlotInfo.lasty	= 0;
	PlotInfo.maxheight   = 0.0;
	PlotInfo.style		= style;
	PlotInfo.labelfont	= font;
 	PlotInfo.edgefont	= font;

    // Space for a scale bar
	int scalebar_space = PlotInfo.edgefont.GetSize() * 2;
	if ((style & TS_PHYLOGRAM) == TS_PHYLOGRAM)
	{
		PlotInfo.r.SetBottom (PlotInfo.r.GetBottom() - scalebar_space);
	}

	double w = PlotInfo.r.GetWidth();
    double h = PlotInfo.r.GetHeight();
	double l = Leaves - 1.0;

	PlotInfo.nodegap = w/l;
	PlotInfo.leafgap = h/l;


	if ((style & TS_PHYLOGRAM) == TS_PHYLOGRAM)
	{
    	Root->SetPathLength (0.0);
        MaxPathLength = 0.0;
        getPathLengths (Root);
        PlotInfo.maxheight = MaxPathLength;

			// Draw scale bar using "log" scale
			float m = log10 (PlotInfo.maxheight);
			int i = (int) m - 1;
			float bar = pow (10.0, i);
			int scalebar = (int)((bar / PlotInfo.maxheight) * PlotInfo.r.GetWidth());

			GPoint pt1, pt2;
			if (PlotInfo.style & TS_LEFT)
			{
            	pt1.SetX (PlotInfo.r.GetLeft());
                pt1.SetY (PlotInfo.r.GetBottom() + scalebar_space/2);
                pt2 = pt1;
                pt2.Offset (scalebar, 0);
			}
			else
			{
           		pt2.SetX (PlotInfo.r.GetRight());
                pt2.SetY (PlotInfo.r.GetBottom() + scalebar_space/2);
                pt1 = pt2;
                pt1.Offset (-scalebar, 0);
			}
            int w = Port.GetPenWidth ();
            Port.SetPenWidth (1);
            Port.DrawLinePts (pt1, pt2);


            // Label the bar
			char buf[20];
			if (i >= 0)
			{
				sprintf (buf, "%d", int (bar));
			}
			else
			{
				int j = abs (i);
				sprintf (buf, "%.*f", j, bar);
			}

            Port.SetCurrentFont (PlotInfo.edgefont);
            if (PlotInfo.style & TS_LEFT)
            {
        		Port.DrawText (pt2.GetX() + PlotInfo.edgefont.GetSize()/2,
                	pt2.GetY() + PlotInfo.edgefont.GetSize()/3, buf);
            }
            Port.SetPenWidth (w);
    }


	// Compute coordinates then draw
	calc ((GNodePtr)Root);
	draw ((GNodePtr)Root);
}
#endif

#if USE_VRML
void GTree::WriteVRML (ostream &f, GRect r, const GFont &font)
{
	treeStream = &f;

	PlotInfo.r = r;

	PlotInfo.count	= 0;
	PlotInfo.leaves	= Leaves;
	PlotInfo.lasty	= 0;
	PlotInfo.maxheight   = 0.0;
	PlotInfo.labelfont	= font;
 	PlotInfo.edgefont	= font;

	double w = PlotInfo.r.GetWidth();
    double h = PlotInfo.r.GetHeight();
	double l = Leaves - 1.0;

	PlotInfo.nodegap = w/l;
	PlotInfo.leafgap = h/l;

	// Compute coordinates then draw
	calc ((GNodePtr)Root);
	VRMLTraverse ((GNodePtr)Root);

	f << endl;



}

void GTree::VRMLTraverse (GNodePtr p)
{
	if (p)
	{
		VRMLTraverse ((GNodePtr)p->GetChild());

        if (p != Root)
        {
            *treeStream << endl;
            *treeStream << "# Root = (" << ((GNodePtr)Root)->GetX() << ", " << ((GNodePtr)Root)->GetY() << ") ";
            *treeStream << "p = (" << p->GetX() << ", " << p->GetY() << ") " << endl;

            // Node's ancestor relative to origin
            float dx = ((GNodePtr)Root)->GetX() - ((GNodePtr)p->GetAnc())->GetX();
            float dy = ((GNodePtr)Root)->GetY() - ((GNodePtr)p->GetAnc())->GetY();

            // Node relative to its ancestor
            float dax = ((GNodePtr)p->GetAnc())->GetX() - p->GetX();
            float day = ((GNodePtr)p->GetAnc())->GetY() - p->GetY();


            *treeStream << "\tTransform {" << endl;
            *treeStream << "\t	  #           X  Y  Z" << endl;
            *treeStream << "\t   translation " << (int)(dy + day/2) << " " << (int)(dx + dax/2) << " 0" << endl;

            float length = sqrt ((dax*dax)+(day*day));

            *treeStream << "\t   scale 1 " << length << " 1" << endl;

            float angle =  asin (day/length);

            *treeStream << "\t   rotation 0 0 1 " << angle << endl;

            //cout << "\t   rotation 0 0 1 0.78" << endl;
            *treeStream << "\t   children [" << endl;
            *treeStream << "\t      USE EDGE" << endl;
            *treeStream << "\t   ]" << endl;
            *treeStream << "\t}" << endl;
            *treeStream << endl;

            if (p->IsLeaf())
            {
                // Output a leaf label
                // Node's position relative to origin
                float dx = ((GNodePtr)Root)->GetX() - ((GNodePtr)p)->GetX();
                float dy = ((GNodePtr)Root)->GetY() - ((GNodePtr)p)->GetY();

                *treeStream << "\tTransform {" << endl;
                *treeStream << "\t	   rotation 0 0 -1 1.57" << endl; //Note an extra 90 degrees rotation
                *treeStream << "\t	  #           X  Y  Z" << endl;
                *treeStream << "\t   translation " << (int)dy << " " << (int)dx << " 0" << endl;
                *treeStream << "\t   children [" << endl;
                *treeStream << "\t      Shape {" << endl;
                *treeStream << "\t         appearance Appearance {" << endl;
                *treeStream << "\t            material Material {" << endl;
                *treeStream << "\t            }" << endl;
                *treeStream << "\t         }" << endl;
                *treeStream << "\t         geometry Text {" << endl;
                *treeStream << "\t              string \"" << p->GetLabel() << "\"" << endl;
                *treeStream << "\t              fontStyle FontStyle {" << endl;
                *treeStream << "\t              size 300" << endl;
                *treeStream << "\t              justify \"LEFT\"" << endl;
                *treeStream << "\t              }" << endl;
                *treeStream << "\t         }" << endl;
                *treeStream << "\t      }" << endl;
                *treeStream << "\t   ]" << endl;
                *treeStream << "\t}" << endl;

            }
        }
		VRMLTraverse((GNodePtr)p->GetSibling());
	}
}
#endif

