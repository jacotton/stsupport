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

 // $Id: quartet.cpp,v 1.2 2002/03/14 16:37:13 rdmp1c Exp $
 
/**
 * @file quartet.cpp
 *
 * Compute quartet distance between two trees.
 *
 */

#include "quartet.h"
#include "nodeiterator.h"
#include "lcaquery.h"

#include <iomanip.h>
#ifdef __GNUC__
	#include <algo.h>
#endif


#define DEBUG_QUARTETS 0


class ECODE
{
public:
	ECODE (Tree *tree);
    virtual ~ECODE ();
	virtual void EncodeNode (NNodePtr r);
    virtual void EncodeTree (int ni, int nj);
	virtual bool Visit (NNodePtr p);
    void Write (ostream &s);
	virtual int  GetLeaves () { return n; };
	int *E[3];
protected:
    int vertex;
    int subtree;
    int n;
	int i;
	int j;
	Tree * t;
    SimpleLCAQuery lca;
    IntegerSet mVisited;
};

class TCODE : public ECODE
{
public:
	TCODE (Tree *tree) : ECODE (tree) {};
    virtual ~TCODE () {};
    virtual void EncodeTree (int ni, int nj);
	virtual bool Visit (NNodePtr p);
};




// Local routines

long n_choose_2 (int n);
long n_choose_3 (int n);
long n_choose_4 (int n);
void RadixSort (ECODE &E1, ECODE &E2, int &s, int &r, int &x);



// Store a tuple representing the vertex and subtree codes in the two trees
class Tuple
{
public:
	int Leaf;
    int Vertex1;
    int Subtree1;
    int Vertex2;
    int Subtree2;
    Tuple () { Leaf = Vertex1 = Subtree1 = Vertex2 = Subtree2 = 0; };
    int operator< (const Tuple &t) const
    {
    	if (Vertex1 < t.Vertex1)
        	return 1;
        else if (Vertex1 > t.Vertex1)
        	return 0;
        else // Vertex1 ==
        {
        	if (Subtree1 < t.Subtree1)
            	return 1;
            else if (Subtree1 > t.Subtree1)
            	return 0;
            else // Subtree1 ==
            {
                if (Vertex2 < t.Vertex2)
                    return 1;
                else if (Vertex2 > t.Vertex2)
                    return 0;
                else // Vertex2 ==
                {
                    if (Subtree2 < t.Subtree2)
                        return 1;
                    else if (Subtree2 > t.Subtree2)
                        return 0;
                    else return 0;
                }
            }
        }

 	};

    int operator== (const Tuple &t) const
    {
    	return (
        	(t.Vertex1 == Vertex1)
        && (t.Subtree1 == Subtree1)
       	&& (t.Vertex2 == Vertex2)
       	&& (t.Subtree2 == Subtree2) );
    };
};


//------------------------------------------------------------------------------
long n_choose_2 (int n)
{
	if (n < 2)
		return 0;
	else
		return  (n * (n-1)) / 2;
}


//------------------------------------------------------------------------------
long n_choose_3 (int n)
{
	if (n < 3)
		return 0;
	else
		return  (n * (n-1) * (n-2)) / 6;
}

//------------------------------------------------------------------------------
long n_choose_4 (int n)
{
	if (n < 4)
		return 0;
	else
		return  (n * (n-1) * (n-2) * (n-3)) / 24;
}


//------------------------------------------------------------------------------
ECODE::ECODE (Tree *tree)
{
	t = tree;

    n = t->GetNumLeaves();

    E[0] = new int [n+1];
    E[1] = new int [n+1];
    E[2] = new int [n+1];

	for (int k = 1; k <=n; k++)
	{
		E[0][k] = k;
		E[1][k] = 0;
		E[2][k] = 0;
	}
	vertex = subtree = 0;

    lca.SetTree (t);
}

//------------------------------------------------------------------------------
ECODE::~ECODE ()
{
	delete [] E[0];
	delete [] E[1];
	delete [] E[2];
}

//------------------------------------------------------------------------------
// Node should be encoded if it is not on the path <i,j> and its cluster is
// not a subset of {1..i}
bool ECODE::Visit (NNodePtr p)
{
    return (!p->IsMarked() && !(p->Cluster <= mVisited));
/*	SetRelations s = p->ClusterRelationship (Visited);
	return (!p->IsFlag (NF_MARKED) && (s != rdmpIDENTITY && s != rdmpSUBSET));
*/
}


//------------------------------------------------------------------------------
/*    isit desc of node and encode.
	  Each node that does not
	  include i or j as a descendant
	  is a new subtree of the current
	  vertex:

			    i    a b  c
				 \   |  \/
				  \  |  /
			  	   \ 1 2
					\|/
					 +

			  + = vertex
			  1 = subtree 1
			  2 = subtree 2

	  A vertex with no subtrees meeting the
	  requirements of ECODE::Visit is irrelevant
	  to the problem. These cases are flagged by
	  subtree=0.
*/
void ECODE::EncodeNode (NNodePtr r)
{
	subtree = 0;
	NNodePtr q = r;
	while (q)
	{
		if (Visit (q))
		{
			subtree++;

            IntegerSet::iterator nit = q->Cluster.begin();
            IntegerSet::iterator nend = q->Cluster.end();
            while (nit != nend)
            {
            	if ((*nit) >= i)
                {
	            	E[1][*nit] = vertex;
    	        	E[2][*nit] = subtree;
                }
                nit++;
            }
		}
		q = (NNodePtr)(q->GetSibling ());
	}
}



//------------------------------------------------------------------------------
/*
	  Visit each node on path <i,j> and encode
		desc leaves. Note that leaves below lub(i,j)
		are also "descendants" of lub (i,j) since T
		is an unrooted tree. These leaves are all
		on the same subtree.

				 i = 2
				 j = 4

				  1  5 2    3   4
				   \/   \   |   /
					\    \  |  /
					 \    \ | /
					  \    \|/
					   \    +
						\  /
						 \/

				 is really

					 2---+---4
						 / \
						/   \
				    1--+     3
					   |
					   5

	

*/
void ECODE::EncodeTree (int ni, int nj)
{
	i = ni;
	j = nj;
	NNodePtr inode = (NNode *)(*t)[i - 1];
	NNodePtr jnode = (NNode *)(*t)[j - 1];

    NNodePtr lub = (NNode *)lca.LCA (inode, jnode);
    vertex = 1;

    mVisited.erase(mVisited.begin(), mVisited.end());
    for (int k = 1; k <= i; k++)
    	mVisited.insert (k);

    // Mark path ij
 	NNodePtr q = inode;
    while (q != lub)
    {
        q->SetMarked (true);
        q = (NNode *)(q->GetAnc());
    }
	q = jnode;
    while (q != lub)
    {
        q->SetMarked (true);
        q = (NNode *)(q->GetAnc());
    }



	// i -- lub(i,j)
    q = (NNode *)(inode->GetAnc());
    while (q->IsMarked())
    {
        EncodeNode ((NNode *)(q->GetChild()));
        if (subtree)
        	vertex++;
        q = (NNode *)(q->GetAnc());
    }


	// j--lub(i,j)
    q = (NNode *)(jnode->GetAnc());
    while (q->IsMarked())
    {
        EncodeNode ((NNode *)(q->GetChild()));
        if (subtree)
        	vertex++;
        q = (NNode *)(q->GetAnc());
    }

	// visit lub (i,j)
	EncodeNode ((NNode *)(q->GetChild()));
	subtree++;

	for (int k = i + 1; k <= n; k++)
	{
    	bool kIsElement = (q->Cluster.find(k) != q->Cluster.end());
		if ((k != j) && !kIsElement)
		{
			E[1][k] = vertex;
			E[2][k] = subtree;
		}
	}

    // Unmark path between i and j
 	q = inode;
    while (q != lub)
    {
        q->SetMarked (false);
        q = (NNode *)(q->GetAnc());
    }
	q = jnode;
    while (q != lub)
    {
        q->SetMarked (false);
        q = (NNode *)(q->GetAnc());
    }
}


//------------------------------------------------------------------------------
void ECODE::Write (ostream &s)
{
	s << endl;
	int k;
	for (k = 1; k <= n; k++)
		s << setw(3) << E[0][k];
	s << endl;
	for (k = 1; k <= n; k++)
		s << "---";
	s << endl;
	for (k = 1; k <= n; k++)
		s << setw(3) << E[1][k];
	s << endl;
	for (k = 1; k <= n; k++)
		s << setw(3) << E[2][k];
	s << endl;
}


//------------------------------------------------------------------------------
// Replace original code from Douchette and COMPONENT by a simple use of
// STL.
void RadixSort (ECODE &E1, ECODE &E2, int &s, int &r, int &x)
{
	std::vector <Tuple> E;
	for (int i = 1; i <= E1.GetLeaves (); i++)
    {
    	Tuple t;
        t.Vertex1 = E1.E[1][i];
        t.Subtree1 = E1.E[2][i];
        t.Vertex2 = E2.E[1][i];
        t.Subtree2 = E2.E[2][i];
		E.push_back (t);
    }

#if DEBUG_QUARTETS
    // Show
    cout << "Before sort" << endl;
	for (int i = 0; i < E.size(); i++)
    {
    	cout << " " << i;
    }
    cout << endl;
	for (int i = 0; i < E.size(); i++)
    {
    	cout << "--";
    }
    cout << endl;
	for (int i = 0; i < E.size(); i++)
    {
    	cout << " " << E[i].Vertex1;
    }
    cout << endl;
	for (int i = 0; i < E.size(); i++)
    {
    	cout << " " << E[i].Subtree1;
    }
    cout << endl;
	for (int i = 0; i < E.size(); i++)
    {
    	cout << "--";
    }
    cout << endl;
 	for (int i = 0; i < E.size(); i++)
    {
    	cout << " " << E[i].Vertex2;
    }
    cout << endl;
	for (int i = 0; i < E.size(); i++)
    {
    	cout << " " << E[i].Subtree2;
    }
    cout << endl;
	for (int i = 0; i < E.size(); i++)
    {
    	cout << "==";
    }
   cout << endl;
#endif

   sort (E.begin(), E.end());

#if DEBUG_QUARTETS

       // Show
    cout << "After sort" << endl;
	for (int i = 0; i < E.size(); i++)
    {
    	cout << " " << i;
    }
    cout << endl;
	for (int i = 0; i < E.size(); i++)
    {
    	cout << "--";
    }
    cout << endl;
	for (int i = 0; i < E.size(); i++)
    {
    	cout << " " << E[i].Vertex1;
    }
    cout << endl;
	for (int i = 0; i < E.size(); i++)
    {
    	cout << " " << E[i].Subtree1;
    }
    cout << endl;
	for (int i = 0; i < E.size(); i++)
    {
    	cout << "--";
    }
    cout << endl;
 	for (int i = 0; i < E.size(); i++)
    {
    	cout << " " << E[i].Vertex2;
    }
    cout << endl;
	for (int i = 0; i < E.size(); i++)
    {
    	cout << " " << E[i].Subtree2;
    }
    cout << endl;
	for (int i = 0; i < E.size(); i++)
    {
    	cout << "==";
    }
   cout << endl;
#endif

   // Count

	int a = 0;
	int b = 0;
	int c = 0;
	int d = 0;
	s = 0;
	r = 0;
	x = 0;
	int insubtree1 = 0;
	int insubtree2 = 0;
	int innode2 = 0;

	for (unsigned int k = 0; k < E.size(); k++)
	{
    	if (E[k].Vertex1 != 0)
        {
			if (  (d != E[k].Subtree2)
				|| (c != E[k].Vertex2)
				|| (b != E[k].Subtree1)
				|| (a != E[k].Vertex1))
			{
				s += n_choose_2 (insubtree2);
				r += insubtree2 * innode2;
				innode2 += insubtree2;
				insubtree2 = 0;
				d = E[k].Subtree2;
			}

			if (   (c != E[k].Vertex2)
				|| (b != E[k].Subtree1)
				|| (a != E[k].Vertex1))
			{
				innode2 = 0;
				c = E[k].Vertex2;
			}

			if (  (b != E[k].Subtree1) || (a != E[k].Vertex1))
			{
				x += n_choose_2 (insubtree1);
				insubtree1 = 0;
				b = E[k].Subtree1;
				a = E[k].Vertex1;
			}
		insubtree1++;
		insubtree2++;
		}
	}
	s += n_choose_2 (insubtree2);
	x += n_choose_2 (insubtree1);
	r += insubtree2 * innode2;
#if DEBUG_QUARTETS
    cout << "s=" << s << " x=" << x << " r=" << r << endl;
#endif
}


//------------------------------------------------------------------------------
void ShowQTRecord (ostream &s, QTValues &QR)
{
	s 	<< setprecision (3) << setw (6) << setiosflags (ios::right)
		<< setw (6)<< QR.SD << " "
		<< setw (6)<< QR.EA << " "
		<< setw (6)<< QR.DC << " "
		<< setw (6)<< QR.SJA << " "
		<< setw (8)<< QR.n
		<< setw (8)<< QR.s
		<< setw (8)<< QR.d
		<< setw (8)<< QR.r1
		<< setw (8)<< QR.r2
		<< setw (8)<< QR.u
		<< setw (8)<< endl;
}

//------------------------------------------------------------------------------
void ShowHeader (ostream &s)
{
	s << "    SD     EA     DC    SJA        n       s       d      r1      r2       u" << endl;
	s << "----------------------------------------------------------------------------" << endl;
}

//------------------------------------------------------------------------------
void SummaryStats (QTValues &QR)
{
	QR.d  = QR.x1 - (QR.s + QR.r1);
	QR.u  = QR.n - (QR.s + QR.d + QR.r1 + QR.r2);
	QR.SD = float (2 * QR.d + QR.r1 + QR.r2)/ float (2 * QR.d + 2* QR.s + QR.r1 + QR.r2);
	QR.EA = float (QR.d + QR.r1 + QR.r2 + QR.u)/ float (QR.n);
	QR.DC = float (QR.d) / float (QR.n);
	int tmp = QR.d + QR.s;
	if (tmp == 0)
		QR.SJA = -1;
	else
		QR.SJA = float (QR.d) / float (QR.d + QR.s);
}

//------------------------------------------------------------------------------
void CompareQuartets (NTree &t1, NTree &t2, QTValues &QR)
{
	// Clear indices
	QR.SD = 0.0;
	QR.EA = 0.0;
	QR.SJA = 0.0;
	QR.DC = 0.0;
	QR.d = 0;
	QR.s = 0;
	QR.r1 = 0;
	QR.r2 = 0;
	QR.x1 = 0;
	QR.u = 0;
	QR.n = n_choose_4 (t1.GetNumLeaves ());
 	for (int i = 1; i < t1.GetNumLeaves (); i++)
	{
		for (int j = i + 1; j <= t1.GetNumLeaves (); j++)
		{
			int SS, RR1, RR2, XX1, XX2;

//            cout << "i=" << i << " (" << t1[i-1]->GetLabel() << ") " << " j=" << j << " (" << t1[j-1]->GetLabel() << ") "<< endl;
//           cout << "i=" << i << " (" << t2[i-1]->GetLabel() << ") " << " j=" << j << " (" << t2[j-1]->GetLabel() << ") "<< endl;

            ECODE E1 (&t1);
            ECODE E2 (&t2);

            E1.EncodeTree (i, j);
            E2.EncodeTree (i, j);
			RadixSort (E1, E2, SS, RR1, XX1);

            QR.s += SS;
            QR.r1 += RR1;
            QR.x1 += XX1;

 			RadixSort (E2, E1, SS, RR2, XX2);
            QR.r2 += RR2;
        }
 	}

}

//------------------------------------------------------------------------------
// Node should be encoded if it is not on the path <j,root>.
bool TCODE::Visit (NNodePtr p)
{
    return (!p->IsMarked());
}


//------------------------------------------------------------------------------
/*
	{ Doucette's algorithm for quartets takes each
	  pair of leaves in the tree and hangs the
	  remaining subtrees from the path connecting
	  those two leaves, e.g.:

			  1   2   3   4        1---*---*---4
               \   \   \   /            |   |
				\   \   \ /     =       |   |
				 \   \   *              2   3
				  \   \ /
					\   *
					 \ /
					  *

	  In triplets, one "leaf" is always the root,
	  so the path is simply those nodes between the
	  leaf and the root, e.g.:

			  1   2   3   4        root---*---*---*---4
			  \   \   \   /               |   |   |
				\   \   \ /     =          |   |   |
				 \   \   *                 1   2   3
				  \   \ /
					\   *
					 \ /
					  *
					  |
					  |
					root
*/
void TCODE::EncodeTree (int ni, int nj)
{
	i = 1; // Set to 1 so we can use ECODE::Encode
	j = nj;

	NNodePtr jnode = (NNode *)(*t)[j - 1];


    // Mark path to root
 	NNodePtr q = jnode;
    while (q != NULL)
    {
        q->SetMarked (true);
        q = (NNode *)(q->GetAnc());
    }

    vertex = 1;
	q = (NNode *)(jnode->GetAnc());
    while (q)
    {
    	if (q->IsMarked())
        {
	        EncodeNode ((NNode *)(q->GetChild()));
        	if (subtree)
        		vertex++;
        }
        q = (NNode *)(q->GetAnc());
    }

 	q = jnode;
    while (q != NULL)
    {
        q->SetMarked (false);
        q = (NNode *)(q->GetAnc());
    }

}

//------------------------------------------------------------------------------
void CompareTriplets (NTree &t1, NTree &t2, QTValues &QR)
{
	// Clear indices
	QR.SD = 0.0;
	QR.EA = 0.0;
	QR.SJA = 0.0;
	QR.DC = 0.0;
	QR.d = 0;
	QR.s = 0;
	QR.r1 = 0;
	QR.r2 = 0;
	QR.x1 = 0;
	QR.u = 0;
	QR.n = n_choose_3 (t1.GetNumLeaves ());
 	for (int j = 1; j <= t1.GetNumLeaves (); j++)
	{
        int SS, RR1, RR2, XX1, XX2;

//        cout << "j=" << j << endl;

        TCODE E1 (&t1);
        TCODE E2 (&t2);

        E1.EncodeTree (1, j);
        E2.EncodeTree (1, j);
        RadixSort (E1, E2, SS, RR1, XX1);

        QR.s += SS;
        QR.r1 += RR1;
        QR.x1 += XX1;

        RadixSort (E2, E1, SS, RR2, XX2);
        QR.r2 += RR2;
 	}

}
