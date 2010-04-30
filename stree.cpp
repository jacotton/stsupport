#include "stree.h"

void STree::AddLeaf(const std::string label)
{
	Leaves++;
	CurNode->SetLeaf(true);
	CurNode->SetWeight(1);
	CurNode->SetLeafNumber(Leaves);
	CurNode->SetLabel (label);
}


void STree::AddCherry (const std::string label1, const std::string label2)
{
	NodePtr p = NewNode();
    Leaves++;
    p->SetLeaf (true);
	p->SetWeight(1);
	p->SetLeafNumber(Leaves);
	p->SetLabel (label1);

	NodePtr	q = NewNode();
    Leaves++;
    q->SetLeaf (true);
	q->SetWeight(1);
	q->SetLeafNumber(Leaves);
	q->SetLabel (label2);

    CurNode->SetChild(p);
    p->SetAnc(CurNode);
    p->SetSibling (q);
    q->SetAnc (CurNode);

    CurNode->SetDegree (2);
    CurNode->SetWeight (2);
    Internals++;
}

void STree::AddSubtree (Tree &T, bool asChild)
{
	// Copy the subtree
	NodePtr p = T.CopyOfSubtree (T.GetRoot());

    // Add it to our tree
    if (asChild)
    {
    	CurNode->SetChild (p);
    	p->SetAnc(CurNode);
    	CurNode->IncrementDegree();
        CurNode->AddWeight(T.GetNumNodes());
	}
    else
    {
		NodePtr	ancestor = CurNode->GetAnc();
    	CurNode->SetSibling(p);
    	p->SetAnc(ancestor);
    	ancestor->AddWeight(T.GetNumNodes());
    	ancestor->IncrementDegree();
     }
	Internals += T.GetNumInternals ();
    Leaves += T.GetNumLeaves ();
    CurNode = p;
}
