#include "nexusdefs.h"
#include "xnexus.h"
#include "nexustoken.h"
#include "nexus.h"
#include "taxablock.h"
#include "distancedatum.h"
#include "distancesblock.h"

/**
 * @class      DistancesBlock
 * @file       distancesblock.h
 * @file       distancesblock.cpp
 * @author     Paul O. Lewis
 * @copyright  Copyright © 1999. All Rights Reserved.
 * @variable   diagonal [int:private] indicates whether the diagonal elements of the matrix will be provided
 * @variable   interleave [int:private] indicates whether the matrix will be interleaved
 * @variable   labels [int:private] indicates whether taxon labels are provided in the matrix
 * @variable   matrix [DistanceDatum**:private] array holding distance data
 * @variable   missing [char:private] symbol used to indicate missing data
 * @variable   nchar [int:private] the number of characters used in generating the pairwise distances
 * @variable   newtaxa [int:private] if 1, new taxa will be defined in the matrix
 * @variable   ntax [int:private] the number of taxa (determines dimensions of the matrix)
 * @variable   taxa [TaxaBlock&:private] reference to a TaxaBlock object
 * @variable   taxonPos [int*:private] array holding 0-offset index into taxa's list of taxon labels
 * @variable   triangle [int:private] indicates whether matrix is upper triangular, lower triangular, or rectangular
 * @see        NexusReader
 * @see        NexusToken
 * @see        XNexus
 *
 * This class handles reading and storage for the Nexus block DISTANCES.
 * It overrides the member functions Read and Reset, which are abstract
 * virtual functions in the base class NexusBlock.
 *
 * <P> Below is a table showing the correspondence between the elements of a
 * DISTANCES block and the variables and member functions that can be used
 * to access each piece of information stored.
 *
 * <p><table border=1>
 * <tr>
 *   <th> Nexus command
 *   <th> Nexus subcommand
 *   <th> Data Members
 *   <th> Member Functions
 * <tr>
 *   <td rowspan=3 align=left> DIMENSIONS
 *   <td> NEWTAXA
 *   <td> int <a href="#newtaxa">newtaxa</a>
 *   <td>
 * <tr>
 *   <td> NTAX
 *   <td> int <a href="#ntax">ntax</a>
 *   <td> int <a href="#GetNtax">GetNtax()</a>
 * <tr>
 *   <td> NCHAR
 *   <td> int <a href="#ntax">nchar</a>
 *   <td> int <a href="#GetNchar">GetNchar()</a>
 * <tr>
 *   <td rowspan=5 align=left> FORMAT
 *   <td> TRIANGLE
 *   <td> int <a href="#triangle">triangle</a>
 *   <td> int <a href="#GetTriangle">GetTriangle()</a>
 *        <br> int <a href="#IsUpperTriangular">IsUpperTriangular()</a>
 *        <br> int <a href="#IsLowerTriangular">IsLowerTriangular()</a>
 *        <br> int <a href="#IsBoth">IsBoth()</a>
 * <tr>
 *   <td> [NO]DIAGONAL
 *   <td> int <a href="#diagonal">diagonal</a>
 *   <td> int <a href="#IsDiagonal">IsDiagonal()</a>
 * <tr>
 *   <td> [NO]LABELS
 *   <td> int <a href="#labels">labels</a>
 *   <td> int <a href="#IsLabels">IsLabels()</a>
 * <tr>
 *   <td> MISSING
 *   <td> char <a href="#missing">missing</a>
 *   <td> char <a href="#GetMissingSymbol">GetMissingSymbol()</a>
 * <tr>
 *   <td> INTERLEAVE
 *   <td> int <a href="#interleave">interleave</a>
 *   <td> int <a href="#IsInterleave">IsInterleave()</a>
 * <tr>
 *   <td colspan=2 align=left> TAXLABELS
 *   <td> (stored in TaxaBlock object)
 *   <td> (access through taxa)
 * <tr>
 *   <td colspan=2 align=left> MATRIX
 *   <td> DistanceDatum** <a href="#matrix">matrix</a>
 *   <td> double <a href="#GetDistance">GetDistance( i, j )</a>
 *        <br> int <a href="#IsMissing">IsMissing( i, j )</a>
 *        <br> void <a href="#SetMissing">SetMissing( i, j )</a>
 *        <br> void <a href="#SetDistance">SetDistance( i, j, double d )</a>
 * </table>
 */

/**
 * @enumeration
 * @enumitem  upper [1] matrix is upper-triangular
 * @enumitem  lower [2] matrix is lower-triangular
 * @enumitem  both  [3] matrix is rectangular
 *
 * For use with the variable triangle.
 */

/**
 * @constructor
 *
 * Default constructor. Performs the following initializations:
 * <table>
 * <tr><th align="left">Variable <th> <th align="left"> Initial Value
 * <tr><td> id             <td>= <td> "DISTANCES"
 * <tr><td> diagonal       <td>= <td> 1
 * <tr><td> interleave     <td>= <td> 0
 * <tr><td> labels         <td>= <td> 1
 * <tr><td> matrix         <td>= <td> NULL
 * <tr><td> missing        <td>= <td> '?'
 * <tr><td> nchar          <td>= <td> 0
 * <tr><td> newtaxa        <td>= <td> 0
 * <tr><td> ntax           <td>= <td> 0
 * <tr><td> taxonPos       <td>= <td> NULL
 * <tr><td> triangle       <td>= <td> lower
 * </table>
 */
DistancesBlock::DistancesBlock( TaxaBlock& t ) : taxa(t), NexusBlock()
{
   id = "DISTANCES";

   newtaxa     = 0;
   ntax        = 0;
   nchar       = 0;
   diagonal    = 1;
   triangle    = lower;
   interleave  = 0;
   labels      = 1;
   missing     = '?';
   matrix      = NULL;
   taxonPos    = NULL;
}

/**
 * @destructor
 *
 * Deletes the memory used by id and flushes taxonLabels.
 */
DistancesBlock::~DistancesBlock()
{
   if( matrix != NULL )
      delete matrix;
   if( taxonPos != NULL )
      delete [] taxonPos;
}

/**
 * @method HandleDimensionsCommand [void:protected]
 * @param token [NexusToken&] the token used to read from in
 * @throws XNexus
 *
 * Called when DIMENSIONS command needs to be parsed from within the
 * DISTANCES block.  Deals with everything after the token DIMENSIONS
 * up to and including the semicolon that terminates the DIMENSIONS
 * command.
 */
void DistancesBlock::HandleDimensionsCommand( NexusToken& token )
{
	for(;;)
	{
		token.GetNextToken();

      // token should either be ';' or the name of a subcommand
      //
		if( token.Equals(";") ) {
			break;
		}
      else if( token.Equals("NEWTAXA") ) {
         ntax = 0;
         newtaxa = 1;
      }
      else if( token.Equals("NTAX") ) {
         if( !newtaxa ) {
      		errormsg = "Must specify NEWTAXA before NTAX if new taxa are being defined";
      		throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
         }

         // this should be the equals sign
   		token.GetNextToken();
         if( !token.Equals("=") ) {
      		errormsg = "Expecting '=' but found ";
            errormsg += token.GetToken();
            errormsg += " instead";
      		throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
         }

         // this should be the number of taxa
   		token.GetNextToken();
         ntax = atoi( token.GetToken().c_str() );
      }
      else if( token.Equals("NCHAR") ) {
         // this should be the equals sign
   		token.GetNextToken();
         if( !token.Equals("=") ) {
      		errormsg = "Expecting '=' but found ";
            errormsg += token.GetToken();
            errormsg += " instead";
      		throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
         }

         // this should be the number of characters
   		token.GetNextToken();
         nchar = atoi( token.GetToken().c_str() );
      }
	}

   if( ntax == 0 )
      ntax = taxa.GetNumTaxonLabels();
}

/**
 * @method HandleFormatCommand [void:protected]
 * @param token [NexusToken&] the token used to read from in
 * @throws XNexus
 *
 * Called when FORMAT command needs to be parsed from within the
 * DISTANCES block.  Deals with everything after the token FORMAT
 * up to and including the semicolon that terminates the FORMAT
 * command.
 */
void DistancesBlock::HandleFormatCommand( NexusToken& token )
{
	for(;;)
	{
		token.GetNextToken();

      // token should either be ';' or the name of a subcommand
      //
		if( token.Equals(";") ) {
			break;
		}
      else if( token.Equals("TRIANGLE") ) {

         // this should be the equals sign
   		token.GetNextToken();
         if( !token.Equals("=") ) {
      		errormsg = "Expecting '=' but found ";
            errormsg += token.GetToken();
            errormsg += " instead";
      		throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
         }

         // this should be LOWER, UPPER, or BOTH
   		token.GetNextToken();
         if( token.Equals("LOWER") )
            triangle = lower;
         else if( token.Equals("UPPER") )
            triangle = upper;
         else if( token.Equals("BOTH") )
            triangle = both;
         else {
      		errormsg = "Expecting UPPER, LOWER, or BOTH but found ";
            errormsg += token.GetToken();
            errormsg += " instead";
      		throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
         }

      }
      else if( token.Equals("DIAGONAL") ) {
         diagonal = 1;
      }
      else if( token.Equals("NODIAGONAL") ) {
         diagonal = 0;
      }
      else if( token.Equals("LABELS") ) {
         labels = 1;
      }
      else if( token.Equals("NOLABELS") ) {
         labels = 0;
      }
      else if( token.Equals("INTERLEAVE") ) {
         interleave = 1;
      }
      else if( token.Equals("NOINTERLEAVE") ) {
         interleave = 0;
      }
      else if( token.Equals("MISSING") ) {

         // this should be the equals sign
   		token.GetNextToken();
         if( !token.Equals("=") ) {
      		errormsg = "Expecting '=' but found ";
            errormsg += token.GetToken();
            errormsg += " instead";
      		throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
         }

         // this should be the missing data symbol
   		token.GetNextToken();
         if( token.GetTokenLength() != 1 ) {
      		errormsg = "Missing data symbol specified (";
            errormsg += token.GetToken();
            errormsg += ") is invalid (must be a single character)";
      		throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
         }
         missing = token.GetToken()[0];
      }
      else {
         errormsg = "Token specified (";
         errormsg += token.GetToken();
         errormsg += ") is an invalid subcommand for the FORMAT command";
         throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
      }
	}
}

/**
 * @method HandleNextPass [int:protected]
 * @param token [NexusToken&] the token we are using for reading the data file
 * @param offset [int&] the offset (see below)
 *
 * Called from within HandleMatrix, this function is used to deal with interleaved
 * matrices.  It is called once for each pass through the taxa.
 *
 * <P>The local variable jmax records the number of columns read in the current
 * interleaved page and is used to determine the offset used for j in subsequent
 * pages.
 */
int DistancesBlock::HandleNextPass( NexusToken& token, int& offset )
{
   int i, j, k, jmax = 0, done = 0;

   int i_first = 0;
   if( triangle == lower )
      i_first = offset;

   int i_last  = ntax;

   for( i = i_first; i < i_last; i++ )
   {
      // Deal with taxon label if provided
      //
      if( labels && ( !newtaxa || offset>0 ) )
      {
         do {
            token.SetLabileFlagBit( NexusToken::newlineIsToken );
            token.GetNextToken();
         } while( token.AtEOL() );
         try {
            k = taxa.FindTaxon( token.GetToken() );
            if( taxonPos[i]==-1 )
               taxonPos[i] = k;
            else if( taxonPos[i] != k ) {
               errormsg = "Taxon labeled ";
               errormsg += token.GetToken();
               errormsg += " is out of order compared to previous interleave pages";
               throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
            }
         }
         catch( out_of_range ) {
            errormsg = "Could not find ";
            errormsg += token.GetToken();
            errormsg += " among taxa previously defined";
            throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
         }
      }
      else if( labels && newtaxa ) {
         do {
            token.SetLabileFlagBit( NexusToken::newlineIsToken );
            token.GetNextToken();
         } while( token.AtEOL() );
         taxa.AddTaxonLabel( token.GetToken() );
         taxonPos[i] = i;
      }

      // Now deal with the row of distance values
      //
      int true_j = 0;
      for( j = 0; j < ntax; j++ )
      {
         if( i == ntax-1 && j == ntax-1 ) {
            done = 1;
         }

         if( i == ntax-1 && true_j == ntax-1 ) {
            done = 1;
            break;
         }

         if( i == ntax-1 && !diagonal && triangle==upper ) {
            done = 1;
            break;
         }

         if( !diagonal && triangle==lower && j == ntax-offset-1) {
            done = 1;
            break;
         }

         token.SetLabileFlagBit( NexusToken::newlineIsToken );
         token.GetNextToken();

      	if( token.AtEOL() ) {
            if( j > jmax ) {
               jmax = j;
               if( !diagonal && triangle == upper && i >= offset )
                  jmax++;
               if( interleave && triangle == upper )
                     i_last = jmax + offset;
            }
            break;
         }

         true_j = j+offset;
         if( triangle==upper && i > offset )
            true_j += ( i - offset );
         if( !diagonal && triangle==upper && i >= offset )
            true_j++;

         if( true_j == ntax ) {
            errormsg = "Too many distances specified in row just read in";
            throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
         }

         string t = token.GetToken();
         if( token.GetTokenLength() == 1 && t[0] == missing )
            SetMissing( i, true_j );
         else
            SetDistance( i, true_j, atof( t.c_str() ) );
      }

   }

   offset += jmax;

   if( done )
      return 1;
   else
      return 0;
}

/**
 * @method HandleMatrixCommand [void:protected]
 * @param token [NexusToken&] the token used to read from in
 * @throws XNexus
 *
 * Called when MATRIX command needs to be parsed from within the
 * DISTANCES block.  Deals with everything after the token MATRIX
 * up to and including the semicolon that terminates the MATRIX
 * command.
 */
void DistancesBlock::HandleMatrixCommand( NexusToken& token )
{
   int i;

   if( ntax == 0 )
      ntax = taxa.GetNumTaxonLabels();
   if( ntax == 0 ) {
      errormsg = "MATRIX command cannot be read if NTAX is zero";
      throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
   }

   if( triangle == both && !diagonal ) {
      errormsg = "Cannot specify NODIAGONAL and TRIANGLE=BOTH at the same time";
      throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
   }

   if( newtaxa )
      taxa.Reset();

   // allocate memory to hold the taxonPos array
   //
   if( taxonPos != NULL )
      delete [] taxonPos;
   taxonPos = new int[ntax];
   for( i = 0; i < ntax; i++ )
      taxonPos[i] = -1;

   // allocate memory to hold the matrix
   //
   if( matrix != NULL )
      delete matrix;
   matrix = new DistanceDatum*[ntax];
   for( i = 0; i < ntax; i++ )
      matrix[i] = new DistanceDatum[ntax];

   int offset = 0;
   int done = 0;
   while( !done ) {
      done = HandleNextPass( token, offset );
   }

   // token should be equal to the terminating semicolon
   token.GetNextToken();
   if( !token.Equals(";") ) {
      errormsg = "Expecting ';' to terminate MATRIX command, but found ";
      errormsg += token.GetToken();
      errormsg += " instead";
      throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
   }
}

/**
 * @method HandleTaxlabelsCommand [void:protected]
 * @param token [NexusToken&] the token used to read from in
 * @throws XNexus
 *
 * Called when TAXLABELS command needs to be parsed from within the
 * DISTANCES block.  Deals with everything after the token TAXLABELS
 * up to and including the semicolon that terminates the TAXLABELS
 * command.
 */
void DistancesBlock::HandleTaxlabelsCommand( NexusToken& token )
{
	if( !newtaxa ) {
		errormsg = "NEWTAXA must have been specified in DIMENSIONS command to use the TAXLABELS command in a ";
      errormsg += id;
      errormsg += " block";
		throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
	}

   if( ntax == 0 ) {
      errormsg = "NTAX must be specified before TAXLABELS command";
      throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
   }

   for( int i = 0; i < ntax; i++ ) {
      token.GetNextToken();
      taxa.AddTaxonLabel( token.GetToken() );
   }

   token.GetNextToken(); // this should be terminating semicolon
   if( !token.Equals(";") ) {
      errormsg = "Expecting ';' to terminate TAXLABELS command, but found ";
      errormsg += token.GetToken();
      errormsg += " instead";
      throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
   }

   // OPEN ISSUE: Some may object to setting newtaxa to 0 here, because then the
   // fact that new taxa were specified in this DISTANCES block rather than in
   // a preceding TAXA block is lost.  This will only be important if we wish to
   // recreate the original data file, which I don't anticipate anyone doing with
   // this code (too difficult to remember all comments, the order of blocks in
   // the file, etc.)

   newtaxa = 0;
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
void DistancesBlock::Read( NexusToken& token )
{
   isEmpty = false;
	token.GetNextToken(); // this should be the semicolon after the block name
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

		if( token.Equals("DIMENSIONS") ) {
         HandleDimensionsCommand( token );
		}
		else if( token.Equals("FORMAT") ) {
         HandleFormatCommand( token );
		}
		else if( token.Equals("TAXLABELS") ) {
         HandleTaxlabelsCommand( token );
		}
		else if( token.Equals("MATRIX") ) {
         HandleMatrixCommand( token );
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
void DistancesBlock::Report( ostream& out )
{
   int ntaxTotal = ntax;
   if( ntaxTotal == 0 )
      ntaxTotal = taxa.GetNumTaxonLabels();

	out << endl;
	out << id << " block contains ";
	if( ntaxTotal == 0 ) {
		out << "no taxa" << endl;
	}
	else if( ntaxTotal == 1 )
		out << "one taxon" << endl;
	else
		out << ntaxTotal << " taxa" << endl;

   if( IsLowerTriangular() )
      out << "  Matrix is lower-triangular" << endl;
   else if( IsUpperTriangular() )
      out << "  Matrix is upper-triangular" << endl;
   else
      out << "  Matrix is rectangular" << endl;

   if( IsInterleave() )
      out << "  Matrix is interleaved" << endl;
   else 
      out << "  Matrix is non-interleaved" << endl;

   if( IsLabels() )
      out << "  Taxon labels provided" << endl;
   else
      out << "  No taxon labels provided" << endl;

   if( IsDiagonal() )
      out << "  Diagonal elements specified" << endl;
   else 
      out << "  Diagonal elements not specified" << endl;

   out << "  Missing data symbol is " << missing << endl;

	if( ntax == 0 ) return;

   out.setf( ios::floatfield, ios::fixed );
   out.setf( ios::showpoint );
	for( int i = 0; i < ntax; i++ )
   {
      if( labels )
         out << setw(20) << taxa.GetTaxonLabel(i);
      else
         out << "\t\t";
      for( int j = 0; j < ntax; j++ )
      {
         if( triangle==upper && j < i ) {
            out << setw(12) << " ";
         }
         else if( triangle==lower && j > i )
            continue;
         else if( !diagonal && i == j ) {
            out << setw(12) << " ";
         }
         else if( IsMissing( i, j ) )
            out << setw(12) << missing;
         else
            out << setw(12) << setprecision(5) << GetDistance( i, j );
      }
      out << endl;
   }
}

/**
 * @method Reset [void:protected]
 *
 * Flushes taxonLabels and sets ntax to 0 in preparation for reading a
 * new TAXA block.
 */
void DistancesBlock::Reset()
{
   isEmpty     = true;
   newtaxa     = 0;
   ntax        = 0;
   nchar       = 0;
   diagonal    = 1;
   triangle    = lower;
   interleave  = 0;
   labels      = 1;
   missing     = '?';

   if( matrix != NULL )
      delete matrix;
   matrix = NULL;

   if( taxonPos != NULL )
      delete taxonPos;
   taxonPos = NULL;
}

/**
 * @method GetNtax [int:public]
 *
 * Returns the value of ntax.
 */
int DistancesBlock::GetNtax()
{
   return ntax;
}

/**
 * @method GetNchar [int:public]
 *
 * Returns the value of nchar.
 */
int DistancesBlock::GetNchar()
{
   return nchar;
}

/**
 * @method GetDistance [double:public]
 * @param i [int] the row
 * @param j [int] the column
 *
 * Returns the value of the (i, j)th element of matrix.
 * Assumes i and j are both in the range [0..ntax)
 * and the distance stored at matrix[i][j] is not
 * missing.  Also assumes matrix is not NULL.
 */
double DistancesBlock::GetDistance( int i, int j )
{
   assert( i >= 0 );
   assert( i < ntax );
   assert( j >= 0 );
   assert( j < ntax );
   assert( matrix != NULL );

   return matrix[i][j].value;
}

/**
 * @method GetMissingSymbol [char:public]
 *
 * Returns the value of missing.
 */
char DistancesBlock::GetMissingSymbol()
{
   return missing;
}

/**
 * @method GetTriangle [int:public]
 *
 * Returns the value of triangle.
 */
int DistancesBlock::GetTriangle()
{
   return triangle;
}

/**
 * @method IsBoth [int:public]
 *
 * Returns 1 if the value of triangle is both, 0 otherwise.
 */
int DistancesBlock::IsBoth()
{
   return ( triangle == both ? 1 : 0 );
}

/**
 * @method IsUpperTriangular [int:public]
 *
 * Returns 1 if the value of triangle is upper, 0 otherwise.
 */
int DistancesBlock::IsUpperTriangular()
{
   return ( triangle == upper ? 1 : 0 );
}

/**
 * @method IsLowerTriangular [int:public]
 *
 * Returns 1 if the value of triangle is lower, 0 otherwise.
 */
int DistancesBlock::IsLowerTriangular()
{
   return ( triangle == lower ? 1 : 0 );
}

/**
 * @method IsDiagonal [int:public]
 *
 * Returns the value of diagonal.
 */
int DistancesBlock::IsDiagonal()
{
   return diagonal;
}

/**
 * @method IsInterleave [int:public]
 *
 * Returns the value of interleave.
 */
int DistancesBlock::IsInterleave()
{
   return interleave;
}

/**
 * @method IsLabels [int:public]
 *
 * Returns the value of labels.
 */
int DistancesBlock::IsLabels()
{
   return labels;
}

/**
 * @method IsMissing [int:public]
 * @param i [int] the row
 * @param j [int] the column
 *
 * Returns 1 if the (i,j)th distance is missing.
 * Assumes i and j are both in the range [0..ntax)
 * and matrix is not NULL.
 */
int DistancesBlock::IsMissing( int i, int j )
{
   assert( i >= 0 );
   assert( i < ntax );
   assert( j >= 0 );
   assert( j < ntax );
   assert( matrix != NULL );

   return ( matrix[i][j].missing );
}

/**
 * @method SetDistance [void:public]
 * @param i [int] the row
 * @param j [int] the column
 * @param d [double] the distance value
 *
 * Sets the value of the (i,j)th matrix element to d
 * and the missing flag to 0.
 * Assumes i and j are both in the range [0..ntax)
 * and matrix is not NULL.
 */
void DistancesBlock::SetDistance( int i, int j, double d )
{
   assert( i >= 0 );
   assert( i < ntax );
   assert( j >= 0 );
   assert( j < ntax );
   assert( matrix != NULL );

   matrix[i][j].value = d;
   matrix[i][j].missing = 0;
}

/**
 * @method SetMissing [void:public]
 * @param i [int] the row
 * @param j [int] the column
 *
 * Sets the value of the (i,j)th matrix element to missing.
 * Assumes i and j are both in the range [0..ntax)
 * and matrix is not NULL.
 */
void DistancesBlock::SetMissing( int i, int j )
{
   assert( i >= 0 );
   assert( i < ntax );
   assert( j >= 0 );
   assert( j < ntax );
   assert( matrix != NULL );

   matrix[i][j].missing = 1;
   matrix[i][j].value = 0.0;
}

/**
 * @method SetNchar [void:private]
 * @param n [int] the number of characters
 *
 * Sets nchar to n.
 */
void DistancesBlock::SetNchar( int n )
{
   nchar = n;
}

