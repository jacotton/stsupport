#include "lcaquery.h"

#define DEBUG_LCA 1
#if DEBUG_LCA
	#include <stdio.h>
#endif

LCAQuery::LCAQuery (Tree *tree)
{
	SetTree (tree);
}

void LCAQuery::SetTree (Tree *tree)
{
	t = tree;
    Initialise ();
}

void SimpleLCAQuery::Initialise ()
{
    PreorderIterator <Node> n (t->GetRoot());

	int count = 0;
    Node *q = n.begin();
    while (q)
    {
    	depth[q] = count++;

#if DEBUG_LCA
        if (!q->IsLeaf())
        {
        	char buf[32];
            sprintf (buf, "%d", (count-1));
            q->SetLabel (buf);
        }
#endif
        q = n.next();
    }
}

NodePtr SimpleLCAQuery::LCA (NodePtr i, NodePtr j)
{
	NodePtr p = i;
    NodePtr q = j;

	while (depth[p] != depth[q])
    {
    	if (depth[p] < depth[q])
        	q = q->GetAnc();
        else
        	p = p->GetAnc();
    }
    return p;
}

