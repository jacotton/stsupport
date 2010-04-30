#include "ntree.h"

#ifdef __GNUC__
#include <algo.h>
#endif


void NTree::BuildClustersTraverse (NNodePtr p)
{
	if (p)
    {
    	p->Cluster.erase (p->Cluster.begin(), p->Cluster.end());
        BuildClustersTraverse ((NNodePtr)(p->GetChild()));
        BuildClustersTraverse ((NNodePtr)(p->GetSibling()));
        if (p->IsLeaf())
        {
        	switch (use)
            {
            	case useLeafNumber:
        			p->Cluster.insert (p->GetLeafNumber());
                    break;

                case useLabelNumber:
        			p->Cluster.insert (p->GetLabelNumber());
                    break;
            }
        }
        if (p !=Root)
        {
        	NNodePtr anc = (NNodePtr)(p->GetAnc());
			std::set<int, less<int> > temp_set;
			std::set_union(anc->Cluster.begin(), anc->Cluster.end(),
				p->Cluster.begin(), p->Cluster.end(),
				std::inserter(temp_set, temp_set.begin()));
			anc->Cluster.swap(temp_set);
        }
    }
}

void NTree::BuildLeafLabelsTraverse (NNodePtr p)
{
	if (p)
    {
        BuildLeafLabelsTraverse ((NNodePtr)(p->GetChild()));
        BuildLeafLabelsTraverse ((NNodePtr)(p->GetSibling()));
        if (p->IsLeaf())
        {
			leaf_labels[p->GetLabel()] = p;
        }
    }
}

// debugging
//------------------------------------------------------------------------------
void NTree::ShowClustersTraverse (NNodePtr p)
{
	if (p)
	{
        ShowClustersTraverse ((NNodePtr)(p->GetChild()));
        // Show clusters
		cout << "{ ";
		std::copy (p->Cluster.begin(), p->Cluster.end(),
			std::ostream_iterator<int>(cout, " "));
		cout << "}" << endl;
        ShowClustersTraverse ((NNodePtr)(p->GetSibling()));
	}
}


