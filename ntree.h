//$Id: ntree.h,v 1.5 2001/07/23 13:47:55 rdmp1c Exp $
 
#ifndef NTREEH
#define NTREEH

#include "TreeLib.h"
#include "gtree.h"

#include <set>
#include <map>

/**
 *@typedef  std::set <int, std::less<int> > IntegerSet;
 */
typedef std::set <int, std::less<int> > IntegerSet;

class NTree;

/**
 * @class NNode
 * A node with a cluster of all descendants of that node.
 */
class NNode : public GNode
{
friend class NTree;
public:
	NNode () {};
	/**
	 * A set of indices of all descendants of this node.
	 */		
	IntegerSet Cluster;
};
typedef NNode *NNodePtr;

/**
 *@typedef std::map <string, NNodePtr, less <std::string> > NNodeLabelMap;
 */
typedef std::map <string, NNodePtr, less <std::string> > NNodeLabelMap;

/**
 * @class NTree
 * NTree implements an n-tree, i.e. a tree in which each node has a cluster.
 *
 * In the standard definition of a n-tree the clusters are subsets of
 * @f$S=\{1,\ldots,n\}@f$, where n is the number of leaves in the tree. BuildLeafClusters
 * creates clusters that satisfy this condition. However, in some applications (such as
 * supertrees) the clusters may be subsets of a larger set @f$L \supset S@f$. 
 * BuildLabelClusters creates clusters where the members are the indices of the leaf
 * labels in a larger list (such as the set of labels in a set of trees with overlapping
 * leaf sets). In this case the clusters are subsets of @f$S=\{1,\ldots,m\}@f$, 
 * where @f$m \geq n@f$.
 */
class NTree : public GTree
{
public:
	/**
	 * A map between leaf labels in the profile and a unique integer index
	 */	
	NNodeLabelMap leaf_labels;	
	enum { useLeafNumber, useLabelNumber } use;
	NTree () { use = useLeafNumber; };
	/**
	 * Create all node clusters, where each cluster is a subset of @f$\{1,\ldots,n\}@f$.
	 * Clusters contain the index of the corresponding node in the node list for
	 * this tree.
	 */	
	virtual void BuildLeafClusters ()
    {
    	use = useLeafNumber;
        BuildClustersTraverse ((NNodePtr)Root);
    };
	/**
	 * Create all node clusters, where each cluster is a subset of @f$\{1,\ldots,m\}@f$,
	 * where @f$m \geq n@f$.
	 * Clusters contain the index of the label of each node.
	 */	
	virtual void BuildLabelClusters ()
    {
    	use = useLabelNumber;
        BuildClustersTraverse ((NNodePtr)Root);
    };
	virtual NodePtr NewNode () const { return new NNode; };
	/**
	 * Output the clusters for debugging.
	 */		
	virtual void ShowClusters () { ShowClustersTraverse ((NNodePtr)Root); };
	virtual void BuildLeafLabels () { BuildLeafLabelsTraverse ((NNodePtr)Root); };
protected:
	virtual void BuildClustersTraverse (NNodePtr p);
	virtual void BuildLeafLabelsTraverse (NNodePtr p);
	virtual void ShowClustersTraverse (NNodePtr p);
};

#endif // NTREEH

