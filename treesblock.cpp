#include "nexusdefs.h"
#include "xnexus.h"
#include "nexustoken.h"
#include "nexus.h"
#include "taxablock.h"
#include "treesblock.h"

/**
 * @class      TreesBlock
 * @file       taxablock.h
 * @file       taxablock.cpp
 * @author     Paul O. Lewis
 * @copyright  Copyright © 1999. All Rights Reserved.
 * @variable   translateList [AssocList:private] storage for translation table (if any)
 * @variable   treeName [LabelList:private] storage for tree names
 * @variable   treeDescription [LabelList:private] storage for tree descriptions
 * @variable   rooted [BoolVect:private] stores information about rooting for each tree
 * @variable   ntrees [int:private] number of trees stored
 * @see        Nexus
 * @see        NexusBlock
 * @see        NexusReader
 * @see        NexusToken
 * @see        TaxaBlock
 * @see        XNexus
 *
 * This class handles reading and storage for the Nexus block TREES.
 * It overrides the member functions Read and Reset, which are abstract
 * virtual functions in the base class NexusBlock.  The translation table
 * (if one is supplied) is stored in the AssocList translateList.  The
 * tree names are stored in the LabelList treeName and the tree descriptions
 * in the LabelList treeDescription. Information about rooting of trees
 * is stored in the BoolVect rooted.
 *
 * <P> Below is a table showing the correspondence between the elements of a
 * TREES block and the variables and member functions that can be used
 * to access each piece of information stored.
 *
 * <p><table border=1>
 * <tr>
 *   <th> Nexus command
 *   <th> Data Members
 *   <th> Member Functions
 * <tr>
 *   <td> TRANSLATE
 *   <td> AssocList <a href="#translateList">translateList</a>
 *   <td> No access functions defined
 * <tr>
 *   <td> TREE
 *   <td> LabelList <a href="#treeName">treeName</a>
 *        <br> BoolVect <a href="#rooted">rooted</a>
 *   <td> nxsstring <a href="#GetTreeName">GetTreeName()</a>
 *        <br> nxsstring <a href="#GetTreeDescription">GetTreeDescription()</a>
 *        <br> int <a href="#GetNumTrees">GetNumTrees()</a>
 *        <br> int <a href="#GetNumDefaultTree">GetNumDefaultTree()</a>
 *        <br> int <a href="#IsDefaultTree">IsDefaultTree()</a>
 *        <br> int <a href="#IsRootedTree">IsRootedTree()</a>
 * </table>
 */

/**
 * @constructor
 *
 * Initializes id to "TREES" and ntrees and defaultTrees to 0.
 */
TreesBlock::TreesBlock( TaxaBlock& tb ) : ntrees(0), defaultTree(0), taxa(tb), NexusBlock()
{
   id = "TREES";
}

/**
 * @destructor
 *
 * Flushes translateList, rooted, and treeList.
 */
TreesBlock::~TreesBlock()
{
	translateList.erase( translateList.begin(), translateList.end() );
	rooted.erase( rooted.begin(), rooted.end() );
	treeName.erase( treeName.begin(), treeName.end() );
	treeDescription.erase( treeDescription.begin(), treeDescription.end() );
}

/**
 * @method Read [void:protected]
 * @param token [NexusToken&] the token used to read from in
 * @throws XNexus
 *
 * This function provides the ability to read everything following the block name
 * (which is read by the Nexus object) to the end or endblock statement.
 * Characters are read from the input stream in. Overrides the
 * abstract virtual function in the base class.
 */
void TreesBlock::Read( NexusToken& token )
{
   isEmpty = false;
	token.GetNextToken(); // this should be the semicolon after the block name
	if( !token.Equals(";") ) {
      errormsg = "Expecting ';' after TREES block name, but found ";
      errormsg += token.GetToken();
      errormsg += " instead";
		throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
   }

	for(;;)
	{
		token.GetNextToken();

		if( token.Equals("TRANSLATE") ) {

         int numEntries = taxa.GetNumTaxonLabels();


         // rdmp

         if (numEntries == 0)
         {
         	// Stand alone TREES block
            do {
            	// get the key
            	token.GetNextToken();
            	nxsstring skey = token.GetToken();

            	// get the value
            	token.GetNextToken();
            	nxsstring sval = token.GetToken();

            	// add the Association object to the translate list
            	translateList[ skey ] = sval;

	            token.GetNextToken();
                if (!token.Equals (";") && !token.Equals (","))
                {
                    errormsg = "Expecting ',' or ';' in TRANSLATE command, but found ";
                    errormsg +=token.GetToken();
                    errormsg += " instead.";
                    throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
				}

            } while (!token.Equals (";"));
         }
         else
         {



         for( int k = 0; k < numEntries; k++ ) {
            // create the Association

            // get the key
            token.GetNextToken();
            nxsstring skey = token.GetToken();

            // get the value
            token.GetNextToken();
            nxsstring sval = token.GetToken();

            // add the Association object to the translate list
            translateList[ skey ] = sval;

            // this should be a comma, unless we are at the last pair, in
            // which case it should be a semicolon
            token.GetNextToken();
            if( k < numEntries-1 )
            {
               if( !token.Equals(",") ) {
						errormsg = "Expecting ',' to terminate each number/name pair in TRANSLATE command, but found ";
						errormsg +=token.GetToken();
						errormsg += " instead\nPerhaps there are fewer taxa in the tree file than in the stored data.";
                  throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
               }
            }
            else
            {
               if( !token.Equals(";") ) {
						errormsg = "Expecting ';' to terminate the TRANSLATE command, but found ";
						errormsg +=token.GetToken();
						errormsg += " instead";
                  throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
               }
            }

         }

         } // rdmp1c

		}
		else if( token.Equals("TREE") ) {

			// this should be either an asterisk or a tree name
			token.GetNextToken();
			if( token.Equals("*") ) {
				defaultTree = ntrees; // ntrees is not incremented until entire tree command has been read

				// this should be tree name
				token.GetNextToken();
			}

         // save the tree name as the key
         nxsstring skey = token.GetToken();

         // this should be an equals sign
         token.GetNextToken();
         if( !token.Equals("=") ) {
				errormsg = "Expecting '=' after tree name in TREE command, but found ";
            errormsg += token.GetToken();
            errormsg += " instead";
				throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
         }

         // this should be either a tree description or a command comment specifying
         // whether this tree is to be rooted ([&R]) or unrooted ([&U]).
         token.SetLabileFlagBit( NexusToken::saveCommandComments );
         token.SetLabileFlagBit( NexusToken::parentheticalToken );
         token.GetNextToken();

	// Paul Lewis' original code just looked for the [&R] command comment, but we want the weight as well
	nxsstring s = token.GetToken();
 	if( s.size() < 2 ) {
		errormsg = "Expecting command comment or tree description in TREE command, but found ";
		errormsg += token.GetToken();
 		errormsg += " instead";
		throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
	}
	
	while (s[0] == '&')
	{
		// command comment found
		if( s[1] == 'R' || s[1] == 'r' )
			rooted.push_back(true);
		else if( s[1] == 'U' || s[1] == 'u' )
			rooted.push_back(false);
                else if (s[1] == 'W' || s[1] == 'u') // rdmp
                {
			// weight of tree
			double weight = -1.0;

			unsigned int pos = s.find ("/", 2);
			nxsstring num;
			if (pos > 0 && pos < s.length())
			{
				// fractional weighting
 				float numerator, denominator;

				num.assign (s, 2, s.size() - pos);
				numerator = atof( num.c_str() );
				num.assign (s, pos + 1, s.size() - pos);
				denominator = atof( num.c_str() );

				weight = numerator / denominator;

			}
			else
			{
				num.assign (s, 2, s.size() - 2);
				weight = atof( num.c_str() );
			}

			treeWeight.push_back (weight);

		}
            	else {
			errormsg = "[";
			errormsg += token.GetToken();
			errormsg += "] is not a valid command comment in a TREE command";
			throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
		}
		
		// Get next token
		token.SetLabileFlagBit( NexusToken::saveCommandComments );
		token.SetLabileFlagBit( NexusToken::parentheticalToken );
		token.GetNextToken();
 		if( s.size() < 2 ) 
		{
			errormsg = "Expecting command comment or tree description in TREE command, but found ";
			errormsg += token.GetToken();
 			errormsg += " instead";
			throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
		}
		s = token.GetToken(); 
	}
	
	// Tree description
	nxsstring sval = token.GetToken();

         // this should be a semicolon
         token.GetNextToken();
         if( !token.Equals(";") ) {
				errormsg = "Expecting ';' to terminate the TREE command, but found ";
            errormsg += token.GetToken();
            errormsg += " instead";
   			throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
         }

         ntrees++;
         treeName.push_back( skey );
         treeDescription.push_back( sval );
         if( rooted.size() < ntrees )
            rooted.push_back(false);

         // rdmpc
         if( treeWeight.size() < ntrees )
            treeWeight.push_back(1.0);

		}
		else if( token.Equals("END") ) {
			// get the semicolon following END
			token.GetNextToken();
         if( !token.Equals(";") ) {
				errormsg = "Expecting ';' to terminate the END command, but found ";
            errormsg += token.GetToken();
            errormsg += " instead";
				throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
         }
			break;
		}
		else if( token.Equals("ENDBLOCK") ) {
			// get the semicolon following ENDBLOCK
			token.GetNextToken();
         if( !token.Equals(";") ) {
				errormsg = "Expecting ';' to terminate the ENDBLOCK command, but found ";
            errormsg += token.GetToken();
            errormsg += " instead";
				throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
         }
			break;
		}
		else {
      	SkippingCommand( token.GetToken() );
         do {
            token.GetNextToken();
         } while( !token.AtEOF() && !token.Equals(";") );
         if( token.AtEOF() ) {
				errormsg = "Unexpected end of file encountered";
				throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
         }
		}
	}
}

/**
 * @method Reset [void:protected]
 *
 * Flushes treeList, translateList and rooted, and sets ntrees to 0
 * in preparation for reading a new TREES block.
 */
void TreesBlock::Reset()
{
   isEmpty = true;
	treeName.erase( treeName.begin(), treeName.end() );
	treeDescription.erase( treeDescription.begin(), treeDescription.end() );
	translateList.erase( translateList.begin(), translateList.end() );
	rooted.erase( rooted.begin(), rooted.end() );
	ntrees = 0;
}

/**
 * @method GetNumDefaultTree [int:public]
 *
 * Returns the 0-offset index of the default tree, which will be 0 if there is
 * only one tree stored or no trees stored.  If more than one tree is stored,
 * the default tree will be the one specifically indicated by the user
 * (using an asterisk in the data file) to be the default tree (or 0 if the
 * user failed to specify.
 */
int TreesBlock::GetNumDefaultTree()
{
	return defaultTree;
}

/**
 * @method GetNumTrees [int:public]
 *
 * Returns the number of trees stored in this TreesBlock object.
 */
int TreesBlock::GetNumTrees()
{
	return ntrees;
}

/**
 * @method GetTreeName [char*:public]
 * @param i [int] the index of the tree for which the name is to be returned
 *
 * Returns the name of the tree stored at position i in treeList.  Assumes
 * that i will be in the range 0...ntrees-1.
 */
nxsstring TreesBlock::GetTreeName( int i )
{
	assert( i >= 0 );
	assert( i < ntrees );
   return treeName[i];
//trash	return treeList.GetKey(i);
}

static nxsstring& blanks_to_underscores( nxsstring& s )
{
   int len = s.length();
   for( int k = 0; k < len; k++ ) {
      if( s[k] == ' ' )
         s[k] = '_';
   }
   return s;
}

/**
 * @method GetTreeWeight [double:public]
 * @param i [int] the index of the tree for which the weight is to be returned
 *
 * Returns the weight of the tree stored at position i in treeList.  Assumes
 * that i will be in the range 0...ntrees-1.
 */
double TreesBlock::GetTreeWeight ( int i)
{
	assert( i >= 0 );
	assert( i < ntrees );
   return treeWeight[i];
}



/**
 * @method GetTranslatedTreeDescription [nxsstring:public]
 * @param i [int] the index of the tree for which the description is to be returned
 *
 * Returns the description of the tree stored at position i in treeList.  Assumes
 * that i will be in the range 0...ntrees-1.  Node numbers will be translated to
 * names in the resulting tree description.  Use GetTreeDescription if translation
 * is not desired.  When translating, blank spaces in names are converted to
 * underscores.
 */
nxsstring TreesBlock::GetTranslatedTreeDescription( int i )
{
	assert( i >= 0 );
	assert( i < ntrees );
   nxsstring s = treeDescription[i];
   nxsstring x;
   x += s[0];
   int slen = s.size();
   assert( slen > 1 );
   for( int k = 1; k < slen; k++ )
   {
      char prev = s[k-1];
      char curr = s[k];
      if( isdigit(curr) && ( prev == '(' || prev == ',' ) ) {
         nxsstring ns;
         ns += curr;
         for(;;) {
            curr = s[k+1];
            prev = s[k++];
            if( isdigit(curr) )
               ns += curr;
            else {
               --k;
               break;
            }
         }
         nxsstring nss = translateList[ns];
	// rdmp
	// Hack: surround name in single quotes so that my parsing code
	// will work. For example, we need to ensure that a name like 'a-b'
	// is parsed correctly. Without single quotes the hyphen will
	// generate a syntax error.
         x += "'";
	x += blanks_to_underscores( nss );
	x += "'";
      }
      else
         x += curr;
   }
   return x;
}

/**
 * @method GetTreeDescription [nxsstring:public]
 * @param i [int] the index of the tree for which the description is to be returned
 *
 * Returns the description of the tree stored at position i in treeList.  Assumes
 * that i will be in the range 0...ntrees-1.
 */
nxsstring TreesBlock::GetTreeDescription( int i )
{
	assert( i >= 0 );
	assert( i < ntrees );
   return treeDescription[i];
//trash	return treeList.GetValue(i);
}

/**
 * @method IsDefaultTree [int:public]
 * @param i [int] the index of the tree in question
 *
 * Returns true if the ith tree (0-offset) is the default tree, 0 otherwise.
 * Assumes that i will be in the range 0...ntrees-1.
 */
int TreesBlock::IsDefaultTree( int i )
{
	assert( i >= 0 );
	assert( i < ntrees );
	if( i == GetNumDefaultTree() )
		return 1;
	else
		return 0;
}

/**
 * @method IsRootedTree [int:public]
 * @param i [int] the index of the tree in question
 *
 * Returns true if the ith tree (0-offset) is rooted, 0 otherwise.
 * Assumes that i will be in the range 0...ntrees-1.
 */
int TreesBlock::IsRootedTree( int i )
{
	assert( i >= 0 );
	assert( i < ntrees );
	return (int)rooted[i];
}

/**
 * @method Report [void:public]
 * @param out [ostream&] the output stream to which to write the report
 *
 * This function outputs a brief report of the contents of this taxa block.
 * Overrides the abstract virtual function in the base class.
 */
void TreesBlock::Report( ostream& out )
{
	out << endl;
	out << id << " block contains ";
	if( ntrees == 0 ) {
		out << "no trees" << endl;
	}
	else if( ntrees == 1 )
		out << "one tree" << endl;
	else
		out << ntrees << " trees" << endl;

	if( ntrees == 0 ) return;

	for( int k = 0; k < ntrees; k++ ) {
		out << '\t' << (k+1) << '\t' << treeName[k];
		out << "\t(";
		if( rooted[k] )
			out << "rooted";
		else
			out << "unrooted";
        out << ", weight = " << treeWeight[k]; // rdmp
		if( defaultTree == k )
			out << ",default tree)" << endl;
		else
			out << ')' << endl;
	}
}


