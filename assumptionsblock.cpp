#include <iostream.h>
#include <stdio.h>

#include "nexusdefs.h"
#include "xnexus.h"
#include "nexustoken.h"
#include "nexus.h"
#include "setreader.h"
#include "taxablock.h"
#include "discretedatum.h"
#include "discretematrix.h"
#include "charactersblock.h"
#include "assumptionsblock.h"

/**
 * @class      AssumptionsBlock
 * @file       assumptionsblock.h
 * @file       assumptionsblock.cpp
 * @author     Paul O. Lewis
 * @copyright  Copyright © 1999. All Rights Reserved.
 * @variable   taxa [TaxaBlock&:private] reference to the TaxaBlock object
 * @variable   charBlockPtr [CharactersBlock*:private] pointer to the CharactersBlock-derived object to be notified in the event of exset changes
 * @variable   charsets [IntSetMap:protected] the variable storing charsets
 * @variable   taxsets [IntSetMap:protected] the variable storing taxsets
 * @variable   exsets [IntSetMap:protected] the variable storing exsets
 * @see        CharactersBlock
 * @see        Nexus
 * @see        NexusBlock
 * @see        NexusToken
 * @see        SetReader
 * @see        TaxaBlock
 * @see        XNexus
 *
 * This class handles reading and storage for the Nexus block ASSUMPTIONS.
 * It overrides the member functions Read and Reset, which are abstract
 * virtual functions in the base class NexusBlock.
 */

/**
 * @constructor
 *
 * Performs the following initializations:
 * <table>
 * <tr><th align="left">Variable <th> <th align="left"> Initial Value
 * <tr><td> id             <td>= <td> "ASSUMPTIONS"
 * </table>
 */
AssumptionsBlock::AssumptionsBlock( TaxaBlock& t )
	: taxa(t), charBlockPtr(NULL)
{
	id = "ASSUMPTIONS";
}

/**
 * @destructor
 *
 * Nothing needs to be done.
 */
AssumptionsBlock::~AssumptionsBlock()
{
}

/**
 * @method GetNumCharSets [int:public]
 *
 * Returns the number of character sets stored.
 */
int AssumptionsBlock::GetNumCharSets()
{
	return (int)charsets.size();
}

/**
 * @method GetCharSetNames [void:public]
 * @param names [LabelList&] the vector in which to store the names
 *
 * Erases names, then fills names with the names of all stored
 * character sets.
 */
void AssumptionsBlock::GetCharSetNames( LabelList& names )
{
	names.erase( names.begin(), names.end() );
   IntSetMap::const_iterator i;
   for( i = charsets.begin(); i != charsets.end(); i++ )
		names.push_back( (*i).first );
}

/**
 * @method GetCharSet [IntSet&:public]
 * @param nm [nxsstring] the name of the character set to return
 *
 * Returns reference to character set having name nm.
 */
IntSet& AssumptionsBlock::GetCharSet( nxsstring nm )
{
	return charsets[nm];
}

/**
 * @method GetDefCharSetName [nxsstring:public]
 *
 * Returns name of default character set.  If returned
 * string has zero length, then no default character set
 * was defined in the data set.
 */
nxsstring AssumptionsBlock::GetDefCharSetName()
{
	return def_charset;
}

/**
 * @method GetNumTaxSets [int:public]
 *
 * Returns the number of taxon sets stored.
 */
int AssumptionsBlock::GetNumTaxSets()
{
	return (int)taxsets.size();
}

/**
 * @method GetTaxSetNames [void:public]
 * @param names [LabelList&] the vector in which to store the names
 *
 * Erases names, then fills names with the names of all stored
 * taxon sets.
 */
void AssumptionsBlock::GetTaxSetNames( LabelList& names )
{
	names.erase( names.begin(), names.end() );
   IntSetMap::const_iterator i;
   for( i = taxsets.begin(); i != taxsets.end(); i++ )
		names.push_back( (*i).first );
}

/**
 * @method GetTaxSet [IntSet&:public]
 * @param nm [nxsstring] the name of the taxon set to return
 *
 * Returns reference to taxon set having name nm.
 */
IntSet& AssumptionsBlock::GetTaxSet( nxsstring nm )
{
	return taxsets[nm];
}

/**
 * @method GetDefTaxSetName [nxsstring:public]
 *
 * Returns name of default taxon set.  If returned
 * string has zero length, then no default taxon set
 * was defined in the data set.
 */
nxsstring AssumptionsBlock::GetDefTaxSetName()
{
	return def_taxset;
}

/**
 * @method GetNumExSets [int:public]
 *
 * Returns the number of exclusion sets stored.
 */
int AssumptionsBlock::GetNumExSets()
{
	return (int)exsets.size();
}

/**
 * @method GetExSetNames [void:public]
 * @param names [LabelList&] the vector in which to store the names
 *
 * Erases names, then fills names with the names of all stored
 * exclusion sets.
 */
void  AssumptionsBlock::GetExSetNames( LabelList& names )
{
	names.erase( names.begin(), names.end() );
   IntSetMap::const_iterator i;
   for( i = exsets.begin(); i != exsets.end(); i++ )
		names.push_back( (*i).first );
}

/**
 * @method GetExSet [IntSet&:public]
 * @param nm [nxsstring] the name of the exclusion set to return
 *
 * Returns reference to exclusion set having name nm.
 */
IntSet& AssumptionsBlock::GetExSet( nxsstring nm )
{
	return exsets[nm];
}

/**
 * @method GetDefExSetName [nxsstring:public]
 *
 * Returns name of default exclusion set.  If returned
 * string has zero length, then no default exclusion set
 * was defined in the data set.
 */
nxsstring AssumptionsBlock::GetDefExSetName()
{
	return def_exset;
}

/**
 * @method ApplyExSet [void:public]
 * @param nm [nxsstring] the name of the exclusion set to apply
 *
 * Applies exclusion set having name nm by calling the
 * ApplyExset method of the CharactersBlock or
 * CharactersBlock-derived object stored in the charBlockPtr
 * pointer (which will be whichever block last called
 * the AssumptionsBlock::SetCallback method).
 */
void AssumptionsBlock::ApplyExSet( nxsstring nm )
{
	assert( charBlockPtr != NULL );
   charBlockPtr->ApplyExset( exsets[nm] );
}

/**
 * @method HandleCharset [void:protected]
 * @param token [NexusToken&] the token used to read from in
 * @throws XNexus
 *
 * Reads and stores information contained in the command
 * CHARSET within an ASSUMPTIONS block.
 */
void AssumptionsBlock::HandleCharset( NexusToken& token )
{
   bool asterisked = false;

	// Next token should be either an asterisk or the name of a charset
	token.GetNextToken();

	if( token.Equals("*") ) {
      asterisked = true;
   	token.GetNextToken();
	}

   // Token now stored should be the name of a charset
   nxsstring charset_name = token.GetToken();

   // Now grab the equals sign
 	token.GetNextToken();
   if( !token.Equals("=") ) {
		errormsg = "Expecting '=' in CHARSET definition but found ";
      errormsg += token.GetToken();
      errormsg += " instead";
		throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
   }

   assert( charBlockPtr != NULL );
   CharactersBlock& charBlock = *charBlockPtr;
   IntSet s;
   int totalChars = charBlock.GetNCharTotal();
   SetReader( token, totalChars, s, charBlock, SetReader::charset ).Run();

   charsets[charset_name] = s;

   if( asterisked )
   	def_charset = charset_name;
}

/**
 * @method HandleEndblock [void:protected]
 * @param token [NexusToken&] the token used to read from in
 * @throws XNexus
 *
 * Called when the END or ENDBLOCK command needs to be parsed
 * from within the ASSUMPTIONS block.  Basically just checks to make
 * sure the next token in  the data file is a semicolon.
 */
void AssumptionsBlock::HandleEndblock( NexusToken& token )
{
	// get the semicolon following END or ENDBLOCK token
   //
	token.GetNextToken();

	if( !token.Equals(";") ) {
   	errormsg = "Expecting ';' to terminate the END or ENDBLOCK command, but found ";
      errormsg += token.GetToken();
      errormsg += " instead";
		throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
	}
}

/**
 * @method HandleExset [void:protected]
 * @param token [NexusToken&] the token used to read from in
 * @throws XNexus
 *
 * Reads and stores information contained in the command
 * EXSET within an ASSUMPTIONS block.  If EXSET keyword is
 * followed by an asterisk, last read CharactersBlock or
 * CharactersBlock-derived object is notified of the
 * characters to be excluded (its ApplyExset function
 * is called).
 */
void AssumptionsBlock::HandleExset( NexusToken& token )
{
   bool asterisked = false;

	// Next token should be either an asterisk or the name of an exset
	token.GetNextToken();

	if( token.Equals("*") ) {
      asterisked = true;
   	token.GetNextToken();
	}

   // Token now stored should be the name of an exset
   nxsstring exset_name = token.GetToken();

   // Now grab the equals sign
 	token.GetNextToken();
   if( !token.Equals("=") ) {
		errormsg = "Expecting '=' in EXSET definition but found ";
      errormsg += token.GetToken();
      errormsg += " instead";
		throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
   }

   assert( charBlockPtr != NULL );
   CharactersBlock& charBlock = *charBlockPtr;
   IntSet s;
   int totalChars = charBlock.GetNCharTotal();
   SetReader( token, totalChars, s, charBlock, SetReader::charset ).Run();

   exsets[exset_name] = s;

   if( asterisked ) {
   	def_exset = exset_name;
      charBlock.ApplyExset(s);
   }
}

/**
 * @method HandleTaxset [void:protected]
 * @param token [NexusToken&] the token used to read from in
 * @throws XNexus
 *
 * ?
 */
void AssumptionsBlock::HandleTaxset( NexusToken& token )
{
   bool asterisked = false;

	// Next token should be either an asterisk or the name of a taxset
	token.GetNextToken();

	if( token.Equals("*") ) {
      asterisked = true;
   	token.GetNextToken();
	}

   // Token now stored should be the name of a taxset
   nxsstring taxset_name = token.GetToken();

   // Now grab the equals sign
 	token.GetNextToken();
   if( !token.Equals("=") ) {
		errormsg = "Expecting '=' in TAXSET definition but found ";
      errormsg += token.GetToken();
      errormsg += " instead";
		throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
   }

   IntSet s;
   int totalTaxa = taxa.GetNumTaxonLabels();
   SetReader( token, totalTaxa, s, *this, SetReader::taxset ).Run();

   taxsets[taxset_name] = s;

   if( asterisked )
   	def_taxset = taxset_name;
}

/**
 * @method Read [void:protected]
 * @param token [NexusToken&] the token used to read from in
 * @throws XNexus
 *
 * This function provides the ability to read everything following
 * the block name (which is read by the Nexus object) to the end or
 * endblock statement. Characters are read from the input stream
 * in. Overrides the pure virtual function in the base class.
 */
void AssumptionsBlock::Read( NexusToken& token )
{
   isEmpty = false;

   // this should be the semicolon after the block name
   //
	token.GetNextToken();
	if( !token.Equals(";") ) {
		errormsg = "Expecting ';' after ";
      errormsg += id;
      errormsg += " block name, but found ";
      errormsg += token.GetToken();
      errormsg += " instead";
		throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
	}

	for(;;)
	{
		token.GetNextToken();

		if( token.Equals("EXSET") ) {
			HandleExset( token );
		}
		else if( token.Equals("TAXSET") ) {
			HandleTaxset( token );
		}
		else if( token.Equals("CHARSET") ) {
			HandleCharset( token );
		}
		else if( token.Equals("END") ) {
			HandleEndblock( token );
			break;
		}
		else if( token.Equals("ENDBLOCK") ) {
			HandleEndblock( token );
			break;
		}
		else
      {
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
 * Prepares for reading a new ASSUMPTIONS block.
 * Overrides the pure virtual function in the base class.
 */
void AssumptionsBlock::Reset()
{
   isEmpty = true;
   exsets.erase( exsets.begin(), exsets.end() );
   taxsets.erase( taxsets.begin(), taxsets.end() );
   charsets.erase( charsets.begin(), charsets.end() );
   def_taxset = "";
   def_charset = "";
   def_exset = "";
}

/**
 * @method Report [void:public]
 * @param out [ostream&] the output stream to which to write the report
 *
 * This function outputs a brief report of the contents of this ASSUMPTIONS block.
 * Overrides the pure virtual function in the base class.
 */
void AssumptionsBlock::Report( ostream& out )
{
	out << endl;
	out << id << " block contains the following:" << endl;

   if( charsets.empty() )
   	out << "  No character sets were defined" << endl;
   else {
      IntSetMap::const_iterator charsets_iter = charsets.begin();
      if( charsets.size() == 1 ) {
         out << "  1 character set defined:" << endl;
         out << "    " << (*charsets_iter).first << endl;
      }
      else {
         out << "  " << charsets.size() << " character sets defined:" << endl;
	      for( ; charsets_iter != charsets.end(); charsets_iter++ ) {
         	nxsstring nm = (*charsets_iter).first;
   	      out << "    " << nm;
            if( nm == def_charset )
            	out << " (default)";
          	out << endl;
         }
      }
   }

   if( taxsets.empty() )
   	out << "  No taxon sets were defined" << endl;
   else {
      IntSetMap::const_iterator taxsets_iter = taxsets.begin();
      if( taxsets.size() == 1 ) {
         out << "  1 taxon set defined:" << endl;
 	      out << "    " << (*taxsets_iter).first << endl;
      }
      else {
         out << "  " << taxsets.size() << " taxon sets defined:" << endl;
	      for( ; taxsets_iter != taxsets.end(); taxsets_iter++ ) {
         	nxsstring nm = (*taxsets_iter).first;
   	      out << "    " << nm;
            if( nm == def_taxset )
            	out << " (default)";
            out << endl;
         }
	   }
   }

   if( exsets.empty() )
   	out << "  No exclusion sets were defined" << endl;
   else {
      IntSetMap::const_iterator exsets_iter = exsets.begin();
      if( exsets.size() == 1 ) {
         out << "  1 exclusion set defined:" << endl;
 	      out << "    " << (*exsets_iter).first << endl;
      }
      else {
         out << "  " << exsets.size() << " exclusion sets defined:" << endl;
	      for( ; exsets_iter != exsets.end(); exsets_iter++ ) {
         	nxsstring nm = (*exsets_iter).first;
   	      out << "    " << nm;
            if( nm == def_exset )
            	out << " (default)";
            out << endl;
      	}
      }
   }

   out << endl;
}

/**
 * @method SetCallback [void:public]
 * @param p [CharactersBlock*] the object to be called in the event of a change in character status
 *
 * A CHARACTERS, DATA, or ALLELES block can call this function to specify that
 * it is to receive notification when the current taxon or character set
 * changes (e.g., an "EXSET *" command is read or a program requests that
 * one of the predefined taxon sets, character sets, or exsets be applied).
 * Normally, a CharactersBlock-derived object calls this function upon
 * entering its MATRIX command, since when that happens it becomes the
 * primary data-containing block.
 */
void AssumptionsBlock::SetCallback( CharactersBlock* p )
{
	charBlockPtr = p;
}

/**
 * @method TaxonLabelToNumber [void:protected]
 * @param s [nxsstring&] the taxon label to convert
 *
 * Converts a taxon label to a number corresponding to
 * the taxon's position within the list maintained by
 * the TaxaBlock object.  This method overrides the
 * virtual function of the same name in the NexusBlock
 * base class.  If s is not a valid taxon label, returns
 * the value 0.
 */
int AssumptionsBlock::TaxonLabelToNumber( nxsstring s )
{
   int i;
   try {
      i = 1 + taxa.FindTaxon(s);
   }
   catch( TaxaBlock::nosuchtaxon ) {
      i = 0;
   }
   return i;
}


