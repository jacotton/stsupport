#include "nexusdefs.h"
#include "xnexus.h"
#include "nexustoken.h"
#include "nexus.h"
#include "setreader.h"

/**
 * @class      SetReader
 * @file       setreader.h
 * @file       setreader.cpp
 * @author     Paul O. Lewis
 * @copyright  Copyright © 1999. All Rights Reserved.
 * @variable   block [NexusBlock&] the NexusBlock used for looking up labels
 * @variable   max [int] maximum number of elements in the set
 * @variable   settype [int] the type of set being read (see enum)
 * @variable   nxsset [IntSet&] reference to the set being read
 * @variable   token [NexusToken&] the token object to use in reading the file
 * @see        NexusReader
 * @see        NexusToken
 * @see        XNexus
 *
 * A class for reading Nexus set objects and storing them in a set of int values.
 * The IntSet nxsset will be flushed if it is not empty, and nxsset will be built
 * up as the set is read, with each element in the list storing a member of
 * the set (ranges are stored as individual elements).
 *
 * <p>This class handles set descriptions of the following form:
 * <pre>
 * 4-7 15 20-.\3;
 * </pre>
 * The above set includes all numbers from 4 to 100 (inclusive) as well as
 * 105 and every third number from 110 to max.  If max were 30, the array
 * stored would look like this:
 * <table border=1>
 * <tr> <td>4</td> <td>5</td> <td>6</td> <td>7</td> <td>15</td> <td>20</td>
 * <td>23</td> <td>26</td> <td>29</td>
 * </table>
 */

/**
 * @enumeration
 * @enumitem  generic [1] means expect a generic set (say, characters weights)
 * @enumitem  charset [2] means expect a character set
 * @enumitem  taxsert [3] means expect a taxon set
 *
 * For use with the variable settype.  Default is 1 (generic).
 */

/**
 * @constructor
 *
 * Initializes token to t and nxsset to iset, then
 * erases nxsset (if it is nonempty).
 */
SetReader::SetReader( NexusToken& t, int maxValue, IntSet& iset
	, NexusBlock& nxsblk, int type )
   : token(t), nxsset(iset), max(maxValue), block(nxsblk), settype(type)
{
   if( !nxsset.empty() )
      nxsset.erase( nxsset.begin(), nxsset.end() );
}

/**
 * @method AddRange [bool:protected]
 * @param first [int] the first member of the range (inclusive)
 * @param last [int] the last member of the range (inclusive)
 * @param modulus [int] the modulus to use (if non-zero)
 *
 * Adds the range specified by first, last, and modulus to the set.  If
 * modulus is zero (the default value) it is ignored.  The parameters
 * first and last are from the data file and thus have range [1..max].
 * We store them with offset = 0 in nxsset (i.e., subtract 1 from every
 * value stored).
 */
bool SetReader::AddRange( int first, int last, int modulus /* = 0 */ )
{
	if( last > max || first < 1 || first > last )
		return false;
	for( int i = first-1; i < last; i++ ) {
		int diff = i-first+1;
		if( modulus > 0 && diff % modulus != 0 )
			continue;
		nxsset.insert(i);
	}
	return true;
}

/**
 * @method GetTokenValue [int:private]
 * @throws XNexus
 *
 * Tries to interpret token as a number.  Failing that,
 * tries to interpret token as a character or taxon
 * label, which it then converts to a number.  Failing
 * that, it throws an XNexus exception.
 */
int SetReader::GetTokenValue()
{
   int v = atoi( token.GetToken().c_str() );

   if( v == 0 && settype != SetReader::generic )
   {
      if( settype == SetReader::charset )
         v = block.CharLabelToNumber( token.GetToken() );
      else if( settype == SetReader::taxset )
         v = block.TaxonLabelToNumber( token.GetToken() );
   }

   if( v == 0 )
   {
      block.errormsg = "Set element (";
      block.errormsg += token.GetToken();
      block.errormsg += ") not a number ";
      if( settype == SetReader::charset )
         block.errormsg += "and not a valid character label";
      else if( settype == SetReader::taxset )
         block.errormsg += "and not a valid taxon label";
      throw XNexus( block.errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
   }

   return v;
}

/**
 * @method Run [bool:public]
 * @throws XNexus
 *
 * Reads in a set from a NEXUS data file. Returns true if the set was
 * terminated by a semicolon, false otherwise.
 */
bool SetReader::Run()
{
	bool ok;
	bool retval = false;

	int rangeBegin = -1;
	int rangeEnd = rangeBegin;
	bool insideRange = false;
	int modValue = 0;
	for(;;)
	{
		// next token should be one of the following:
		// ';'  --> set definition finished
		// '-'  --> range being defined
		// int  --> member of set (or beginning or end of a range)
		// '.'  --> signifies the number max
		// '\' --> signifies modulus value coming next
		//
		token.GetNextToken();

		if( token.Equals("-") )
		{
         // We should not be inside a range when we encounter a hyphenation symbol.
         // The hyphen is what _puts_ us inside a range!
			if( insideRange ) {
				block.errormsg = "The symbol '-' is out of place here";
				throw XNexus( block.errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
			}
			insideRange = true;
		}

		else if( token.Equals(".") )
		{
         // We _should_ be inside a range if we encounter a period, as this
         // is a range termination character
			if( !insideRange ) {
				block.errormsg = "The symbol '.' can only be used to specify the end of a range";
				throw XNexus( block.errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
			}
			rangeEnd = max;
		}

		else if( token.Equals("\\") )
		{
         // The backslash character is used to specify a modulus to a range, and
         // thus should only be encountered if currently inside a range
			if( !insideRange ) {
				block.errormsg = "The symbol '\\' can only be used after the end of a range has been specified";
				throw XNexus( block.errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
			}

			token.GetNextToken();
			modValue = atoi( token.GetToken().c_str() );
			if( modValue <= 0 ) {
				block.errormsg = "The modulus value specified (";
            block.errormsg += token.GetToken();
            block.errormsg += ") is invalid; must be greater than 0";
				throw XNexus( block.errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
			}
		}

		else if( insideRange && rangeEnd == -1 )
      {
         // The beginning of the range and the hyphen symbol have been read
         // already, just need to store the end of the range at this point
         rangeEnd = GetTokenValue();
      }

      else if( insideRange )
      {
         // If insideRange is true, we must have already stored the beginning
         // of the range and read in the hyphen character.  We would not have
         // made it this far if we had also not already stored the range end.
         // Thus, we can go ahead and add the range.
         ok = AddRange( rangeBegin, rangeEnd, modValue );
         if( !ok ) {
            block.errormsg = "Character number out of range (or range incorrectly specified) in set specification";
            throw XNexus( block.errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
         }

         // We have actually already read in the next token, so deal with it
         // now so that we don't end up skipping a token
         if( token.Equals(";") ) {
         	retval = true;
            break;
         }
         else if( token.Equals(",") )
         	break;
         rangeBegin = GetTokenValue();
         rangeEnd = -1;
         insideRange = false;
      }
      else if( rangeBegin != -1 )
      {
         // If we were inside a range, we would have not gotten this far.
         // If not in a range, we are either getting ready to begin a new
         // range or have previously read in a single value.  Handle the
         // latter possibility here.
         ok = AddRange( rangeBegin, rangeBegin, modValue );
         if( !ok ) {
            block.errormsg = "Character number out of range (or range incorrectly specified) in set specification";
            throw XNexus( block.errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
         }
         if( token.Equals(";") ) {
         	retval = true;
            break;
         }
         else if( token.Equals(",") )
         	break;
         rangeBegin = GetTokenValue();
         rangeEnd = -1;
      }
      else if( token.Equals(";") ) {
      	retval = true;
         break;
      }
      else if( token.Equals(",") ) {
         break;
      }
      else if( token.Equals("ALL") ) {
         rangeBegin = 1;
         rangeEnd = max;
         ok = AddRange( rangeBegin, rangeEnd );
      }
      else {
         // Can only get here if rangeBegin still equals -1 and thus we
         // are reading in the very first token and that token is neither
         // the word "all" nor is it a semicolon
         rangeBegin = GetTokenValue();
         rangeEnd = -1;
      }
	}
	
	return retval;
}

