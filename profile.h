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
 
 // $Id: profile.h,v 1.21 2002/03/19 09:26:23 rdmp1c Exp $
 
/**
 * @file profile.h
 *
 * Storage of trees
 *
 */
#ifndef PROFILE_H
#define PROFILE_H

#ifdef __BORLANDC__
	// Undefine __MINMAX_DEFINED so that min and max are correctly defined
	#ifdef __MINMAX_DEFINED
		#undef __MINMAX_DEFINED
	#endif
    // Ignore "Cannot create precompiled header: code in header" message
    // generated when compiling string.cc
    #pragma warn -pch
#endif

#include "TreeLib.h"
#include "gtree.h"

#include "treereader.h"
#include "treewriter.h"

// NCL includes
#include "nexusdefs.h"
#include "xnexus.h"
#include "nexustoken.h"
#include "nexus.h"
#include "taxablock.h"
#include "assumptionsblock.h"
#include "treesblock.h"
#include "discretedatum.h"
#include "discretematrix.h"
#include "charactersblock.h"
#include "datablock.h"

#if USE_VC2
	#include "VMsg.h"
#endif

#if USE_WXWINDOWS
   	#include "wx/wx.h"
#endif

#if (__BORLANDC__ < 0x0550)
	#include <time.h>
#else
	#include <ctime>
#endif    


/**
 *@typedef map <std::string, int, less<std::string> > LabelMap;
 */
typedef map <std::string, int, less<std::string> > LabelMap;


/**
 * @class Profile
 * Encapsulates reading and storing a set of trees.
 *
 */
template <class T> class Profile
{
public:
	/**
	 * A map between leaf labels in the profile and a unique integer index
	 */	
	LabelMap Labels;
	/**
	 * For each leaf label the number of trees in the profile that have the corresponding leaf
	 */	
	LabelMap LabelFreq;
	
	/**
	 * Constructor
	 */
	Profile () {};
	/**
	 * Destructor
	 */
	virtual ~Profile () {};

	/**
	 * @brief The ith tree in the profile
	 *
	 * @param i the index of the tree in the range 0 - (n-1)
	 * @return The tree
	 */
	 virtual T GetIthTree (int i) { return Trees[i]; };
	/**
	 * @brief The name of the ith tree in the profile
	 *
	 * @param i the index of the tree in the range 0 - (n-1)
	 * @return The tree
	 */
	 virtual std::string GetIthTreeName (int i) { return Trees[i].GetName(); };
	/**
	 * @return The number of labels in the profile
	 */
	virtual int GetNumLabels () { return Labels.size(); };
	/**
	 * @return The number of trees in the profile
	 */
	virtual int GetNumTrees () { return Trees.size(); };
	/**
	 * @brief The index of a leaf label
	 * @param s A leaf label
	 * @return The index of the leaf label
	 */
	virtual int GetIndexOfLabel (string s);
	/**
	 * @brief The unique index of a leaf label
	 * @param i A leaf index
	 * @return The ith leaf label
	 * @sa Profile::GetIndexOfLabel
	 */
	virtual std::string GetLabelFromIndex (int i) { return LabelIndex[i]; };

	/**
	 * @brief Assign a unique integer index to each leaf label in the profile
	 */
	virtual void MakeLabelList ();
	/**
	 * @brief Count the number of trees each leaf label occurs in
	 */
	virtual void MakeLabelFreqList ();
	/**
	 * @brief Read a NEXUS file and store the trees in Profile::trees. 
	 
	 * If a TAXA block is
	 * present then the leaf labels are stored in Labels in the same order as in
	 * the TAXA block, otherwise Profile::MakeLabelList is called to assign a unique integer
	 * index to each label.
	 *
	 * @param f input stream in NEXUS format
	 * @return true if sucessful
	 */
	virtual bool ReadNEXUS (istream &f);
	/**
	 * @brief Read a PHYLIP tree file and store the trees in Profile::trees. 
	 *
	 * @param f input stream in PHYLIP format
	 * @return true if sucessful
	 */
 	virtual bool ReadPHYLIP (istream &f);
	/**
	 * @brief Read a set of trees from an input stream
	 * At present only PHYLIP and NEXUS formats are supported
	 * @param f input stream 
	 * @return true if successful
	 */
	virtual bool ReadTrees (istream &f);
	/**
	 * @brief Output leaf labels.
	 *
	 * @param f output stream
	 */
	virtual void ShowLabelList (ostream &f);

	/**
	 * @brief Output trees as dendrograms.
	 *
	 * @param f output stream
	 */
    	virtual void ShowTrees (ostream &f);
	/**
	 * @brief Write a set of trees to an output stream
	 * @param f output stream 
	 * @param format file format to use (at present nexus only)
	 * @return true if successful
	 */
	virtual bool WriteTrees (ostream &f, const int format = 0, const char *endOfLine = "\n");

protected:
	/**
	 * The trees
	 *
	 */
	vector <T> Trees;
	/**
	 * The leaf labels stored as a vector so they can be accessed by an index value
	 *
	 */
	vector <string> LabelIndex;
};


//------------------------------------------------------------------------------
template <class T> int Profile<T>::GetIndexOfLabel (string s)
{
	return Labels[s];
}

//------------------------------------------------------------------------------
template <class T> void Profile<T>::MakeLabelList ()
{
	for (unsigned int i = 0; i < Trees.size(); i++)
	{
    	T t = Trees[i];
		t.MakeNodeList();
		for (int j = 0; j < t.GetNumLeaves(); j++)
		{
			string s = t[j]->GetLabel();
			
			if (Labels.find (s) == Labels.end ())
			{
				int index = Labels.size();
//				cout << "Labels.size() = " << Labels.size();
				Labels[s] = index;
				LabelIndex.push_back (s);
//				cout << "Labels[s] =" << Labels[s] << endl;
			}
		} 
	}
/*	cout << endl << "Number of labels = " << Labels.size() << endl;
	LabelMap::iterator it = Labels.begin();
	LabelMap::iterator end = Labels.end();
	while (it != end)
	{
		cout << (*it).first << " - " << (*it).second << endl;
		it++;
	}*/
}


//------------------------------------------------------------------------------
template <class T> void Profile<T>::ShowLabelList (ostream &f)
{
	f << endl << "Number of labels = " << Labels.size() << endl;
	LabelMap::iterator it = Labels.begin();
	LabelMap::iterator end = Labels.end();
	while (it != end)
	{
		f << (*it).first << " - " << (*it).second << endl;
		it++;
	}
	f << endl;
}

//------------------------------------------------------------------------------
template <class T> void Profile<T>::MakeLabelFreqList ()
{
//	cout << "MakeLabelListFreq" << endl;
	for (unsigned int i = 0; i < Trees.size(); i++)
	{
		T t = Trees[i];
		t.MakeNodeList();
		for (int j = 0; j < t.GetNumLeaves(); j++)
		{
			string s = t[j]->GetLabel();
			LabelMap::iterator there = LabelFreq.find (s);
			
			if (there == Labels.end ())
			{
				LabelFreq[s] = 1;
			}
			else
			{
				LabelFreq[s] += 1;
			}
		} 
	}
/*	cout << "Frequency of labels" << endl;
	LabelMap::iterator it = LabelFreq.begin();
	LabelMap::iterator end = LabelFreq.end();
	while (it != end)
	{
		cout << (*it).first << " - " << (*it).second << endl;
		it++;
	}*/
}

/**
 * @class MyNexus
 * Extends Nexus class to output progress to cout
 *
 */
class MyNexus : public Nexus
{
public:
	MyNexus () : Nexus() { isOK = true; };
	
#if (USE_VC2 || USE_WXWINDOWS)

	#if USE_VC2
		virtual void EnteringBlock( nxsstring blockName ) { }
		virtual void ExitingBlock( nxsstring blockName ) { };
		virtual void SkippingBlock( nxsstring blockName ) {  };
		virtual void SkippingDisabledBlock( nxsstring blockName ) { };
		virtual void ExecuteStarting() { };
		virtual void ExecuteStopping() { };
		virtual void OutputComment( nxsstring comment ) {};
	 	virtual void NexusError( nxsstring& msg, streampos pos, long line, long col )
		{
			char buf[256];
			sprintf (buf,"%s at line %d, column %d", msg.c_str(), line, col);
			Message (MSG_ERROR, "Error reading NEXUS file", buf);
			isOK = false;
		};
	#endif

	
	#if USE_WXWINDOWS
	#if 1
	   	 virtual void EnteringBlock( nxsstring blockName ) { }
	   	 virtual void ExitingBlock( nxsstring blockName ) { };
	   	 virtual void SkippingBlock( nxsstring blockName ) {  };
	   	 virtual void SkippingDisabledBlock( nxsstring blockName ) { };
		virtual void ExecuteStarting() { };
		virtual void ExecuteStopping() { };
	#else // debugging NEXUS reader
	   	 virtual void EnteringBlock( nxsstring blockName ) { wxLogMessage ("Entering %s block", blockName.c_str()); }
	   	 virtual void ExitingBlock( nxsstring blockName ) { wxLogMessage ("Exiting %s block", blockName.c_str()); };
	   	 virtual void SkippingBlock( nxsstring blockName ) { wxLogWarning ("Skipping %s block", blockName.c_str());  };
	   	 virtual void SkippingDisabledBlock( nxsstring blockName ) {  wxLogWarning ("Skipping disabled %s block", blockName.c_str());  };
		virtual void ExecuteStarting() { wxLogMessage ("Starting to execute NEXUS file"); };
		virtual void ExecuteStopping() { wxLogMessage ("Finished executing NEXUS file"); };
	#endif
		virtual void OutputComment( nxsstring comment ) { cout << comment << endl;};
	 	virtual void NexusError( nxsstring& msg, streampos pos, long line, long col )
		{
			wxLogError ("%s at line %d, column %d", msg.c_str(), line, col);
			isOK = false;
		};
	#endif

#else
	virtual void EnteringBlock( nxsstring blockName ) { cout << "   Entering " << blockName << " block..."; };
	virtual void ExitingBlock( nxsstring blockName ) { cout << "done" << endl; };
	virtual void SkippingBlock( nxsstring blockName ) { cout << "   (Skipping " << blockName << " block)" << endl; };
	virtual void SkippingDisabledBlock( nxsstring blockName ) { cout << "   (Skipping disabled " << blockName << " block)" << endl; };
	virtual void ExecuteStarting() { cout << "Starting to execute NEXUS file" << endl; };
	virtual void ExecuteStopping() { cout << "Finished executing NEXUS file" << endl; };
	virtual void OutputComment( nxsstring comment ) { cout << comment << endl;};
	virtual void NexusError( nxsstring& msg, streampos pos, long line, long col )
	{
   		cerr << "Error: " << msg << " line " << line << ", col " << col << endl;
		isOK = false;
	};
#endif
	bool GetIsOK () { return isOK; };

protected:
	bool isOK;
};

//------------------------------------------------------------------------------
template <class T> bool Profile<T>::ReadNEXUS (istream &f)
{
	bool result = false;
 //   cout << "Reading NEXUS" << endl;
	TaxaBlock* taxa;
	DataBlock *data;
	CharactersBlock *characters;
	AssumptionsBlock *assumptions;
	TreesBlock *trees;

	taxa = new TaxaBlock();
	assumptions = new AssumptionsBlock (*taxa);
	data = new DataBlock (*taxa, *assumptions);
	characters = new CharactersBlock (*taxa, *assumptions);
	trees = new TreesBlock (*taxa);

	MyNexus nexus;
	nexus.Add( taxa );
	nexus.Add( data );
	nexus.Add( characters );
	nexus.Add( trees );
    


    // Set to binary to handle Mac and Unix files
#ifndef __MWERKS__
  //      f.ios::setf (ios::binary);
#endif

	NexusToken token (f);
    

	try 
	{
    		nexus.Execute (token);
	}
	catch (XNexus x)
	{
		cout << x.msg << " (line " << x.line << ", column " << x.col << ")" << endl;
	}    	

	if (nexus.GetIsOK() && (trees->GetNumTrees() > 0))
	{

		// Display information about the trees
#if (USE_WXWINDOWS || USE_VC2)
#else
		trees->Report (cout);
		cout << endl;
#endif
		// Store the trees themselves
		int i = 0;
		int error = 0;
		while ((i < trees->GetNumTrees()) && (error == 0))
		{ 
			T t;
			std::string tstr;
			if (trees->HasTranslationTable())
				tstr = trees->GetTranslatedTreeDescription (i);
			else
				tstr = trees->GetTreeDescription (i);
			tstr += ";";
			error = t.Parse (tstr.c_str());
			if (error == 0)
			{
				t.SetName (trees->GetTreeName (i));
				t.SetRooted (trees->IsRootedTree (i));
				t.SetWeight (trees->GetTreeWeight (i));
				Trees.push_back (t);
			}
			else
			{
#if USE_WXWINDOWS
				wxLogError ( "Error in description of tree %d: %s", (i+1), t.GetErrorMsg().c_str());
#elif USE_VC2
				char buf[256];
				sprintf (buf, "Reading tree %d: %s", (i+1), t.GetErrorMsg().c_str());
				Message (MSG_ERROR, "Error in tree description", buf);
#else
				cerr << "Error in tree description " << (i + 1) << t.GetErrorMsg() << endl;
#endif
				return false;
			}           
			 i++;
		}
            //    cout << "DONE1" << endl;
		// Assign each label a unique index
		if (taxa->GetNumTaxonLabels() == 0)
		{
			// No taxa block in NEXUS file
			MakeLabelList ();
		}
		else
		{
			// Store the labels in the same order encountered in the
			// NEXUS file
			for (int i = 0; i < taxa->GetNumTaxonLabels (); i++)
			{
				Labels[taxa->GetTaxonLabel (i)] = i;
				LabelIndex.push_back (taxa->GetTaxonLabel (i));
			}
		}
           //     cout << "DONE2" << endl;
		result = true;
	}
  //      cout << "DONE3" << endl;
	return result;
}


//------------------------------------------------------------------------------
template <class T> bool Profile<T>::ReadTrees (istream &f)
{
	bool result = false;
  //  cout << "READING TREES" << endl;
	char ch = (char)f.peek ();
	if (ch == '#')
		result = ReadNEXUS (f);
	else if (strchr ("([", ch))
		result = ReadPHYLIP (f);
   //     cout << "finished reading routines" << endl;
	return result;
}

//------------------------------------------------------------------------------
template <class T> bool Profile<T>::ReadPHYLIP (istream &f)
{
 //   cout << "READING PHYLIP" << endl;
	Tokeniser p (f);
	PHYLIPReader tr (p);
	bool ok = true;
	while (ok)
	{
        //    cout << "LOOPY1" << endl;
		T t;
      //      cout << "LOOPY" << endl;
		try
		{
                  //  cout << "reading" << endl;
			ok = tr.Read (&t);
               //         cout << "read" << endl;
		}
		catch (XTokeniser x)
		{
#if USE_WXWINDOWS 
			wxLogError ("%s at line %d, column %d", x.msg.c_str(), x.line, x.col);           
#elif USE_VC2
			char buf[256];
			sprintf (buf, "%s at line %d, column %d", x.msg.c_str(), x.line, x.col);
			Message (MSG_ERROR, "Error reading tree file", buf);
#else
			cerr << x.msg << " (line " << x.line << ", column " << x.col << ")" << endl;
#endif
		 	return false;
		}

		if (ok)
                {

                 //   cout << "Tree onto stack" << endl;
			Trees.push_back (t);
                  //      cout << "Tree onto stack" << endl;
                }
             //   cout << "OK IS " << ok << endl;
	}
    //    cout << "DONE1" << endl;
	bool result = (Trees.size() > 0);
    //    cout << "DONE2" << endl;
	if (result)
	{
		// Build a list of labels in the profile, such that each label 
		// is assigned a unique index    	
		MakeLabelList ();
	}
    //    cout << "DONE3" << endl;
	return result;
}

//------------------------------------------------------------------------------
template <class T> void Profile<T>::ShowTrees (ostream &f)
{
	cout << "ShowTrees" << endl;
	for (unsigned int i = 0; i < Trees.size(); i++)
	{
		T t = Trees[i];
		t.Update ();
		t.Draw (f);
	}
}

//------------------------------------------------------------------------------
template <class T> bool Profile<T>::WriteTrees (ostream &f, const int format, const char *endOfLine)
{
	
	
	// Simple nexus tree file
	f << "#nexus" << endOfLine;
	f << endOfLine;
	f << "begin trees;";
		
	// Date the file
	f << " [Treefile written ";
	time_t timer = time(NULL);
	struct tm* tblock = localtime(&timer);
	char time_buf[64];
	strncpy (time_buf, asctime(tblock), sizeof (time_buf));
	char *q = strrchr (time_buf, '\n');
	if (q)
		*q = '\0';
	f << time_buf << "]" << endOfLine;

	for (unsigned int i = 0; i < Trees.size(); i++)
	{
		T t = Trees[i];
		f << "\ttree ";
		if (t.GetName() != "")
			f << NEXUSString (t.GetName());
		else
			f << "tree_" << (i+1);
		f << " = ";
		if (t.IsRooted())
			f << "[&R] ";
		else
			f << "[&U] ";
			
		// Tree
		NewickTreeWriter tw (&t);
		tw.SetStream (&f);
		tw.Write();	
		f << endOfLine;		
			
//		f << t << endOfLine;
	}
	f << "end;" << endOfLine;
	
	return true;
}

#if __BORLANDC__
	// Redefine __MINMAX_DEFINED so Windows header files compile
	#ifndef __MINMAX_DEFINED
    		#define __MINMAX_DEFINED
	#endif
#endif

#endif

