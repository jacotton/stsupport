
 
// $Id: stsupport2.cpp,v 1.15 2002/03/22 12:52:09 jcotton Exp $

/**
 * @file stsupport2.cpp
 *
 * stsupport
 *
 */

#include "ntree.h"
#include "stree.h"
#include "profile.h"
#include "nodeiterator.h"
#include "quartet.h"

//addede JAC 18/03/04 for Support
#include <iterator>
#include <list>
#include <set>
#include <fstream>
#include <algorithm>
#include <numeric>

#ifdef __GNUC__
	#include <strstream>
#else
	#include <sstream>
#endif
#include <ctime>

#if __MWERKS__
	#if macintosh
		// Metrowerks support for Macintosh command line interface
		#include <console.h>
	#endif
#endif

#ifndef min
    #define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
    #define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#define MAJOR_VERSION "0"
#define MINOR_VERSION "1"
#define MINI_VERSION "0"

/*#include "mincut_st.h"
#include "strong_components.h"*/


// Modified SQUID code to handle command line options
#include "getoptions.h"
#define FILENAME_SIZE 256		// Maximum file name length

// Program options
static struct opt_s OPTIONS[] = {

	{ "-b", true, ARG_INT },
	{ "-v", true, ARG_NONE },
};

#define NOPTIONS (sizeof(OPTIONS) / sizeof(struct opt_s))

static char usage[] = "Usage: stsupport [-options] <tree-file> <outfile>\n\
\n\
  Available options: \n\
     -v             show version information\n\
     -b n           set verbosity level\n\
   	 ";


//typedef set<node, less<node> > NodeSet;

// Debugging/verbose flags

bool bAll				= false; //Invesigates all splits, not just on the STree
bool bVerbose			= false; // Write Verbose junk to cout

/**
 * @var  vector <NTree> NTreeVector
 * @brief A vector of NTree's, used to store the input trees
 *
 */
typedef vector <NTree> NTreeVector;


/**
 * @var  STree superTree
 * @brief The supertree
 *
 */
STree superTree;


void GetCladeStrRecursor (NodePtr n, string& s)
{	
	if (n)
	{
		GetCladeStrRecursor (n->GetSibling(),s);
		GetCladeStrRecursor (n->GetChild(),s);
		if (n->IsLeaf())
		{
			s += n->GetLabel();
			s += ",";
		}
	}
}

string GetCladeStrForNode( NodePtr n )
{
	string result = "(";
	GetCladeStrRecursor ( n->GetChild(), result);
	result.erase(result.size()-1,1);
	result  += ")";
	return result;
}

void ShowSplit(IntegerSet* ingroup, IntegerSet* outgroup, Profile<NTree>* p, ostream& os)
{
    os << "{";
    unsigned int cnt = 1;
    for (IntegerSet::iterator i = ingroup->begin(); i != ingroup->end(); i++)
    {
        os << p->GetLabelFromIndex( *i - 1);
        if (cnt != ingroup->size()) os << ",";
        cnt++;
    }
    cnt = 1;  
    os << "|";
    for (IntegerSet::iterator i = outgroup->begin(); i != outgroup->end(); i++)
    {
		os << p->GetLabelFromIndex( *i - 1);
        if (cnt != outgroup->size()) os << ",";
        cnt++;
    }
    os << "}";
}



//------------------------------------------------------------------------------
int main (int argc, char **argv)
{

#if __MWERKS__
#if macintosh
    argc = ccommand(&argv);
#endif
#endif

    // Parse options
    // Heavily borrowed from the squid library
    char *optname;
    char *optarg;
    int   optind;

   
	bVerbose			= false; // Write Verbose junk to cout
	bAll				= false;
	int support_verbose = 0;
	
	while (Getopt(argc, argv, OPTIONS, NOPTIONS, usage,
                  &optind, &optname, &optarg))
    {
        if (strcmp(optname, "-b") == 0) {  support_verbose = atoi(optarg);
            if (support_verbose > 2) cout << "Writing verbose information" << endl;}
		if (strcmp(optname, "-v") == 0)
        {
            cout << "STSSupport " << MAJOR_VERSION << "." MINOR_VERSION << "." << MINI_VERSION << endl;
            cout << "(c)2004 James A. Cotton" << endl;
            cout << "distributed under GPL etc." << endl;
                        exit(EXIT_SUCCESS);
        }
	}
	
    if (argc - optind != 2)
    {
        cerr << "Incorrect number of arguments:" << usage << endl;
        exit (0);
    }
	
	
    // Get options from command line
    char fname[FILENAME_SIZE];
    strcpy( fname, argv[optind++] );
	
	char ofname[FILENAME_SIZE];
	strcpy( ofname, argv[optind++] );
	
    // Check file exists
    FILE* file = fopen( fname, "r" );
    if( file != NULL )
    {
        fclose(file);
        file = NULL;
    }
    else
    {
        cerr << "File \"" << fname << "\" does not exist." << endl;
        exit (0);
    }
	
	
   // cout << "STSupport " << MAJOR_VERSION << "." MINOR_VERSION << "." << MINI_VERSION << endl;

    ifstream f (fname);

	ofstream of (ofname);

    Profile<NTree> p;

    if (!p.ReadTrees (f))
    {
        cerr << "Failed to read trees, bailing out" << endl;
        exit(0);
    }

    p.MakeLabelFreqList ();
	
	 // Create initial multiset of trees T
  	
		  //
    // Retrieve all tokens for this command, stopping only in the event
    // of a semicolon or an unrecognized keyword
       
	cout << "read " << p.GetNumTrees() << endl;
    int StTax,StClades;
	if( p.GetNumTrees() > 1 )
    {
       
        if (support_verbose > 2)
            p.ShowTrees (cout);
      
		int i = 0;  // the tree to be tested!  -the supertree
		
		//these are now in terms of input TREES supporting/conflicting/etc. each node.
		 map<NNodePtr,int > support_count_per_STnode;
        map<NNodePtr,int > conflict_count_per_STnode;
        map<NNodePtr,int> consistent_count_per_STnode;
        map<NNodePtr,int> irrelevant_count_per_STnode;
		
		NTree t1 = p.GetIthTree (i);
        t1.MakeNodeList();
        StTax = t1.GetNumLeaves();
		StClades = t1.GetNumNodes() - t1.GetNumLeaves();
		//setting labels etc right
        for (int jset = 0; jset < t1.GetNumLeaves(); jset++)
        {
            t1[jset]->SetLabelNumber (p.GetIndexOfLabel (t1[jset]->GetLabel()) + 1);
        }
        for (int t1_cl = t1.GetNumLeaves(); t1_cl != t1.GetNumNodes(); t1_cl++)
        {
            NNodePtr insertp = (NNodePtr) t1[t1_cl];
            
            if (insertp != t1.GetRoot() )
            {
                support_count_per_STnode.insert( pair<NNodePtr,int> (insertp,0));
                conflict_count_per_STnode.insert( pair<NNodePtr,int> (insertp,0));
                consistent_count_per_STnode.insert( pair<NNodePtr,int> (insertp,0));
                irrelevant_count_per_STnode.insert( pair<NNodePtr,int> (insertp,0));
            }
        }

			t1.BuildLabelClusters ();
        t1.Update();
        IntegerSet t1_leafset;
        NNodePtr t1root = (NNodePtr) t1.GetRoot(); 
        
        copy(t1root->Cluster.begin(),t1root->Cluster.end(),insert_iterator<IntegerSet>(t1_leafset,t1_leafset.end()));
        vector<int> t1_leafsetv;
        copy(t1root->Cluster.begin(),t1root->Cluster.end(),insert_iterator<vector<int> > (t1_leafsetv,t1_leafsetv.end()));
        vector<int> t1_ingroup;
		
		multiset<double> treecompleteness;
		
		for (int j = 1; j != p.GetNumTrees(); j++) //p.GetNumTrees()
		{
			
			if (support_verbose > 2)
			{
				cout << "-----------------------------------------" << endl;
				cout << "Looking at tree " << j  << endl;
				cout << "-----------------------------------------" << endl;
			}
			//generate the clusters of each tree..
			NTree t2 = p.GetIthTree(j);
			t2.MakeNodeList();
			//set the bits right.
			for (int jset = 0; jset < t2.GetNumLeaves(); jset++)
			{
				t2[jset]->SetLabelNumber (p.GetIndexOfLabel (t2[jset]->GetLabel()) + 1);
			}
			t2.BuildLabelClusters ();
			t2.Update();
			
			treecompleteness.insert( (double) t2.GetNumLeaves() /  (double) StTax );
			
			IntegerSet t2_leafset;
			NNodePtr t2root = (NNodePtr) t2.GetRoot();
			copy(t2root->Cluster.begin(),t2root->Cluster.end(),insert_iterator<IntegerSet>(t2_leafset,t2_leafset.end()));
			
			for (int t1_cl = t1.GetNumLeaves(); t1_cl != t1.GetNumNodes(); t1_cl++)
			{
				//cout << "Looking at ST clade ";
				NNodePtr np = (NNodePtr) t1[t1_cl];
				if (np != t1.GetRoot())
				{
					IntegerSet np_outgroup;
					set_difference(t1_leafset.begin(),t1_leafset.end(),np->Cluster.begin(),np->Cluster.end(),insert_iterator<set<int> >(np_outgroup,np_outgroup.begin()));
									
					//CHECK IF T2 IS RELEVANT TO CLADE..
					IntegerSet i12,o12;
					set_intersection(np->Cluster.begin(),np->Cluster.end(),t2_leafset.begin(),t2_leafset.end(),insert_iterator<IntegerSet>(i12,i12.end()));
					set_intersection(np_outgroup.begin(),np_outgroup.end(),t2_leafset.begin(),t2_leafset.end(),insert_iterator<IntegerSet>(o12,o12.end()));
					if (i12.size() > 0 && o12.size() > 0) 
					{
						//T2 IS RELEVANT.. NEED TO CHECK ALL CLADES IN T2 TO FIND A CONFLICT OR SUPPORT.. IF NEITHER, CONSISTENT
						//CAN BREAK OUT OF t2 NODES LOOP AS SOON AS I GET A SUPPORT OR CONFLICT..
						int t2_cl = t2.GetNumLeaves();
						bool found_conflict = false;
						bool found_support = false;
						while (t2_cl != t2.GetNumNodes() && !found_conflict && !found_support)
						{
							NNodePtr np2 = (NNodePtr) t2[t2_cl];
							if ( np2 != t2.GetRoot())
							{
								IntegerSet np2_outgroup;
								set_difference(t2_leafset.begin(),t2_leafset.end(),np2->Cluster.begin(),np2->Cluster.end(),insert_iterator<set<int> >(np2_outgroup,np2_outgroup.begin()));
					
								//DO THEY SUPPORT?
								IntegerSet ingroupresult;
								IntegerSet outgroupresult;
								set_difference(np2->Cluster.begin(),np2->Cluster.end(),np->Cluster.begin(),np->Cluster.end(),insert_iterator<IntegerSet>(ingroupresult,ingroupresult.end()));
								set_difference(np2_outgroup.begin(),np2_outgroup.end(),np_outgroup.begin(),np_outgroup.end(),insert_iterator<IntegerSet>(outgroupresult,outgroupresult.end()));
								if (ingroupresult.size () == 0 && outgroupresult.size() == 0)
								{
									if (support_verbose > 2)
									{
										cout << "TREE " << j << " SUPPORTS ";
										ShowSplit(&(np->Cluster),&np_outgroup,&p,cout);
										cout << endl;
										
									}
									//add this to the supported nodes statistics
									found_support = true;
								}
								else 
								{       
									//do they conflict?
									IntegerSet i1i2,i1o2,o1i2,o1o2;
									set_intersection(np->Cluster.begin(),np->Cluster.end(),np2->Cluster.begin(),np2->Cluster.end(),insert_iterator<IntegerSet>(i1i2,i1i2.end()));
									set_intersection(np->Cluster.begin(),np->Cluster.end(),np2_outgroup.begin(),np2_outgroup.end(),insert_iterator<IntegerSet>(i1o2,i1o2.end()));
									set_intersection(np_outgroup.begin(),np_outgroup.end(),np2->Cluster.begin(),np2->Cluster.end(),insert_iterator<IntegerSet>(o1i2,o1i2.end()));
									// set_intersection(np_outgroup.begin(),np_outgroup.end(),np2_outgroup.begin(),np2_outgroup.end(),insert_iterator<IntegerSet>(o1o2,o1o2.end()));
									if ( (i1i2.size() > 0) && (i1o2.size() > 0) && (o1i2.size() > 0))// && (o1o2.size() > 0))
									{
										if (support_verbose > 2)
										{
											cout << " TREE " << j << " CONFLICTS WITH ";
											ShowSplit(&(np->Cluster),&np_outgroup,&p,cout);
											cout << endl;
										}
										found_conflict = true;
									} //not conflict
                                } // not support
                            } // tree 2 node not root
							t2_cl++;
                        } //tree 2 nodes loop 
						
						if (found_support && found_conflict) 
						{
							cout << "ERROR - CLADE IS SUPPORTED AND CONFLICTED.. AMBIGUOUS" << endl;
						}
						else if (found_support)
						{   
							map<NNodePtr,int>::iterator i = support_count_per_STnode.find(np);
									if (i != support_count_per_STnode.end())
										(*i).second++;
									else if (support_verbose) cout << "WE HAVE A PROBLEM - CONTACT AUTHOR AND SAY SUPPORT FAIL 1" << endl;
									
						}
						else if (found_conflict)
						{
							map<NNodePtr,int>::iterator i = conflict_count_per_STnode.find(np);
										if (i != conflict_count_per_STnode.end()) 
											(*i).second++;
										else if (support_verbose) cout << "WE HAVE A PROBLEM" << endl;
						}
						else 
						{
							map<NNodePtr,int>::iterator i = consistent_count_per_STnode.find(np);
							if (i != conflict_count_per_STnode.end()) 
								(*i).second++;
							else if (support_verbose) cout << "WE HAVE A PROBLEM" << endl;
						}
					}
					else
					{
						//T2 is IRRELEVANT TO CLADE.. NO NEED TO LOOP 
						
						if (support_verbose > 2)
						{
							
							cout << "tree " << j << " is IRRELEVANT TO " << endl;
							ShowSplit(&(np->Cluster),&np_outgroup,&p,cout);
							cout << endl;
							
						}
						map<NNodePtr,int>::iterator i = irrelevant_count_per_STnode.find(np);
						if (i != irrelevant_count_per_STnode.end()) 
							(*i).second++;
						else if (support_verbose) cout << "WE HAVE A PROBLEM" << endl;
					}   
					
				} //tree1 node not root
			} //tree1 nodes loop
		
			
        } //loop through trees
		
		//NOW READY TO OUTPUT SOME INFORMATION
		//IN THE FORMAT NTREES(tab)
		//cout << "MINIMUM COMPLETENESS IS " << *(min_element(treecompleteness.begin(),treecompleteness.end())) << endl;
		//cout << "MAX COMPLETENESS IS " << *(max_element(treecompleteness.begin(),treecompleteness.end())) << endl;
		double meancompleteness = 0;
		for (multiset<double>::iterator k = treecompleteness.begin(); k != treecompleteness.end(); k++) meancompleteness += *k;
		meancompleteness = meancompleteness / treecompleteness.size();
		int u1 = 0,u2 = 0,u3 = 0,u4 = 0,u5 = 0;
		multiset<double> v1vals,v2vals,v3vals;
		for (map<NNodePtr,int>::iterator k = support_count_per_STnode.begin(); k != support_count_per_STnode.end(); k++)
		{
		//	cout << "Ks=" << (*k).second << endl;
				double v1 = 0;
				double v2 = 0;
				double v3 = 0;
		//		cout << "Kf=" << (*k).first << endl;
				int s = (*k).second;
				int t = p.GetNumTrees() - 1;
				int q,r,p;
				string cladestr = GetCladeStrForNode( (*k).first );
				map<NNodePtr,int>::iterator srch = conflict_count_per_STnode.find( (*k).first );
				if (srch != conflict_count_per_STnode.end() )
				{
					q = (*srch).second;
				}
				else cout << "ERROR PROBLEM MATCHING SUPPORT AND CONFLICT IN u" << endl;
				srch = consistent_count_per_STnode.find( (*k).first );
				if (srch != consistent_count_per_STnode.end() )
				{
					p = (*srch).second;
				}
				else cout << "ERROR PROBLEM MATCHING SUPPORT AND CONSISTENT IN u" << endl;
				srch = irrelevant_count_per_STnode.find( (*k).first );
				if (srch != irrelevant_count_per_STnode.end() )
				{
					r = (*srch).second;
				}
				else cout << "ERROR PROBLEM MATCHING SUPPORT AND IRRELEVANT IN u" << endl;
				
			if (s == 0) 
			{  
				u1++;
				if ( q > 0) u2++;
			//	cout <<  "We have " << q << " conflict and " << (double) (t - r - p) / 2 << " " << ( t - r - p)  << " " << t << " " << r << " " << p << endl;
				if ( !(q < (double)  (t - r ) / 2 ) ) u3++;
				if ( q == ( t - r ) ) u4++;
				if ( q == t) u5++;
			}
			cout << cladestr;
			cout << "\tS=" << s << " Q=" << q << " P=" << p; //<< " S+Q=" << s+q << " s-q=" << s-q << " s-q+p=" << (s-q)+p << " s-q-p=" << (s-q)-p << " s+q+p=" << s+q+p;
			if (s+q != 0)
			{
				v1 =  double (s-q) / double (s+q);
				
			} else v1 = 0;
			if (s + q + p != 0)
			{
				v2 = double (( s- q) + p) / double (s+q+p);
				v3 = double ((s - q) - p) / double (s+q+p);
			} else { v2 = 0; v3 = 0; }
			cout << " v1=" << v1 << " v2=" << v2 << " v3=" << v3 << endl;
			v1vals.insert(v1);
			v2vals.insert(v2);
			v3vals.insert(v3);
		}
		double meanv1 = 0;
		for (multiset<double>::iterator k = v1vals.begin(); k != v1vals.end(); k++) meanv1 += *k;
		meanv1 = meanv1 / v1vals.size();
		double meanv2 = 0;
		for (multiset<double>::iterator k = v2vals.begin(); k != v2vals.end(); k++) meanv2 += *k;
		meanv2 = meanv2 / v2vals.size();
		double meanv3 = 0;
		for (multiset<double>::iterator k = v3vals.begin(); k != v3vals.end(); k++) meanv3 += *k;
		meanv3 = meanv3 / v3vals.size();
	//cout << "TESTIN TESINT" << endl;
		of <<  p.GetNumTrees()-1 << "\t" << StTax << "\t";
		of << meancompleteness << " (" <<  *(min_element(treecompleteness.begin(),treecompleteness.end())) << "," <<  *(max_element(treecompleteness.begin(),treecompleteness.end())) << ")" << "\t";
		of  << StClades-1 << "\t" << u1 << "\t" << u2 << "\t" << u3 << "\t" << u4 << "\t" << u5 << "\t";
		of << meanv1 << " (" << *(min_element(v1vals.begin(),v1vals.end())) << "," << *(max_element(v1vals.begin(),v1vals.end())) << ")" << "\t";
		of << meanv2 << " (" << *(min_element(v2vals.begin(),v2vals.end())) << "," << *(max_element(v2vals.begin(),v2vals.end())) << ")" << "\t";
		of << meanv3 << " (" << *(min_element(v3vals.begin(),v3vals.end())) << "," << *(max_element(v3vals.begin(),v3vals.end())) << ")" << "\t";
		cout <<  p.GetNumTrees()-1 << "\t" << StTax << "\t";
		cout << meancompleteness << " (" <<  *(min_element(treecompleteness.begin(),treecompleteness.end())) << "," <<  *(max_element(treecompleteness.begin(),treecompleteness.end())) << ")" << "\t";
		cout  << StClades-1 << "\t" << u1 << "\t" << u2 << "\t" << u3 << "\t" << u4 << "\t" << u5 << "\t";
		cout << meanv1 << " (" << *(min_element(v1vals.begin(),v1vals.end())) << "," << *(max_element(v1vals.begin(),v1vals.end())) << ")" << "\t";
		cout << meanv2 << " (" << *(min_element(v2vals.begin(),v2vals.end())) << "," << *(max_element(v2vals.begin(),v2vals.end())) << ")" << "\t";
		cout << meanv3 << " (" << *(min_element(v3vals.begin(),v3vals.end())) << "," << *(max_element(v3vals.begin(),v3vals.end())) << ")" << "\t";
	}
    else
    {
        cout << "Needs at least 2 trees - a supertree and at least one input tree" << endl;
    }	
	
	f.close();
	of.close();
  

  
    return 0;
}
