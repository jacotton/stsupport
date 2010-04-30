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

 // $Id: nodeiterator.h,v 1.2 2002/02/24 20:37:17 rdmp1c Exp $
 
/**
 * @file nodeiterator.h
 *
 * Iterate over nodes in a tree
 *
 */

#ifndef NODEITERATORH
#define NODEITERATORH

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
#include <stack>

/**
 * @class NodeIterator
 * Iterator class that visits nodes in a tree in post order. Uses a stack to keep
 * track of place in tree. By making this a template class we can apply it to
 * any descendant of the class Node.
 *
 */
template <class N> class NodeIterator
{
public:
	/**
	 * Constructor takes the root of the tree as a parameter.
     * @param r the root of the tree
	 */
	NodeIterator (N *r)  { root =  r; };
    virtual ~NodeIterator () {};
    
	/**
	 * Initialises the iterator and returns the  first node.
	 * @return The first node of the tree
	 */
    virtual N *begin ();
 	/**
	 * Moves to the next node in the tree.
	 * @return The next node in the tree, or NULL if all nodes have been visited.
	 */
   virtual N *next ();


protected:
	N *root;
    N *cur;
	std::stack < N *, std::vector<N *> > stk;
};


template <class N> class PreorderIterator : public NodeIterator<N>
{
public:
	PreorderIterator (N *r) : NodeIterator<N> (r) {};
    virtual N *begin ();
	virtual N *next ();
};





template <class N> N *NodeIterator<N>::begin ()
{
	cur = root;
	while (cur->GetChild())
    {
    	stk.push (cur);
        cur = (N *)(cur->GetChild());
    }
    return cur;
}

template <class N> N *NodeIterator<N>::next ()
{
	if (stk.empty())
    	cur = NULL;
    else
    {
    	if (cur->GetSibling())
        {
        	N *p = (N *)(cur->GetSibling());
            while (p->GetChild())
            {
            	stk.push (p);
                p = (N *)(p->GetChild());
            }
            cur = p;
		}
        else
        {
        	cur = stk.top();
            stk.pop();
        }
    }
    return cur;
}

template <class N> N *PreorderIterator<N>::begin ()
{
	NodeIterator<N>::cur = NodeIterator<N>::root;
    return NodeIterator<N>::cur;
}

template <class N> N *PreorderIterator<N>::next ()
{
	if (NodeIterator<N>::cur->GetChild())
    {
       	NodeIterator<N>::stk.push (NodeIterator<N>::cur);
       	N *p = (N *)(NodeIterator<N>::cur->GetChild());
        NodeIterator<N>::cur = p;
    }
    else
    {
      	while (!NodeIterator<N>::stk.empty() && (NodeIterator<N>::cur->GetSibling() == NULL))
        {
            NodeIterator<N>::cur = NodeIterator<N>::stk.top();
        	NodeIterator<N>::stk.pop();
        }
        if (NodeIterator<N>::stk.empty())
           	NodeIterator<N>::cur = NULL;
        else
        {
        	N *p = (N *)(NodeIterator<N>::cur->GetSibling());
            NodeIterator<N>::cur = p;
        }
    }
    return NodeIterator<N>::cur;
}


#if __BORLANDC__
	// Redefine __MINMAX_DEFINED so Windows header files compile
	#ifndef __MINMAX_DEFINED
    		#define __MINMAX_DEFINED
	#endif
#endif



#endif
