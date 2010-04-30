#ifndef STREEH
#define STREEH

#include "ntree.h"

#include <string>
#include <stack>

/**
 * @class STree
 * STree is a tree class to support constructing mincut supertrees. It extends
 * Tree to add a stack for keeping track of the growing supertree.
 *
 */
class STree : public NTree
{
public:
	STree () { stk.empty(); };
	/**
	 * Make the current node a leaf
	 * @param label leaf label
	 */
	virtual void AddLeaf (const std::string label);
	/**
	 * Add a two-leaf tree (a "cherry") to the tree
	 * @param label1 first leaf label
	 * @param label1 second leaf label
  	 */
    virtual void AddCherry (const std::string label1, const std::string label2);
	/**
	 * Add the subtree T to the tree
	 * @param T the subtree
  	 */
    virtual void AddSubtree (Tree &T, bool asChild);
	/**
	 * Put CurNode onto the stack on nodes
	 * @param label leaf label
	 */
    virtual void PushNode () { stk.push (CurNode); };
	/**
	 * Pop a node off the stack and set CurNode to the top of the
     * stack.
  	 */
    virtual void PopNode () { CurNode = stk.top(); stk.pop (); };
protected:
	std::stack< NodePtr, std::vector<NodePtr> > stk;


};



#endif
