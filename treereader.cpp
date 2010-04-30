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
 
 // $Id: treereader.cpp,v 1.4 2002/03/05 17:23:29 rdmp1c Exp $

#include "treereader.h"

#if __MWERKS__
	#include <string.h>
	#include <stdlib.h>
#endif

TreeReader::TreeReader (Tokeniser &p) : parser (p)
{
}

// Parse the edge length token.
bool TreeReader::LabelEdge ()
{
	bool result = false;
	Tokeniser::tokentype token = parser.GetNextToken ();

	// Handle negative branch lengths by peeking at the token. If
	// it is a '-' then call ParseNumber to ensure it handles the sign
	// correctly.
	if (token == Tokeniser::MINUS)
		result = parser.ParseNumber();
	 else
		result = (token == Tokeniser::NUMBER);

	if (result)
	{
		// Convert token to a number
		char number_string[128], *endptr;

		strncpy (number_string, parser.GetToken().c_str(), sizeof (number_string));
		double value = strtod (number_string, &endptr);
		if (*endptr == '\0' || endptr == NULL)
		{
#ifdef __BORLANDC__
			if (value == HUGE_VAL)
			{
				errormsg = "The number ";
				errormsg += parser.GetToken();
				errormsg += " caused strtod to report a HUGE_VAL error";
			}
else
			{
#endif
				// Set -ve branch lengths to zero
				if (value < 0.0)
				value = 0.0;
				tree->GetCurNode()->SetEdgeLength (value);
				tree->SetEdgeLengths (true);
#ifdef __BORLANDC__
			}
#endif
		}
		else
		{
			errormsg = "The token ";
			errormsg += parser.GetToken();
			errormsg += " is not a valid number";
		}
	}
	return result;
}


// Label the current leaf
bool TreeReader::LabelLeaf (std::string s)
{
	tree->MakeCurNodeALeaf (tree->GetNumLeaves() + 1);
	tree->GetCurNode()->SetLabel (s);
	return true;
}

// Label the current internal node
void TreeReader::LabelInternalNode (std::string s)
{
	tree->GetCurNode()->SetLabel (s);
	tree->SetInternalLabels (true);
}

// Uses a simple pushdown automaton to read Newick-style trees
bool TreeReader::Read (TreePtr t)
{
	// States of pushdown automaton that reads trees
	enum statetype
	{
		GETNAME,
		GETINTERNODE,
		NEXTMOVE,
		DOSIBLING,
		FINISHCHILDREN,
		ACCEPTED,
		CLEANUP,
		QUIT
	} state;

	std::stack< NodePtr, std::vector<NodePtr> > stk;
	Tokeniser::tokentype 	token;

	tree = t;
	tree->MakeRoot();
	token = parser.GetNextToken ();

	if (token == Tokeniser::EMPTY)
		return false;

	// Parse the tree description
	state = GETNAME;
	while ((state != QUIT) && (state != ACCEPTED))
	{
		switch (state)
		{
			case GETNAME:
				switch (token)
				{
					case Tokeniser::STRING:
					case Tokeniser::NUMBER:
						LabelLeaf (parser.GetToken());
						token = parser.GetNextToken ();
						state = GETINTERNODE;
						break;
					case Tokeniser::LPAR:
						state = NEXTMOVE;
						break;
					default:
						errormsg = "Syntax error [GETNAME]: expecting a \"(\" or leaf name, got \"";
						errormsg += parser.GetToken();
						errormsg += "\" instead";
						state = QUIT;
						break;
				}
				break;

			case GETINTERNODE:
				switch (token)
				{
					case Tokeniser::COLON:
					case Tokeniser::COMMA:
					case Tokeniser::RPAR:
						state = NEXTMOVE;
						break;
					default:
						errormsg = "Syntax error [GETINTERNODE]: expecting one of \":,)\", got ";
						errormsg += parser.GetToken();
						errormsg += " instead";
						state = QUIT;
						break;
				}
				break;

			case NEXTMOVE:
				switch (token)
				{
					case Tokeniser::COLON:
						if (LabelEdge ())
							token = parser.GetNextToken ();
						else
							state = QUIT;
						break;
					// The next node encountered will be a sibling
					// of Curnode and a descendant of the node on
					// the top of the node stack.
					case Tokeniser::COMMA:
						if (stk.empty())
						{
							errormsg = "Tree description unbalanced, this \")\" has no matching \"(\"";
							state = QUIT;
						}
						else
						{
							tree->MakeSibling ();
							token = parser.GetNextToken ();
							state = GETNAME;
						}
						break;
					// The next node will be a child of CurNode, hence
					// we create the node and push CurNode onto the
					// node stack.
					case Tokeniser::LPAR:
						stk.push (tree->GetCurNode());
						tree->MakeChild();
						token = parser.GetNextToken ();
						state = GETNAME;
						break;
					// We've finished ready the descendants of the node
					// at the top of the node stack so pop it off.
					case Tokeniser::RPAR:
						if (stk.empty())
						{
							errormsg = "Tree description unbalanced (an extra \")\")";
							state = QUIT;
						}
						else
						{
							NodePtr q = stk.top();
							q->AddWeight(tree->GetCurNode()->GetWeight());
							tree->SetCurNode (q);
							stk.pop ();
							token = parser.GetNextToken ();
							state = FINISHCHILDREN;
						}
						break;
					// We should have finished the tree
					case Tokeniser::SEMICOLON:
						if (stk.empty())
						{
							state = ACCEPTED;
						}
						else
						{
							errormsg = "Tree description ended prematurely (stack not empty)";
							state = QUIT;
						}
						break;
					default:
						errormsg = "Syntax error [NEXTMOVE]: expecting one of \":,();\", got ";
						errormsg += parser.GetToken();
						errormsg += " instead";
						state = QUIT;
						break;
				}
				break;

			case FINISHCHILDREN:
				switch (token)
				{
					case Tokeniser::STRING:
					case Tokeniser::NUMBER:
						LabelInternalNode (parser.GetToken());
						token = parser.GetNextToken ();
						break;
					case Tokeniser::COLON:
						if (LabelEdge ())
							token = parser.GetNextToken ();
						else
							state = QUIT;
						break;
					// We've completed traversing the descendants of the
					// node at the top of the stack, so pop it off.
					case Tokeniser::RPAR:
						if (stk.empty())
						{
							errormsg = "Tree description unbalanced, this \")\" has no matching \"(\"";
							state = QUIT;
						}
						else
						{
							NodePtr q = stk.top();
							q->AddWeight(tree->GetCurNode()->GetWeight());
							tree->SetCurNode (q);
							stk.pop ();
							token = parser.GetNextToken ();
						}
						break;

					// The node at the top of the stack still has some
					// descendants.
					case Tokeniser::COMMA:
						if (stk.empty())
						{
							errormsg = "Tree description unbalanced, missing a \"(\"";
							state = QUIT;
						}
						else
						{
							tree->MakeSibling ();
							token = parser.GetNextToken ();
							state = GETNAME;
						}
						break;
					case Tokeniser::SEMICOLON:
						state = NEXTMOVE;
						break;
					default:
						if (stk.empty())
						{
							errormsg = "Tree description unbalanced";
							state = QUIT;
						}
						else
						{
							errormsg = "Syntax error [FINISHCHILDREN]: expecting one of \":,();\" or internal label, got ";
							errormsg += parser.GetToken();
							errormsg += " instead";
						}
						state = QUIT;
						break;
				}
				break;
		}
	}
	// Handle errors
	if (state == QUIT)
	{
		// Clean memory here...

		// ... then throw exception
		 throw XTokeniser (errormsg, parser.GetFilePosition(),
			parser.GetFileLine (), parser.GetFileColumn());
	}
	else
	{
		tree->GetRoot()->SetWeight(tree->GetNumLeaves());
		doAdjust ();
	}
    return true;
}

// Unrooted PHYLIP trees have degree > 2
void PHYLIPReader::doAdjust ()
{
	tree->SetRooted (tree->GetRoot()->GetDegree() == 2);
}



