#include "nexusdefs.h"
#include "xnexus.h"
#include "nexustoken.h"
#include "nexus.h"
#include "taxablock.h"

/**
 * @class      TaxaBlock
 * @file       taxablock.h
 * @file       taxablock.cpp
 * @author     Paul O. Lewis
 * @copyright  Copyright © 1999. All Rights Reserved.
 * @variable   ntax [int:private] number of taxa (set from NTAX specification)
 * @variable   taxonLabels [LabelList:private] storage for list of taxon labels
 * @see        LabelList
 * @see        Nexus
 * @see        NexusBlock
 * @see        NexusReader
 * @see        NexusToken
 * @see        XNexus
 *
 * This class handles reading and storage for the Nexus block TAXA.
 * It overrides the member functions Read and Reset, which are abstract
 * virtual functions in the base class NexusBlock.  The taxon names are
 * stored in an array of strings (taxonLabels) that is accessible through
 * the member functions GetTaxonLabel, AddTaxonLabel, ChangeTaxonLabel,
 * and GetNumTaxonLabels.
 *
 * <P> Below is a table showing the correspondence between the elements of a
 * TAXA block and the variables and member functions that can be used
 * to access each piece of information stored.
 *
 * <p><table border=1>
 * <tr>
 *   <th> Nexus command
 *   <th> Nexus subcommand
 *   <th> Data Members
 *   <th> Member Functions
 * <tr>
 *   <td> DIMENSIONS
 *   <td> NTAX
 *   <td> int <a href="#ntax">ntax</a>
 *   <td> int <a href="#GetNumTaxonLabels">GetNumTaxonLabels()</a>
 * <tr>
 *   <td colspan=2 align=left> TAXLABELS
 *   <td> LabelList <a href="#taxonLabels">taxonLabels</a>
 *   <td> nxsstring <a href="#GetTaxonLabel">GetTaxonLabel( int i )</a>
 *        <br> int <a href="#FindTaxon">FindTaxon( nxsstring label )</a>
 *        <br> int <a href="#GetMaxTaxonLabelLength">GetMaxTaxonLabelLength()</a>
 * </table>
 */

/**
 * @constructor
 *
 * Default constructor. Initializes id to "TAXA" and ntax to 0.
 */
TaxaBlock::TaxaBlock() : ntax(0), NexusBlock()
{
   id = "TAXA";
}

/**
 * @destructor
 *
 * Flushes taxonLabels.
 */
TaxaBlock::~TaxaBlock()
{
	taxonLabels.erase( taxonLabels.begin(), taxonLabels.end() );
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
void TaxaBlock::Read( NexusToken& token )
{
   isEmpty = false;
	token.GetNextToken(); // this should be the semicolon after the block name
	if( !token.Equals(";") ) {
		errormsg = "Expecting ';' after TAXA block name, but found ";
      errormsg += token.GetToken();
      errormsg += " instead";
		throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
	}

	for(;;)
	{
		token.GetNextToken();
		
		if( token.Equals("DIMENSIONS") ) {
			token.GetNextToken(); // this should be the NTAX keyword
			if( !token.Equals("NTAX") ) {
				errormsg = "Expecting NTAX keyword, but found ";
            errormsg += token.GetToken();
            errormsg += " instead";
				throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
			}

			token.GetNextToken(); // this should be the equals sign
         if( !token.Equals("=") ) {
				errormsg = "Expecting '=', but found ";
            errormsg += token.GetToken();
            errormsg += " instead";
				throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
         }

			token.GetNextToken(); // this should be the number of taxa
         ntax = atoi( token.GetToken().c_str() );
         if( ntax <= 0 ) {
				errormsg = "NTAX should be greater than zero (";
            errormsg += token.GetToken();
            errormsg += " was specified)";
				throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
         }

			token.GetNextToken(); // this should be the terminating semicolon
         if( !token.Equals(";") ) {
				errormsg = "Expecting ';' to terminate DIMENSIONS command, but found ";
            errormsg += token.GetToken();
            errormsg += " instead";
				throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
         }
		}
		else if( token.Equals("TAXLABELS") ) {
      	if( ntax <= 0 ) {
				errormsg = "NTAX must be specified before TAXLABELS command";
				throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
         }

         for( int i = 0; i < ntax; i++ ) {
				token.GetNextToken();
            taxonLabels.push_back( token.GetToken() );
         }

			token.GetNextToken(); // this should be terminating semicolon
         if( !token.Equals(";") ) {
				errormsg = "Expecting ';' to terminate TAXLABELS command, but found ";
            errormsg += token.GetToken();
            errormsg += " instead";
				throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
         }
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
 * @method Report [void:public]
 * @param out [ostream&] the output stream to which to write the report
 *
 * This function outputs a brief report of the contents of this taxa block.
 * Overrides the abstract virtual function in the base class.
 */
void TaxaBlock::Report( ostream& out )
{
	out << endl;
	out << id << " block contains ";
	if( ntax == 0 ) {
		out << "no taxa" << endl;
	}
	else if( ntax == 1 )
		out << "one taxon" << endl;
	else
		out << ntax << " taxa" << endl;

	if( ntax == 0 ) return;

	for( int k = 0; k < ntax; k++ )
		out << '\t' << (k+1) << '\t' << taxonLabels[k] << endl;
}

/**
 * @method Reset [void:protected]
 *
 * Flushes taxonLabels and sets ntax to 0 in preparation for reading a
 * new TAXA block.
 */
void TaxaBlock::Reset()
{
   isEmpty = true;
	taxonLabels.erase( taxonLabels.begin(), taxonLabels.end() );
   ntax = 0;
}

/**
 * @method AddTaxonLabel [void:public]
 * @param s [nxsstring] the taxon label to add
 *
 * Adds taxon label s to end of list of taxon labels and increments
 * ntax by 1.
 */
void TaxaBlock::AddTaxonLabel( nxsstring s )
{
   isEmpty = false;
	taxonLabels.push_back(s);
   ntax++;
}

/**
 * @method ChangeTaxonLabel [void:public]
 * @param i [int] the taxon label number to change
 * @param s [nxsstring] the string used to replace label i
 *
 * Changes the label for taxon i to s.
 */
void TaxaBlock::ChangeTaxonLabel( int i, nxsstring s )
{
	assert( i < (int)taxonLabels.size() );
   taxonLabels[i] = s;
}

/**
 * @method GetMaxTaxonLabelLength [char*:public]
 *
 * Returns the length of the longest taxon label stored.  Useful for
 * formatting purposes in outputting the data matrix (i.e., you want the
 * left edge of the matrix to line up).
 */
int TaxaBlock::GetMaxTaxonLabelLength()
{
   assert( ntax == (int)taxonLabels.size() );
   int maxlen = 0;
   for( int i = 0; i < ntax; i++ ) {
      int thislen = taxonLabels[i].size();
      if( thislen > maxlen )
         maxlen = thislen;
   }
   return maxlen;
}

/**
 * @method GetTaxonLabel [char*:public]
 * @param i [int] the taxon label number to return
 *
 * Returns the label for taxon i.
 */
nxsstring TaxaBlock::GetTaxonLabel( int i )
{
	assert( i < (int)taxonLabels.size() );
   return taxonLabels[i];
}

/**
 * @method IsAlreadyDefined [bool:public]
 * @param s [nxsstring] the s to attempt to find in the taxonLabels list
 *
 * Calls IsAlreadyDefined function of taxonLabels, which returns 0
 * if a taxon label equal to s is already stored in taxonLabels.
 * Returns 0 if no taxon label equal to s can be found in the
 * taxonLabels list.
 */
bool TaxaBlock::IsAlreadyDefined( nxsstring s )
{
   LabelList::const_iterator iter = find( taxonLabels.begin(), taxonLabels.end(), s );
   int taxonLabelFound = ( iter != taxonLabels.end() );
   return taxonLabelFound;
}

/**
 * @method FindTaxon [int:public]
 * @param s [nxsstring] the string to attempt to find in the taxonLabels list
 * @throws TaxaBlock::nosuchtaxon
 *
 * Returns index of taxon named s in taxonLabels list.  If taxon named
 * s cannot be found, or if there are no labels currently stored in
 * the taxonLabels list, throws nosuchtaxon exception.
 */
int TaxaBlock::FindTaxon( nxsstring s )
{
   int k = 0;
   LabelList::const_iterator i;
   for( i = taxonLabels.begin(); i != taxonLabels.end(); ++i ) {
      if( *i == s ) break;
      k++;
   }

   if( i == taxonLabels.end() )
      throw TaxaBlock::nosuchtaxon();

   return k;
}

/**
 * @method GetNumTaxonLabels [int:public]
 *
 * Returns number of taxon labels currently stored.
 */
int TaxaBlock::GetNumTaxonLabels()
{
	return taxonLabels.size();
}

/**
 * @method SetNtax [void:private]
 * @param n [int] the number of taxa
 *
 * Sets ntax to n.
 */
void TaxaBlock::SetNtax( int n )
{
   ntax = n;
}

