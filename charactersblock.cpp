#include "nexusdefs.h"
#include "xnexus.h"
#include "nexustoken.h"
#include "nexus.h"
#include "setreader.h"
#include "taxablock.h"
#include "discretedatum.h"
#include "discretematrix.h"
#include "assumptionsblock.h"
#include "charactersblock.h"

/**
 * @class      CharactersBlock
 * @file       charactersblock.h
 * @file       charactersblock.cpp
 * @author     Paul O. Lewis
 * @copyright  Copyright © 1999. All Rights Reserved.
 * @variable   activeChar [bool*:private] activeChar[i] is true if character i not excluded; i is in range [0..nchar)
 * @variable   activeTaxon [bool*:private] activeTaxon[i] is true if taxon i not deleted; i is in range [0..ntax)
 * @variable   charLabels [LabelList:private] storage for character labels (if provided)
 * @variable   charStates [LabelListBag:private] storage for character state labels (if provided)
 * @variable   datatype [int:private] flag variable (see enum starting with standard)
 * @variable   deleted [IntSet:private] set of (0-offset) numbers for taxa that have been deleted
 * @variable   eliminated [IntSet:private] array of (0-offset) character numbers that have been ELIMINATEd (will remain empty if no ELIMINATE command encountered)
 * @variable   equates [AssocList:private] list of associations defined by EQUATE subcommand of FORMAT command
 * @variable   gap [char:private] gap symbol for use with molecular data
 * @variable   interleaving [bool:private] indicates matrix will be in interleaved format
 * @variable   labels [bool:private] indicates whether or not labels will appear on left side of matrix
 * @variable   matchchar [char:private] match symbol to use in matrix
 * @variable   matrix [DiscreteMatrix*:private] storage for discrete data
 * @variable   missing [char:private] missing data symbol
 * @variable   nchar [int:private] number of columns in matrix (same as ncharTotal unless some characters were ELIMINATEd, in which case ncharTotal > nchar)
 * @variable   ncharTotal [int:private] total number of characters (same as nchar unless some characters were ELIMINATEd, in which case ncharTotal > nchar)
 * @variable   newchar [bool:private] true unless CHARLABELS or CHARSTATELABELS command read
 * @variable   newtaxa [bool:private] true if NEWTAXA keyword encountered in DIMENSIONS command
 * @variable   ntax [int:private] number of rows in matrix (same as ntaxTotal unless fewer taxa appeared in CHARACTERS MATRIX command than were specified in the TAXA block, in which case ntaxTotal > ntax)
 * @variable   ntaxTotal [int:private] total number of taxa (same as ntax unless fewer taxa appeared in CHARACTERS MATRIX command than were specified in the TAXA block, in which case ntaxTotal > ntax)
 * @variable   respectingCase [bool:private] if true, RESPECTCASE keyword specified in FORMAT command
 * @variable   symbols [char*:private] list of valid character state symbols
 * @variable   charPos [int*:private] maps character numbers in the data file to column numbers in matrix (necessary if some characters have been ELIMINATEd)
 * @variable   taxa [TaxaBlock&:protected] reference to the TAXA block in which taxon labels are stored
 * @variable   taxonPos [int*:private] maps taxon numbers in the data file to row numbers in matrix (necessary if fewer taxa appear in CHARACTERS block MATRIX command than are specified in the TAXA block)
 * @variable   tokens [bool:private] if false, data matrix entries must be single symbols; if true, multicharacter entries are allows
 * @variable   transposing [bool:private] indicates matrix will be in transposed format
 * @see        DiscreteDatum
 * @see        DiscreteMatrix
 * @see        LabelListAssoc
 * @see        Nexus
 * @see        NexusBlock
 * @see        NexusReader
 * @see        NexusToken
 * @see        SetReader
 * @see        TaxaBlock
 * @see        XNexus
 *
 * This class handles reading and storage for the Nexus block CHARACTERS.
 * It overrides the member functions Read and Reset, which are abstract
 * virtual functions in the base class NexusBlock.
 *
 * <P>The issue of bookkeeping demands a careful explanation.  Users are
 * allowed to control the number of characters analyzed either by "eliminating"
 * or "excluding" characters.  Characters can be <b>eliminated</b> (by using the 
 * ELIMINATE command) at the time of execution of the data file, but not
 * thereafter.  Characters can, however, be <b>excluded</b> at any time after the
 * data are read.  No storage is provided for eliminated characters, whereas
 * excluded characters must be stored because at any time they could be restored
 * to active status.  Because one can depend on eliminated characters continuing
 * to be eliminated, it would be inefficient to constantly have to check 
 * whether a character has been eliminated.  Hence, the characters are renumbered
 * so that one can efficiently traverse the entire range of non-eliminated 
 * characters.  The original range of characters will be hereafter denoted
 * [0..ncharTotal), whereas the new, reduced range will be denoted [0..nchar).
 * The two ranges exactly coincide if ncharTotal = nchar (i.e., no ELIMINATE
 * command was specified in the CHARACTERS block.
 *
 * <P>The possibility for eliminating and excluding characters creates a very 
 * confusing situation that is exacerbated by the fact that character <i>indices</i>
 * used in the code begin at 0 whereas character <i>numbers</i> in the data file
 * begin at 1. The convention used hereafter will be to specify "character number k"
 * when discussing 1-offset character numbers in the data file and either "character index k"
 * or simply "character k" when discussing 0-offset character indices. 
 *
 * There are several functions (and data structures) that provide  services related to  
 * keeping track of the correspondence between character indices in the stored data matrix 
 * compared to character numbers in the original data file. The array <b>charPos</b> can 
 * be used to find the index of one of the original characters in the matrix. The 
 * function <b>GetCharPos</b> provides public access to the protected charPos array. For 
 * example, if character 9 (= character number 10) was the only one ELIMINATEd, 
 * GetCharPos(9) would return -1 indicating that that character 9 does not now exist.
 * GetCharPos(10) returns 9 indicating that character 10 in the data file corresponds
 * to character 9 in the stored data matrix.  All public functions in which a 
 * character number must be supplied (such as GetInternalRepresentation) assume 
 * that the character number is the <i>current</i> position of the character in 
 * the data matrix. This allows one to quickly traverse the data matrix without having to
 * constantly check whether or not a character was eliminated.  Note that
 * <b>GetNChar</b> returns nchar, not ncharTotal, and this function should be used
 * to obtain the endpoint for a traversal of characters in the matrix.
 * Other functions requiring a (current) character index are listed below:
 *
 * <pre>
 * int GetInternalRepresentation( int i, int j, int k = 0 );
 * int GetNumStates( int i, int j );
 * virtual int GetObsNumStates( int j );
 * int GetOrigCharIndex( int j );
 * int GetOrigCharNumber( int j );
 * char GetState( int i, int j, int k = 0 );
 * bool HandleNextState( NexusToken& token, int i, int j );
 * int HandleTokenState( token, j );
 * bool IsGapState( int i, int j );
 * bool IsMissingState( int i, int j );
 * bool IsPolymorphic( int i, int j );
 * void ShowStateLabels( ostream& out, int i, int j );
 * </pre>
 *
 * <p>The following function is exceptional in requiring (by necessity) the original
 * character index:
 *
 * <pre>
 * bool IsEliminated( int origCharIndex );
 * </pre>
 *
 * <p>The function <b>GetOrigCharIndex</b> returns the original character index for any
 * current character index. This is useful only when outputting information that
 * will be seen by the user, and in this case, it is really the character <i>number</i>
 * that should be output.  To get the original character number, either
 * add 1 to GetOrigCharIndex or call <b>GetOrigCharNumber</b> function (which simply returns
 * GetOrigCharIndex + 1).
 *
 * <p>Now we must deal with the issue of character <i>exclusion</i>.  A character may be
 * excluded by calling the function <b>ExcludeCharacter</b> and providing the <i>current</i> character index.
 * or by calling the function <b>ApplyExset</b> and supplying an exclusion set comprising <i>original</i>
 * character indices.  These functions manipulate a bool array, <b>activeChar</b>, which can be 
 * queried using one of two functions: <b>IsActiveChar</b> or <b>IsExcluded</b>.  The array activeChar is
 * nchar elements long, so IsActiveChar and IsExcluded both accept only current character
 * indices.  Thus, a normal loop through all characters in the data matrix should look
 * something like this:
 * <pre>
 *    for( int j = 0; j < nchar; j++ ) {
 *       if( IsExcluded(j) ) continue;
 *       .
 *       .
 *       .
 *    }
 * </pre>
 *
 * A corresponding set of data structures and functions exists to provide the same
 * services for taxa.  Thus, <b>ntax</b> holds the current number of taxa, whereas <b>ntaxTotal</b>
 * holds the number of taxa specified in the TAXA block.  If data is provided in
 * the MATRIX command for all taxa listed in the TAXA block, ntax will be equal to
 * ntaxTotal.  If data is not provided for some of the taxa, the ones left out
 * are treated just like eliminated characters.  The function <b>GetTaxonPos</b> can be
 * used to query the <b>taxonPos</b> array, which behaves like the charPos array does
 * for characters:  -1 for element i means that the taxon whose original index
 * was i has been "eliminated" and no data is stored for it in the matrix.  Otherwise,
 * <b>GetTaxonPos(i)</b> returns the current index corresponding to the taxon with an
 * original index of i.  The function <b>GetNTax</b> returns ntax, whereas <b>GetNTaxTotal</b>
 * must be used to gain access to ntaxTotal (but this is seldom necessary).
 * The functions <b>GetOrigTaxonIndex</b> and <b>GetOrigTaxonNumber</b> behave like their 
 * character counterparts, GetOrigCharIndex and GetOrigCharNumber.  Like characters,
 * taxa can be temporarily inactivated so that they do not participate in any
 * analyses.until they are reactivated by the user.  Inactivation of a taxon is
 * refered to as "deleting" the taxon, whereas "restoring" a taxon means reactivating
 * it.  Thus, the <b>ApplyDelset</b>, <b>DeleteTaxon</b>, and <b>RestoreTaxon</b> functions correspond
 * to the ApplyExset, ExcludeCharacter, and IncludeCharacter functions for characters.
 * To query whether a taxon is currently deleted, use either <b>IsActiveTaxon</b> or
 * <b>IsDeleted</b>.  A normal loop across all active taxa can be constructed as follows:
 * <pre>
 *    for( int i = 0; i < ntax; i++ ) {
 *       if( IsDeleted(i) ) continue;
 *       .
 *       .
 *       .
 *    }
 * </pre>
 *
 * <P> Below is a table showing the correspondence between the elements of a
 * CHARACTERS block and the variables and member functions that can be used
 * to access each piece of information stored.
 *
 * <p><table border=1>
 * <tr>
 *   <th> Nexus command
 *   <th> Nexus subcommand
 *   <th> Data Members
 *   <th> Member Functions
 * <tr>
 *   <td rowspan=3> DIMENSIONS
 *   <td> NEWTAXA
 *   <td> bool <a href="#newtaxa">newtaxa</a>
 *   <td>
 * <tr>
 *   <td> NTAX
 *   <td> int <a href="#ntax">ntax</a> (see also <a href="#ntaxTotal">ntaxTotal</a>)
 *   <td> int <a href="#GetNTax">GetNTax()</a> (see also <a href="#GetNumMatrixRows">GetNumMatrixRows()</a>)
 * <tr>
 *   <td> NCHAR
 *   <td> int <a href="#nchar">nchar</a> (see also <a href="#ncharTotal">ncharTotal</a>)
 *   <td> int <a href="#GetNChar">GetNChar()</a> (see also <a href="#GetNumMatrixCols">GetNumMatrixCols()</a>)
 * <tr>
 *   <td rowspan=13> FORMAT
 *   <td> DATATYPE
 *   <td> datatypes <a href="#datatype">datatype</a>
 *   <td> int <a href="#GetDataType">GetDataType()</a>
 * <tr>
 *   <td> RESPECTCASE
 *   <td> bool <a href="#respectingCase">respectingCase</a>
 *   <td> bool <a href="#IsRespectCase">IsRespectCase()</a>
 * <tr>
 *   <td> MISSING
 *   <td> char <a href="#missing">missing</a>
 *   <td> char <a href="#GetMissingSymbol">GetMissingSymbol()</a>
 * <tr>
 *   <td> GAP
 *   <td> char <a href="#gap">gap</a>
 *   <td> char <a href="#GetGapSymbol">GetGapSymbol()</a>
 * <tr>
 *   <td> SYMBOLS
 *   <td> char* <a href="#symbols">symbols</a>
 *   <td> char* <a href="#GetSymbols">GetSymbols()</a>
 * <tr>
 *   <td> EQUATE
 *   <td> AssocList <a href="#equates">equates</a>
 *   <td> char* <a href="#GetEquateKey">GetEquateKey( int k )</a>
 *        <br> char* <a href="#GetEquateValue">GetEquateValue( int k )</a>
 *        <br> int <a href="#GetNumEquates">GetNumEquates()</a>
 * <tr>
 *   <td> MATCHCHAR
 *   <td> char <a href="#matchchar">matchchar</a>
 *   <td> char <a href="#GetMatchcharSymbol">GetMatchcharSymbol()</a>
 * <tr>
 *   <td> (NO)LABELS
 *   <td> bool <a href="#labels">labels</a>
 *   <td> bool <a href="#IsLabels">IsLabels()</a>
 * <tr>
 *   <td> TRANSPOSE
 *   <td> bool <a href="#transposing">transposing</a>
 *   <td> bool <a href="#IsTranspose">IsTranspose()</a>
 * <tr>
 *   <td> INTERLEAVE
 *   <td> bool <a href="#interleaving">interleaving</a>
 *   <td> bool <a href="#IsInterleave">IsInterleave()</a>
 * <tr>
 *   <td> ITEMS
 *   <td colspan=2> Only ITEMS=STATES implemented
 * <tr>
 *   <td> STATESFORMAT
 *   <td colspan=2> Only STATESFORMAT=STATESPRESENT implemented
 * <tr>
 *   <td> (NO)TOKENS
 *   <td> bool <a href="#tokens">tokens</a>
 *   <td> bool <a href="#IsTokens">IsTokens()</a>
 * <tr>
 *   <td rowspan=1 colspan=2 align=left> ELIMINATE
 *   <td> IntSet <a href="#eliminated">eliminated</a>
 *   <td> bool <a href="#IsEliminated">IsEliminated( int origCharIndex )</a>
 *        <br> int <a href="#GetNumEliminated">GetNumEliminated()</a>
 * <tr>
 *   <td rowspan=1 colspan=2 align=left> MATRIX
 *   <td> DiscreteMatrix* <a href="#matrix">matrix</a>
 *   <td> char <a href="#GetState">GetState( int i, int j, int k = 0 )</a>
 *        <br> int <a href="#GetInternalRepresentation">GetInternalRepresentation( int i, int j, int k = 0 )</a>
 *        <br> int <a href="#GetNumStates">GetNumStates( int i, int j )</a>
 *        <br> int <a href="#GetNumMatrixRows">GetNumMatrixRows()</a>
 *        <br> int <a href="#GetNumMatrixCols">GetNumMatrixCols()</a>
 *        <br> bool <a href="#IsPolymorphic">IsPolymorphic( int i, int j )</a>
 * </table>
 */

/**
 * @enumeration
 * @enumitem  standard   [0] means standard data type
 * @enumitem  dna        [1] means dna data type
 * @enumitem  rna        [2] means rna data type
 * @enumitem  nucleotide [3] means nucleotide data type
 * @enumitem  protein    [4] means protein data type
 * @enumitem  continuous [5] means continuous data type
 *
 * For use with the variable datatype.  Default is 0 (standard data type).
 */

/**
 * @constructor
 * @param tb [TaxaBlock&] the taxa block object to consult for taxon labels
 * @param ab [AssumptionsBlock&] the assumptions block object to consult for exclusion sets
 *
 * Performs the following initializations:
 * <table>
 * <tr><th align="left">Variable <th> <th align="left"> Initial Value
 * <tr><td> id             <td>= <td> "CHARACTERS"
 * <tr><td> datatype       <td>= <td> standard
 * <tr><td> gap            <td>= <td> '\0'
 * <tr><td> interleaving   <td>= <td> false
 * <tr><td> labels         <td>= <td> true
 * <tr><td> matchchar      <td>= <td> '\0'
 * <tr><td> matrix         <td>= <td> NULL
 * <tr><td> missing        <td>= <td> '?'
 * <tr><td> nchar          <td>= <td> 0
 * <tr><td> ncharTotal     <td>= <td> 0
 * <tr><td> newchar        <td>= <td> true
 * <tr><td> newtaxa        <td>= <td> false
 * <tr><td> ntax           <td>= <td> 0
 * <tr><td> ntaxTotal      <td>= <td> 0
 * <tr><td> respectingCase <td>= <td> false
 * <tr><td> symbols        <td>= <td> "01"
 * <tr><td> taxa           <td>= <td> tb
 * <tr><td> charPos        <td>= <td> NULL
 * <tr><td> taxonPos       <td>= <td> NULL
 * <tr><td> tokens         <td>= <td> false
 * <tr><td> transposing    <td>= <td> false
 * </table>
 */
CharactersBlock::CharactersBlock( TaxaBlock& tb, AssumptionsBlock& ab )
	: taxa(tb), assumptionsBlock(ab), NexusBlock()
{
	id = "CHARACTERS";

	symbols = new char[NCL_MAX_STATES+1];
	symbols[0] = '0';
	symbols[1] = '1';
	symbols[2] = '\0';

	ntax           = 0;
	ntaxTotal      = 0;
	nchar          = 0;
	ncharTotal     = 0;
	newchar        = true;
	newtaxa        = false;
	interleaving   = false;
	transposing    = false;
	respectingCase = false;
	labels         = true;
	tokens         = false;
	datatype       = standard;
	missing        = '?';
	gap            = '\0';
	matchchar      = '\0';
	matrix         = NULL;
	charPos        = NULL;
	taxonPos       = NULL;
	activeTaxon    = NULL;
	activeChar     = NULL;
}

/**
 * @destructor
 *
 * Deletes any memory allocated to the arrays symbols, charPos, taxonPos,
 * activeChar, and activeTaxon.  Flushes the containers charLabels, eliminated,
 * and deleted. Also deletes memory allocated to matrix.
 */
CharactersBlock::~CharactersBlock()
{
	// Free memory allocated for the DiscreteMatrix object matrix
	//
	if( matrix != NULL )
		delete matrix;
   
	// Free memory allocated for the arrays symbols, charPos, taxonPos,
	// activeChar and activeTaxon
	//
	if( symbols != NULL )
		delete [] symbols;
	if( charPos != NULL )
		delete [] charPos;
	if( taxonPos != NULL )
		delete [] taxonPos;
	if( activeChar != NULL )
		delete [] activeChar;
	if( activeTaxon != NULL )
		delete [] activeTaxon;

	// Flush the containers eliminated, equates, charStates, and charLabels
	//
	charLabels.erase( charLabels.begin(), charLabels.end() );
	charStates.erase( charStates.begin(), charStates.end() );
	equates.erase( equates.begin(), equates.end() );
	eliminated.erase( eliminated.begin(), eliminated.end() );
}

/**
 * @method ApplyDelset [void:protected]
 * @param delset [IntSet&] set of taxon indices to delete in range [0..ntaxTotal)
 *
 * Deletes (i.e., excludes from further analyses) taxa
 * whose indices are contained in the set delset.  The
 * taxon indices refer to original taxon indices, not current
 * indices (originals will equal current ones iff number of 
 * taxa in TAXA block equals number of taxa in MATRIX command).
 * Returns the number of taxa actually deleted (some may have
 * already been deleted)
 */
int CharactersBlock::ApplyDelset( IntSet& delset )
{
	assert( activeTaxon != NULL );
	int num_deleted = 0;
	int k;
	//for( k = 0; k < ntax; k++ )
	//	activeTaxon[k] = true;
		
	IntSet::const_iterator i;
	for( i = delset.begin(); i != delset.end(); i++ ) {
		k = taxonPos[*i];
		if( k < 0 ) continue;
		
		// k greater than -1 means data was supplied for
		// this taxon and therefore it can be deleted
		//
		if( activeTaxon[k] == true )
			num_deleted++;
		activeTaxon[k] = false;
	}
	return num_deleted;
}

/**
 * @method ApplyExset [void:protected]
 * @param exset [IntSet&] set of character indices to exclude in range [0..ncharTotal)
 *
 * Excludes characters whose indices are contained in the set exset. The indices supplied 
 * should refer to the original character indices, not current character indices.
 * Returns number of characters actually excluded (some may have already been
 * excluded).
 */
int CharactersBlock::ApplyExset( IntSet& exset )
{
	assert( activeChar != NULL );
	int num_excluded = 0;
	int k;
	//for( k = 0; k < nchar; k++ )
	//	activeChar[k] = true;
		
	IntSet::const_iterator i;
	for( i = exset.begin(); i != exset.end(); i++ ) {
		k = charPos[*i];
		if( k < 0 ) continue;
		
		// k greater than -1 means character was not eliminated
		// and therefore can be excluded
		//
		if( activeChar[k] == true )
			num_excluded++;
		activeChar[k] = false;
	}
	return num_excluded;
}

/**
 * @method ApplyIncludeset [void:protected]
 * @param inset [IntSet&] set of character indices to include in range [0..ncharTotal)
 *
 * Includes characters whose indices are contained in the set inset. The indices supplied 
 * should refer to the original character indices, not current character indices.
 */
int CharactersBlock::ApplyIncludeset( IntSet& inset )
{
	assert( activeChar != NULL );
	int num_included = 0;
	int k;
	//for( k = 0; k < nchar; k++ )
	//	activeChar[k] = true;
		
	IntSet::const_iterator i;
	for( i = inset.begin(); i != inset.end(); i++ ) {
		k = charPos[*i];
		if( k < 0 ) continue;
		
		// k greater than -1 means character was not eliminated
		// and therefore can be excluded
		//
		if( activeChar[k] == false )
			num_included++;
		activeChar[k] = true;
	}
	return num_included;
}

/**
 * @method ApplyRestoreset [void:protected]
 * @param restoreset [IntSet&] set of taxon indices to restore in range [0..ntaxTotal)
 *
 * Restores (i.e., includes in further analyses) taxa
 * whose indices are contained in the set restoreset.  The
 * taxon indices refer to original taxon indices, not current
 * indices (originals will equal current ones iff number of 
 * taxa in TAXA block equals number of taxa in MATRIX command).
 */
int CharactersBlock::ApplyRestoreset( IntSet& restoreset )
{
	assert( activeTaxon != NULL );
	int num_restored = 0;
	int k;
	//for( k = 0; k < ntax; k++ )
	//	activeTaxon[k] = true;
		
	IntSet::const_iterator i;
	for( i = restoreset.begin(); i != restoreset.end(); i++ ) {
		k = taxonPos[*i];
		if( k < 0 ) continue;
		
		// k greater than -1 means data was supplied for
		// this taxon and therefore it can be restored
		//
		if( activeTaxon[k] == false )
			num_restored++;
		activeTaxon[k] = true;
	}
	return num_restored;
}

/**
 * @method BuildCharPosArray [void:protected]
 * @param check_eliminated [void] if true, eliminated set has something in it and should be consulted (default is false)
 *
 * Use to allocate memory for (and initialize) charPos array, which keeps track 
 * of the original character index in cases where characters have been eliminated.
 * This function is called by HandleEliminate in response to encountering an ELIMINATE
 * command in the data file, and this is probably the only place where BuildCharPosArray
 * should be called with check_eliminated = true.  BuildCharPosArray is also called
 * in HandleMatrix, HandleCharstatelabels, HandleStatelabels, and HandleCharlabels.
 */
void CharactersBlock::BuildCharPosArray( bool check_eliminated /* = false */ )
{
	assert( charPos == NULL );
	
	charPos = new int[ncharTotal];
	
   int k = 0;
   for( int j = 0; j < ncharTotal; j++ ) {
		if( check_eliminated && IsEliminated(j) )
			charPos[j] = -1;
		else
			charPos[j] = k++;
	}
}

/**
 * @method CharLabelToNumber [int:protected]
 *
 * Converts a character label to a number corresponding to
 * the character's position within charLabels.
 * This method overrides the virtual function of the
 * same name in the NexusBlock base class.  If s is
 * not a valid character label, returns the value 0.
 */
int CharactersBlock::CharLabelToNumber( nxsstring s )
{
	LabelList::const_iterator iter
   	= find( charLabels.begin(), charLabels.end(), s );

   int k = 1;
   if( iter != charLabels.end() )
   	k += ( iter - charLabels.begin() );
   else
   	k = 0;

  return k;
}

/**
 * @method DebugShowMatrix [int:protected]
 * @param out [ostream&] output stream on which to print matrix
 * @param marginText [char*] nxsstring to print first on each line
 *
 * Provides a dump of the contents of the matrix variable.  Useful for testing
 * whether data is being read as expected.  The default for marginText is NULL,
 * which has the effect of placing the matrix output flush left.  If each line
 * of output should be prefaced with a tab character, specify marginText = "\t".
 */
void CharactersBlock::DebugShowMatrix( ostream& out, char* marginText /* = NULL */ )
{
	int i, k;
	int width = taxa.GetMaxTaxonLabelLength();

	for( i = 0; i < ntaxTotal; i++ )
	{
		// Grab taxon name from taxa block
		// Taxa may not have been presented in the matrix in the same order
		// as they were stored in the taxa block, so use taxonPos array to
		// spit them out in the order they appeared in the TAXA command.
		// If the taxonPos cell is -1, then that means there is no row of
		// the data matrix corresponding to that taxon
		//
		if( taxonPos[i] < 0 )
			continue;
		else
		{
			if( marginText != NULL )
				out << marginText;
			nxsstring currTaxonLabel = taxa.GetTaxonLabel( taxonPos[i] );
			out << currTaxonLabel;

			// print out enough spaces to even up the left edge of the matrix output
			int currTaxonLabelLen = currTaxonLabel.size();
			int diff = width - currTaxonLabelLen;
			for( k = 0; k < diff+5; k++ )
				out << ' ';
		}

		for( int currChar = 0; currChar < ncharTotal; currChar++ ) {
         int j = charPos[currChar];
         if( j < 0 ) continue;
			ShowStateLabels( out, i, j );
		}

		out << endl;
	}
}

/**
 * @method DeleteTaxon [void:protected]
 * @param i [int] index of taxon to delete in range [0..ntax)
 *
 * Deletes taxon whose 0-offset current index is i.  If taxon
 * has already been deleted, this function has no effect.
 */
void CharactersBlock::DeleteTaxon( int i )
{
	activeTaxon[i] = false;
}

/**
 * @method ExcludeCharacter [void:protected]
 * @param i [int] index of character to exclude in range [0..nchar)
 *
 * Excludes character whose 0-offset current index is i.  If character
 * has already been excluded, this function has no effect.
 */
void CharactersBlock::ExcludeCharacter( int i )
{
	activeChar[i] = false;
}

/**
 * @method GetActiveCharArray [bool*:public]
 *
 * Returns activeChar data member (pointer to first element of the activeChar array).
 * Access to this protected data member is necessary in certain circumstances, such as
 * when a CharactersBlock object is stored in another class, and that other class
 * needs direct access to the activeChar array even though it is not derived from 
 * CharactersBlock.
 */
bool* CharactersBlock::GetActiveCharArray()
{
	return activeChar;
}

/**
 * @method GetActiveTaxonArray [bool*:public]
 *
 * Returns activeTaxon data member (pointer to first element of the activeTaxon array).
 * Access to this protected data member is necessary in certain circumstances, such as
 * when a CharactersBlock object is stored in another class, and that other class
 * needs direct access to the activeTaxon array even though it is not derived from 
 * CharactersBlock.
 */
bool* CharactersBlock::GetActiveTaxonArray()
{
	return activeTaxon;
}

/**
 * @method GetCharLabel [nxsstring:public]
 * @param i [int] the character in range [0..nchar)
 *
 * Returns label for character i, if a label has been specified.
 * If no label was specified, returns string containing a single blank (i.e., " ").
 */
nxsstring CharactersBlock::GetCharLabel( int i )
{
	nxsstring s = " ";
   if( i < charLabels.size() )
		s = charLabels[i];
   return s;
}

/**
 * @method GetCharPos [int:public]
 * @param origCharIndex [int] original index of character in range [0..ncharTotal-1)
 *
 * Returns current index of character in matrix.  This may
 * differ from the original index if some characters were
 * removed using an ELIMINATE command.  For example, character
 * number 9 in the original data matrix may now be at position
 * 8 if the original character 8 was ELIMINATEd.
 * The parameter origCharIndex is assumed to range from 0 to
 * ncharTotal-1.
 */
int CharactersBlock::GetCharPos( int origCharIndex )
{
   assert( charPos != NULL );
   assert( origCharIndex >= 0 );
   assert( origCharIndex < ncharTotal );

   return charPos[origCharIndex];
}

/**
 * @method GetGapSymbol [char:protected]
 *
 * Returns the gap symbol currently in effect.  If no gap
 * symbol specified, returns '\0'.
 */
char CharactersBlock::GetGapSymbol()
{
	return gap;
}

/**
 * @method GetTaxPos [int:public]
 * @param origTaxonIndex [int] original index of taxon
 *
 * Returns current index of taxon in matrix.  This may
 * differ from the original index if some taxa were listed in
 * the TAXA block but not in the DATA or CHARACTERS block.
 * The parameter origTaxonIndex is assumed to range from 0 to
 * ntaxTotal-1.
 */
int CharactersBlock::GetTaxPos( int origTaxonIndex )
{
   assert( taxonPos != NULL );
   assert( origTaxonIndex >= 0 );
   assert( origTaxonIndex < ntaxTotal );

   return taxonPos[origTaxonIndex];
}

/**
 * @method GetDataType [int:public]
 *
 * Returns value of datatype.
 */
int CharactersBlock::GetDataType()
{
	return (int)datatype;
}

/**
 * @method GetMatchcharSymbol [char:protected]
 *
 * Returns the matchchar symbol currently in effect.  If no matchchar
 * symbol specified, returns '\0'.
 */
char CharactersBlock::GetMatchcharSymbol()
{
	return matchchar;
}

/**
 * @method GetMaxObsNumStates [int:public virtual]
 *
 * Returns the maximum observed number of states for any character.
 * Note: this function is rather slow, as it must walk through 
 * each row of each column, adding the states encountered to a set, 
 * then finally returning the size of the set.  Thus, if this 
 * function is called often, it would be advisable to initialize 
 * an array using this function, then refer to the array subsequently.
 */
int CharactersBlock::GetMaxObsNumStates()
{
   int max = 2;
   for( int j = 0; j < nchar; j++ ) {
      int ns = GetObsNumStates(j);
      if( ns <= max ) continue;
      max = ns;
   }
   return max;
}

/**
 * @method GetInternalRepresentation [int:protected]
 * @param i [int] the taxon in range [0..ntax)
 * @param j [int] the character in range [0..nchar)
 * @param k [int] the state to return (use default of 0 if only one state present)
 *
 * Returns internal representation of the state for taxon i, character j.
 * The default for k is 0, which corresponds to the normal
 * situation in which there is only one state with no uncertainty or polymorphism.
 * If there are multiple states, specify a number in the range [0..n) where
 * n is the number of states returned by the GetNumStates function.  Use the
 * IsPolymorphic function to determine whether the multiple states correspond to
 * uncertainty in state assignment or polymorphism in the taxon.
 *
 * <p> The value returned from this function is one of the following:
 * <ul>
 * <li> -3 means gap state (see note below)
 * <li> -2 means missing state (see note below)
 * <li> an integer 0 or greater is internal representation of a state
 * </ul>
 * Note: gap and missing states are actually represented internally in a
 * different way; for a description of the actual internal representation of
 * states, see <a href="DiscreteDatum.html">DiscreteDatum</a>
 */
int CharactersBlock::GetInternalRepresentation( int i, int j, int k /* = 0 */ )
{
   if( IsGapState( i, j ) )
      return -3;
   else if( IsMissingState( i, j ) )
      return -2;
   else
   	return matrix->GetState( i, j, k );
}

/**
 * @method GetMissingSymbol [char:protected]
 *
 * Returns the missing data symbol currently in effect.  If no missing
 * data symbol specified, returns '\0'.
 */
char CharactersBlock::GetMissingSymbol()
{
	return missing;
}

/**
 * @method GetNChar [int:public]
 *
 * Returns the value of nchar.
 */
int CharactersBlock::GetNChar()
{
	return nchar;
}

/**
 * @method GetNCharTotal [int:public]
 *
 * Returns the value of ncharTotal.
 */
int CharactersBlock::GetNCharTotal()
{
	return ncharTotal;
}

/**
 * @method GetNTax [int:public]
 *
 * Returns the value of ntax.
 */
int CharactersBlock::GetNTax()
{
	return ntax;
}

/**
 * @method GetNTaxTotal [int:public]
 *
 * Returns the value of ntaxTotal.
 */
int CharactersBlock::GetNTaxTotal()
{
	return ntaxTotal;
}

/**
 * @method GetNumActiveChar [int:public]
 *
 * Performs a count of the number of characters for which activeChar 
 * array reports true.
 */
int CharactersBlock::GetNumActiveChar()
{
	int num_active_char = 0;
	for( int i = 0; i < nchar; i++ ) {
		if( activeChar[i] == false )
			continue;
		num_active_char++;
	}
	return num_active_char;
}

/**
 * @method GetNumActiveTaxa [int:public]
 *
 * Performs a count of the number of taxa for which activeTaxon 
 * array reports true.
 */
int CharactersBlock::GetNumActiveTaxa()
{
	int num_active_taxa = 0;
	for( int i = 0; i < ntax; i++ ) {
		if( activeTaxon[i] == false )
			continue;
		num_active_taxa++;
	}
	return num_active_taxa;
}

/**
 * @method GetNumEliminated [int:protected]
 *
 * Returns the number of characters eliminated with the ELIMINATE command.
 */
int CharactersBlock::GetNumEliminated()
{
	return ( ncharTotal - nchar );
}

/**
 * @method GetNumEquates [int:protected]
 *
 * Returns the number of stored equate associations.
 */
int CharactersBlock::GetNumEquates()
{
	return equates.size();
}

/**
 * @method GetNumMatrixCols [int:public]
 *
 * Returns the number of actual columns in matrix.  This number is equal
 * to nchar, but can be smaller than ncharTotal since the user could have
 * ELIMINATEd some of the characters.
 */
int CharactersBlock::GetNumMatrixCols()
{
	return nchar;
}

/**
 * @method GetNumMatrixRows [int:public]
 *
 * Returns the number of actual rows in matrix.  This number is equal
 * to ntax, but can be smaller than ntaxTotal since the user did not
 * have to provide data for all taxa specified in the TAXA block.
 */
int CharactersBlock::GetNumMatrixRows()
{
	return ntax;
}

/**
 * @method GetNumStates [int:public]
 * @param i [int] the taxon in range [0..ntax)
 * @param j [int] the character in range [0..nchar)
 *
 * Returns the number of states for taxon i, character j.
 */
int CharactersBlock::GetNumStates( int i, int j )
{
   return matrix->GetNumStates( i, j );
}

/**
 * @method GetObsNumStates [int:public virtual]
 * @param j [int] the character in range [0..nchar)
 *
 * Returns the number of states for character j over all taxa.
 * Note: this function is rather slow, as it must walk through 
 * each row, adding the states encountered to a set, then finally  
 * returning the size of the set.  Thus, if this function is called 
 * often, it would be advisable to initialize an array using this 
 * function, then refer to the array subsequently.
 */
int CharactersBlock::GetObsNumStates( int j )
{
  	return matrix->GetObsNumStates( j );
}

/**
 * @method GetOrigCharIndex [int:public]
 * @param j [int] the character in range [0..nchar)
 *
 * Returns the original character index in the
 * range [0..ncharTotal).  Will be equal to j unless some
 * characters were ELIMINATEd.
 */
int CharactersBlock::GetOrigCharIndex( int j )
{
   int k = j;
   while( k < ncharTotal && charPos[k] < j )
      k++;

   assert( k < ncharTotal );
   
   return k;
}

/**
 * @method GetOrigCharNumber [int:public]
 * @param j [int] the character in range [0..nchar)
 *
 * Returns the original character number (used in the
 * NEXUS data file) in the range [1..ncharTotal].
 * Will be equal to j+1 unless some characters were ELIMINATEd.
 */
int CharactersBlock::GetOrigCharNumber( int j )
{
   return ( 1 + GetOrigCharIndex(j) );
}

/**
 * @method GetOrigTaxonIndex [int:public]
 * @param i [int] the taxon in range [0..ntax)
 *
 * Returns the original taxon index in the
 * range [0..ntaxTotal).  Will be equal to i unless 
 * data was not provided for some taxa listed in
 * a preceding TAXA block.
 */
int CharactersBlock::GetOrigTaxonIndex( int i )
{
   int k = i;
   while( k < ntaxTotal && taxonPos[k] < i )
      k++;

   assert( k < ntaxTotal );
   
   return k;
}

/**
 * @method GetOrigTaxonNumber [int:public]
 * @param i [int] the character in range [0..ntax)
 *
 * Returns the original taxon number (used in the
 * NEXUS data file) in the range [1..ntaxTotal].
 * Will be equal to i+1 unless data was not provided
 * for some taxa listed in a preceding TAXA block.
 */
int CharactersBlock::GetOrigTaxonNumber( int i )
{
   return ( 1 + GetOrigTaxonIndex(i) );
}

/**
 * @method GetState [char:public]
 * @param i [int] the taxon in range [0..ntax)
 * @param j [int] the character in range [0..nchar)
 * @param k [int] the state to return (use default of 0 if only one state present)
 *
 * Returns symbol from symbols list representing the state for taxon i
 * and character j.  The default for k is 0, which corresponds to the normal
 * situation in which there is only one state with no uncertainty or polymorphism.
 * If there are multiple states, specify a number in the range [0..n) where
 * n is the number of states returned by the GetNumStates function.  Use the
 * IsPolymorphic function to determine whether the multiple states correspond to
 * uncertainty in state assignment or polymorphism in the taxon.
 * Assumes symbols is not equal to NULL.
 */
char CharactersBlock::GetState( int i, int j, int k /* = 0 */ )
{
   assert( symbols != NULL );
   char state_char = '\0';

   int symbolsLen = strlen(symbols);
   int p = matrix->GetState( i, j, k );
   assert( p < symbolsLen );
   state_char = *(symbols + p);

   return state_char;
}

/**
 * @method GetStateLabel [nxsstring:public]
 * @param i [int] the locus in range [0..nchar)
 * @param j [int] the state
 *
 * Returns label for character state j at character i, if a label has been specified.
 * If no label was specified, returns string containing a single blank (i.e., " ").
 */
nxsstring CharactersBlock::GetStateLabel( int i, int j )
{
	nxsstring s = " ";
   LabelListBag::const_iterator cib = charStates.find(i);
   if( cib != charStates.end() && j < (*cib).second.size() )
		s = (*cib).second[j];
   return s;
}

/**
 * @method GetSymbols [char*:public]
 *
 * Returns data member symbols.  Warning: returned value may be NULL.
 */
char* CharactersBlock::GetSymbols()
{
	return symbols;
}

/**
 * @method GetTaxonLabel [nxsstring:public]
 * @param i [int] the taxon's position in the taxa block
 *
 * Returns label for taxon number i (i ranges from 0 to ntax-1).
 */
nxsstring CharactersBlock::GetTaxonLabel( int i )
{
   nxsstring s = taxa.GetTaxonLabel(i);
	return s;
}

/**
 * @method IncludeCharacter [void:protected]
 * @param i [int] index of character to include in range [0..nchar)
 *
 * Includes character whose 0-offset current index is i.  If character
 * is already active, this function has no effect.
 */
void CharactersBlock::IncludeCharacter( int i )
{
	activeChar[i] = true;
}

/**
 * @method IsActiveChar [bool:public]
 * @param j [int] the character in question, in the range [0..nchar)
 *
 * Returns true if character j is active. If character j has been
 * excluded, returns false.  Note that it is irrelevant whether or
 * not this character has been eliminated, since it is assumed that j
 * refers to a non-eliminated character!
 */
bool CharactersBlock::IsActiveChar( int j )
{
	assert( j >= 0 );
	assert( j < nchar );
	return activeChar[j];
}

/**
 * @method IsActiveTaxon [bool:public]
 * @param i [int] the taxon in question, in the range [0..ntax)
 *
 * Returns true if taxon i is active. If taxon i has been
 * deleted, returns false.
 */
bool CharactersBlock::IsActiveTaxon( int i )
{
	assert( i >= 0 );
	assert( i < ntax );
	return activeTaxon[i];
}

/**
 * @method IsDeleted [bool:public]
 * @param i [int] the taxon in question, in the range [0..ntax)
 *
 * Returns true if taxon number i has been deleted, false otherwise.
 */
bool CharactersBlock::IsDeleted( int i )
{
	return !IsActiveTaxon(i);
}

/**
 * @method IsEliminated [bool:public]
 * @param origCharIndex [int] the character in question
 *
 * Returns true if character number origCharIndex was ELIMINATEd, false otherwise.
 * Returns false immediately if "eliminated" set is empty.
 */
bool CharactersBlock::IsEliminated( int origCharIndex )
{
	// Note: it is tempting to try to streamline this method
	// by just looking up character j in charPos to see if it
	// has been eliminated, but this temptation should be
	// resisted since this function is used in setting up
	// charPos in the first place!

	if( eliminated.empty() )
		return false;

	IntSet::const_iterator found = eliminated.find(origCharIndex);
	if( found == eliminated.end() )
		return false;

	return true;
}

/**
 * @method IsExcluded [bool:public]
 * @param j [int] the character in question, in the range [0..nchar)
 *
 * Returns true if character j has been excluded. If character j is
 * active, returns false.  Note that it is irrelevant whether or
 * not this character has been eliminated, since it is assumed that j
 * refers to a non-eliminated character!
 */
bool CharactersBlock::IsExcluded( int j )
{
	return !IsActiveChar(j);
}

/**
 * @method IsGapState [bool:public]
 * @param i [int] the taxon, in range [0..ntax)
 * @param j [int] the character, in range [0..nchar)
 *
 * Returns 1 if the state at taxon i, character j is the gap state.
 */
bool CharactersBlock::IsGapState( int i, int j )
{
   return matrix->IsGap( i, j );
}

/**
 * @method IsInterleave [bool:public]
 *
 * Returns true if INTERLEAVE was specified in the FORMAT command, false otherwise.
 */
bool CharactersBlock::IsInterleave()
{
	return interleaving;
}

/**
 * @method IsLabels [bool:public]
 *
 * Returns true if LABELS was specified in the FORMAT command, false otherwise.
 */
bool CharactersBlock::IsLabels()
{
	return labels;
}

/**
 * @method IsMissingState [bool:public]
 * @param i [int] the taxon, in range [0..ntax)
 * @param j [int] the character, in range [0..nchar)
 *
 * Returns 1 if the state at taxon i, character j is the missing state.
 */
bool CharactersBlock::IsMissingState( int i, int j )
{
   return matrix->IsMissing( i, j );
}

/**
 * @method IsPolymorphic [bool:protected]
 * @param i [int] the taxon in range [0..ntax)
 * @param j [int] the character in range [0..nchar)
 *
 * Returns 1 if taxon i is polymorphic for character j, 0 otherwise.
 * Note that return value will be 0 if there is only one state (i.e.,
 * one cannot tell whether there is uncertainty using this function).
 */
bool CharactersBlock::IsPolymorphic( int i, int j )
{
   return matrix->IsPolymorphic( i, j );
}

/**
 * @method IsRespectCase [bool:public]
 *
 * Returns 1 if RESPECTCASE was specified in the FORMAT command, 0 otherwise.
 */
bool CharactersBlock::IsRespectCase()
{
	return respectingCase;
}

/**
 * @method IsTokens [bool:public]
 *
 * Returns true if TOKENS was specified in the FORMAT command, false otherwise.
 */
bool CharactersBlock::IsTokens()
{
	return tokens;
}

/**
 * @method IsTranspose [bool:public]
 *
 * Returns true if TRANSPOSE was specified in the FORMAT command, false otherwise.
 */
bool CharactersBlock::IsTranspose()
{
	return transposing;
}

/**
 * @method IsInSymbols [int:protected]
 * @param ch [int] the symbol character to search for
 *
 * Returns 1 if ch can be found in symbols array.  The value of respectingCase
 * is used to determine whether the search should be case sensitive or not.
 * Assumes symbols != NULL.
 */
int CharactersBlock::IsInSymbols( char ch )
{
   assert( symbols != NULL );
   int symbolsLength = strlen(symbols);
   int found = 0;
   for( int i = 0; i < symbolsLength; i++ )
   {
      char char_in_symbols = ( respectingCase ? symbols[i] : (char)toupper(symbols[i]) );
      char char_in_question = ( respectingCase ? ch : (char)toupper(ch) );
      if( char_in_symbols != char_in_question ) continue;
      found = 1;
      break;
   }
   return found;
}

/**
 * @method HandleCharlabels [void:protected]
 * @param token [NexusToken&] the token used to read from in
 * @throws XNexus
 *
 * Called when CHARLABELS command needs to be parsed from within the
 * DIMENSIONS block.  Deals with everything after the token CHARLABELS
 * up to and including the semicolon that terminates the CHARLABELS
 * command.  If an ELIMINATE command has been processed, labels
 * for eliminated characters will not be stored.
 */
void CharactersBlock::HandleCharlabels( NexusToken& token )
{
	int num_labels_read = 0;
	charLabels.erase( charLabels.begin(), charLabels.end() );
	
	if( charPos == NULL )
		BuildCharPosArray();
	
	for(;;)
	{
		token.GetNextToken();

		// token should either be ';' or the name of a character
		// (an isolated '_' character is converted automatically by
		// token.GetNextToken() into a space, which is then stored
		// as the character label)
		//
		if( token.Equals(";") ) {
			break;
		}
		else {
			num_labels_read++;
			
			// check to make sure user is not trying to read in more
			// character labels than there are characters
			//
			if( num_labels_read > (unsigned) ncharTotal ) {
				errormsg = "Number of character labels exceeds NCHAR specified in DIMENSIONS command";
				throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
			}

			if( !IsEliminated( num_labels_read - 1 ) )
				charLabels.push_back( token.GetToken() );
		}
	}

	newchar = false;
}

/**
 * @method HandleCharstatelabels [void:protected]
 * @param token [NexusToken&] the token used to read from in
 * @throws XNexus
 *
 * Called when CHARSTATELABELS command needs to be parsed from within the
 * CHARACTERS block.  Deals with everything after the token CHARSTATELABELS
 * up to and including the semicolon that terminates the CHARSTATELABELS
 * command.  Resulting charLabels vector will store labels only for
 * characters that have not been ELIMINATEd, and likewise for charStates.
 * Specifically, charStates[0] refers to the vector of character state
 * labels for the first non-eliminated character.
 */
void CharactersBlock::HandleCharstatelabels( NexusToken& token )
{
	int currChar = 0;
	bool semicolonFoundInInnerLoop = false;
	bool tokenAlreadyRead = false;
	bool save = true;

	charStates.erase( charStates.begin(), charStates.end() );
	charLabels.erase( charLabels.begin(), charLabels.end() );
	
	if( charPos == NULL )
		BuildCharPosArray();

	for(;;)
	{
		save = true;

		if( semicolonFoundInInnerLoop )
			break;

		if( tokenAlreadyRead )
			tokenAlreadyRead = false;
		else
			token.GetNextToken();

		if( token.Equals(";") )
			break;

		// token should be the character number; create a new association
		//
		int n = atoi( token.GetToken().c_str() );
		
		if( n < 1 || n > ncharTotal || n <= currChar ) {
			errormsg = "Invalid character number (";
         errormsg += token.GetToken();
         errormsg += ") found in CHARSTATELABELS command (either out of range or not interpretable as an integer)";
			throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
		}
		
      // If n is not the next character after currChar, need to add some dummy
      // labels to charLabels list
      //
      while( n - currChar > 1 ) 
      {
         currChar++;
         if( !IsEliminated( currChar - 1 ) )
         	charLabels.push_back(" ");
      }

		// If n refers to a character that has been ELIMINATEd, go through the motions of
		// reading in the information but don't actually save any of it
		//
      currChar++;
      assert( n == currChar );
		if( IsEliminated(currChar-1) )
			save = false;

		token.GetNextToken();

      // token should be the character label
      //
      if( save ) 
      	charLabels.push_back( token.GetToken() );

		token.GetNextToken();

      // token should be a slash character if state labels were provided
      // for this character; otherwise, token should be one of the following:
      // 1) the comma separating information for different characters, in 
      //    which case we read in the next token (which should be the next
      //    character number)
      // 2) the semicolon indicating the end of the command
      //
      if( !token.Equals("/") ) {
      	if( !token.Equals(",") && !token.Equals(";") ) {
				errormsg = "Expecting a comma or semicolon here, but found (";
	         errormsg += token.GetToken();
	         errormsg += ") instead";
				throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
      	}
      	if( token.Equals(",") )
	      	token.GetNextToken();
         tokenAlreadyRead = true;
         continue;
      }

      // now create a new association for the character states list

      for(;;)
      {
         token.GetNextToken();

         if( token.Equals(";") ) {
            semicolonFoundInInnerLoop = true;
            break;
         }

         if( token.Equals(",") ) {
            break;
         }

			if( save )
			{
	         // token should be a character state label; add it to the list
	         //
	         nxsstring cslabel = token.GetToken();
	         int k = GetCharPos(n-1);
	         charStates[k].push_back( cslabel );
			}
		
      } // inner loop (grabbing state labels for character n)

	 } // outer loop

   newchar = false;
}

/**
 * @method HandleDimensions [void:protected]
 * @param token [NexusToken&] the token used to read from in
 * @param newtaxaLabel [nxsstring] the label used in data file for newtaxa
 * @param ntaxLabel [nxsstring] the label used in data file for ntax
 * @param ncharLabel [nxsstring] the label used in data file for nchar
 * @throws XNexus
 *
 * Called when DIMENSIONS command needs to be parsed from within the
 * CHARACTERS block.  Deals with everything after the token DIMENSIONS
 * up to and including the semicolon that terminates the DIMENSIONs
 * command. newtaxaLabel, ntaxLabel and ncharLabel are simply "NEWTAXA",
 * "NTAX" and "NCHAR" for this class, but may be different for derived
 * classes that use newtaxa, ntax and nchar for other things (e.g.,
 * ntax is number of populations in an ALLELES block)
 */
void CharactersBlock::HandleDimensions( NexusToken& token
	, nxsstring newtaxaLabel, nxsstring ntaxLabel, nxsstring ncharLabel )
{
	for(;;)
	{
		token.GetNextToken();

		if( token.Equals(newtaxaLabel) ) {
			newtaxa = true;
			taxa.Reset();
		}
		else if( token.Equals(ntaxLabel) ) {

			// this should be the equals sign
			token.GetNextToken();
			if( !token.Equals("=") ) {
				errormsg = "Expecting '=' after ";
            errormsg += ntaxLabel;
            errormsg += " in DIMENSIONS command, but found ";
            errormsg += token.GetToken();
            errormsg += " instead";
				throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
			}

			// this should be the number of taxa
			token.GetNextToken();
			ntax = atoi( token.GetToken().c_str() );
			if( ntax <= 0 ) {
         	errormsg = ntaxLabel;
				errormsg += " must be a number greater than 0";
				throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
			}
			if( newtaxa )
				ntaxTotal = ntax;
			else {
            ntaxTotal = taxa.GetNumTaxonLabels();
            if( ntaxTotal < ntax ) {
            	errormsg = ntaxLabel;
					errormsg += " in ";
               errormsg += id;
               errormsg += " block must be less than or equal to NTAX in TAXA block";
               errormsg += "\nNote: one circumstance that can cause this error is ";
               errormsg += "\nforgetting to specify ";
               errormsg += ntaxLabel;
               errormsg += " in DIMENSIONS command when ";
               errormsg += "\na TAXA block has not been provided";
               throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
				}
			}
		}
		else if( token.Equals(ncharLabel) ) {

			// this should be the equals sign
			token.GetNextToken();
			if( !token.Equals("=") ) {
				errormsg = "Expecting '=' after ";
            errormsg += ncharLabel;
            errormsg += " in DIMENSIONS command, but found ";
            errormsg += token.GetToken();
            errormsg += " instead";
				throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
			}

			// this should be the number of characters
			token.GetNextToken();
			nchar = atoi( token.GetToken().c_str() );
			if( nchar <= 0 ) {
         	errormsg = ncharLabel;
				errormsg += " must be a number greater than 0";
				throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
			}

         ncharTotal = nchar;

		}
		else if( token.Equals(";") ) {
			break;
		}

	}
}

/**
 * @method HandleEliminate [void:protected]
 * @param token [NexusToken&] the token used to read from in
 *
 * Called when ELIMINATE command needs to be parsed from within the
 * CHARACTERS block.  Deals with everything after the token ELIMINATE
 * up to and including the semicolon that terminates the ELIMINATE
 * command.  Any character numbers or ranges of character numbers
 * specified are stored in the IntSet eliminated, which remains empty
 * until an ELIMINATE command is processed.  Note that like all sets
 * the character ranges are adjusted so that their offset is 0.  For
 * example, given "eliminate 4-7;" in the data file, the eliminate
 * array would contain the values 3, 4, 5, and 6 (not 4, 5, 6, and 7).
 * It is assumed that the ELIMINATE command comes before character labels
 * and/or character state labels have been specified; an error message
 * is generated if the user attempts to use ELIMINATE after a CHARLABELS,
 * CHARSTATELABELS, or STATELABELS command.
 */
void CharactersBlock::HandleEliminate( NexusToken& token )
{
   // construct an object of type SetReader, then call its run function
   // to store the set in the eliminated set
   //
	SetReader( token, ncharTotal, eliminated, *this, SetReader::charset ).Run();

   nchar = ncharTotal - eliminated.size();
   
   if( nchar != ncharTotal && ( charLabels.size() > 0 || charStates.size() > 0 ) ) 
   {
		errormsg = "The ELIMINATE command must appear before character\n(or character state) labels are specified";
		throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
	}

	if( charPos != NULL ) 
	{
		errormsg = "Only one ELIMINATE command is allowed, and it must appear before the MATRIX command";
		throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
	}

	BuildCharPosArray( true );
}

/**
 * @method HandleEndblock [void:protected]
 * @param token [NexusToken&] the token used to read from in
 * @throws XNexus
 *
 * Called when the END or ENDBLOCK command needs to be parsed from within the
 * CHARACTERS block.  Does two things:  1) checks to make sure the next token in
 * the data file is a semicolon and 2) eliminates character labels and character
 * state labels for characters that have been ELIMINATEd.
 */
void CharactersBlock::HandleEndblock( NexusToken& token, nxsstring charToken )
{
	// get the semicolon following END or ENDBLOCK token
	token.GetNextToken();
	if( !token.Equals(";") ) {
		errormsg = "Expecting ';' to terminate the END or ENDBLOCK command, but found ";
      errormsg += token.GetToken();
      errormsg += " instead";
		throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
	}

   if( charLabels.empty() && !charStates.empty() ) {
      // make up labels for characters since user has provided labels
      // for character states; that way, we know that charLabels
      // and charStates are either both empty or both full
      for( int k = 0; k < ncharTotal; k++ )
      {
         nxsstring nm = charToken;
         nm += " ";
         nm += (k+1);
         charLabels.push_back( nm.c_str() );
      }
   }

#if 0
	// This part is not necessary now that ELIMINATE is restricted to
	// being used before CHARLABELS, CHARSTATELABELS, and STATELABELS
	//
   if( nchar < ncharTotal && !charLabels.empty() ) 
   {
      // first, make copies of charLabels and charStates
      LabelList tmp_charLabels = charLabels;
      LabelListBag tmp_charStates = charStates;

      // second, erase contents of charLabels and charStates
      charLabels.erase( charLabels.begin(), charLabels.end() );
      charStates.erase( charStates.begin(), charStates.end() );

      // third, copy only information from non-ELIMINATEd characters back
      // into charLabels and charStates
      for( int k = 0; k < ncharTotal; k++ ) {
         int j = charPos[k];
         if( j < 0 ) continue;
         charLabels.push_back( tmp_charLabels[k] );
         int n = tmp_charStates[k].size();
         for( int kk = 0; kk < n; kk++ )
            charStates[j].push_back( tmp_charStates[k][kk] );
      }
   }
#endif
}

/**
 * @method HandleFormat [void:protected]
 * @param token [NexusToken&] the token used to read from in
 * @throws XNexus
 *
 * Called when FORMAT command needs to be parsed from within the
 * DIMENSIONS block.  Deals with everything after the token FORMAT
 * up to and including the semicolon that terminates the FORMAT
 * command.
 */
void CharactersBlock::HandleFormat( NexusToken& token )
{
	int standardDataTypeAssumed = 0;
	int ignoreCaseAssumed = 0;

	for(;;)
	{
		token.GetNextToken();

		if( token.Equals("DATATYPE") )
		{

			// this should be an equals sign
			token.GetNextToken();
			if( !token.Equals("=") ) {
				errormsg = "Expecting '=' after keyword DATATYPE but found ";
            errormsg += token.GetToken();
            errormsg += " instead";
				throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
			}

			// this should be one of the following: STANDARD, DNA, RNA, NUCLEOTIDE, PROTEIN, or CONTINUOUS
			token.GetNextToken();
			if( token.Equals("STANDARD") )
				datatype = standard;
			else if( token.Equals("DNA") )
				datatype = dna;
			else if( token.Equals("RNA") )
				datatype = rna;
			else if( token.Equals("NUCLEOTIDE") )
				datatype = nucleotide;
			else if( token.Equals("PROTEIN") )
				datatype = protein;
			else if( token.Equals("CONTINUOUS") )
				datatype = continuous;
			else {
				errormsg = token.GetToken();
            errormsg += " is not a valid DATATYPE within a ";
            errormsg += id;
            errormsg += " block";
				throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
			}

			if( standardDataTypeAssumed && datatype != standard ) {
				errormsg = "DATATYPE must be specified first in FORMAT command";
				throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
			}

			ResetSymbols();
			if( datatype == continuous )
				tokens = true;

		}

		else if( token.Equals("RESPECTCASE") )
		{
			if( ignoreCaseAssumed ) {
				errormsg = "RESPECTCASE must be specified before MISSING, GAP, SYMBOLS, and MATCHCHAR in FORMAT command";
				throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
			}
			standardDataTypeAssumed = 1;
			respectingCase = true;
		}

		else if( token.Equals("MISSING") )
		{

			// this should be an equals sign
			token.GetNextToken();
			if( !token.Equals("=") ) {
				errormsg = "Expecting '=' after keyword MISSING but found ";
            errormsg += token.GetToken();
            errormsg += " instead";
				throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
			}

			// this should be the missing data symbol (single character)
			token.GetNextToken();
			if( token.GetTokenLength() != 1 ) {
				errormsg = "MISSING symbol should be a single character, but ";
            errormsg += token.GetToken();
            errormsg += " was specified";
				throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
			}
			else if( token.IsPunctuationToken() && !token.IsPlusMinusToken() ) {
				errormsg = "MISSING symbol specified cannot be a punctuation token (";
            errormsg += token.GetToken();
            errormsg += " was specified)";
				throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
			}
			else if( token.IsWhitespaceToken() ) {
				errormsg = "MISSING symbol specified cannot be a whitespace character (";
            errormsg += token.GetToken();
            errormsg += " was specified)";
				throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
			}

			missing = token.GetToken()[0];

			ignoreCaseAssumed = 1;
			standardDataTypeAssumed = 1;
		}

		else if( token.Equals("GAP") )
		{

			// this should be an equals sign
			token.GetNextToken();
			if( !token.Equals("=") ) {
				errormsg = "Expecting '=' after keyword GAP but found ";
            errormsg += token.GetToken();
            errormsg += " instead";
				throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
			}

			// this should be the gap symbol (single character)
			token.GetNextToken();
			if( token.GetTokenLength() != 1 ) {
				errormsg = "GAP symbol should be a single character, but ";
            errormsg += token.GetToken();
            errormsg += " was specified";
				throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
			}
			else if( token.IsPunctuationToken() && !token.IsPlusMinusToken() ) {
				errormsg = "GAP symbol specified cannot be a punctuation token (";
            errormsg += token.GetToken();
            errormsg += " was specified)";
				throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
			}
			else if( token.IsWhitespaceToken() ) {
				errormsg = "GAP symbol specified cannot be a whitespace character (";
            errormsg += token.GetToken();
            errormsg += " was specified)";
				throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
			}

			gap = token.GetToken()[0];

			ignoreCaseAssumed = 1;
			standardDataTypeAssumed = 1;
		}

		else if( token.Equals("SYMBOLS") )
		{
			if( datatype == continuous ) {
				errormsg = "SYMBOLS subcommand not allowed for DATATYPE=CONTINUOUS";
				throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
			}

			int numDefStates;
			int maxNewStates;
			switch( datatype )
			{
				case dna:
				case rna:
				case nucleotide:
					numDefStates = 4;
					maxNewStates = NCL_MAX_STATES-4;
					break;
				case protein:
					numDefStates = 21;
					maxNewStates = NCL_MAX_STATES-21;
					break;
				default:
					numDefStates = 0; // replace symbols list for standard datatype
               symbols[0] = '\0';
					maxNewStates = NCL_MAX_STATES;
			}

			// this should be an equals sign
			token.GetNextToken();
			if( !token.Equals("=") ) {
				errormsg = "Expecting '=' after keyword SYMBOLS but found ";
            errormsg += token.GetToken();
            errormsg += " instead";
				throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
			}

			// this should be the symbols list
			token.SetLabileFlagBit( NexusToken::doubleQuotedToken );
			token.GetNextToken();
			token.StripWhitespace();
			int numNewSymbols = token.GetTokenLength();
			if( numNewSymbols > maxNewStates ) {
				errormsg = "SYMBOLS defines ";
            errormsg += numNewSymbols;
            errormsg += " new states but only ";
            errormsg += maxNewStates;
            errormsg += " new states allowed for this DATATYPE";
				throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
			}

			nxsstring t = token.GetToken();
			int tlen = t.size();

			// check to make sure user has not used any symbols already in the
			// default symbols list for this data type
			for( int i = 0; i < tlen; i++ )
         {
				if( IsInSymbols( t[i] ) ) {
					errormsg = "The character ";
               errormsg += t[i];
               errormsg += " defined in SYMBOLS has already been predefined for this DATATYPE";
					throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
				}
			}

			// if we've made it this far, go ahead and add the user-defined
			// symbols to the end of the list of predefined symbols
			strcpy( symbols+numDefStates, t.c_str() );

			ignoreCaseAssumed = 1;
			standardDataTypeAssumed = 1;
		}

		else if( token.Equals("EQUATE") )
		{

			// this should be an equals sign
			token.GetNextToken();
			if( !token.Equals("=") ) {
				errormsg = "Expecting '=' after keyword EQUATE but found ";
            errormsg += token.GetToken();
            errormsg += " instead";
				throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
			}

			// this should be a double-quote character
			token.GetNextToken();
			if( !token.Equals("\"") ) {
				errormsg = "Expecting '\"' after keyword EQUATE but found ";
            errormsg += token.GetToken();
            errormsg += " instead";
				throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
			}

         // loop until second double-quote character is encountered
         for(;;)
			{
   			token.GetNextToken();
   			if( token.Equals("\"") )
               break;

            // if token is not a double-quote character, then it must be
            // the equate symbol (i.e., the character to be replaced in
            // the data matrix)
				if( token.GetTokenLength() != 1 ) {
               errormsg = "Expecting single-character EQUATE symbol but found ";
               errormsg += token.GetToken();
               errormsg += " instead";
               throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
            }

            // check for bad choice of equate symbol
            nxsstring t = token.GetToken();
            char ch = t[0];
            int badEquateSymbol = 0;
            // the character '^' cannot be an equate symbol
            if( ch == '^' )
               badEquateSymbol = 1;
            // equate symbols cannot be punctuation (except for + and -)
            if( token.IsPunctuationToken() && !token.IsPlusMinusToken() )
               badEquateSymbol = 1;
            // equate symbols cannot be same as matchchar, missing, or gap
				if( ch == missing || ch == matchchar || ch == gap )
               badEquateSymbol = 1;
            // equate symbols cannot be one of the state symbols currently defined
            if( IsInSymbols(ch) )
               badEquateSymbol = 1;
            if( badEquateSymbol ) {
               errormsg = "EQUATE symbol specified (";
               errormsg += token.GetToken();
               errormsg += ") is not valid; must not be same as missing, \nmatchchar, gap, state symbols, or any of the following: ()[]{}/\\,;:=*'\"`<>^";
               throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
            }
            nxsstring k = token.GetToken();

            // this should be an equals sign
				token.GetNextToken();
            if( !token.Equals("=") ) {
               errormsg = "Expecting '=' in EQUATE definition but found ";
               errormsg += token.GetToken();
               errormsg += " instead";
               throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
            }

            // this should be the token to be substituted in for the equate symbol
            token.SetLabileFlagBit( NexusToken::parentheticalToken );
            token.SetLabileFlagBit( NexusToken::curlyBracketedToken );
            token.GetNextToken();
            nxsstring v = token.GetToken();


            // add the new equate association to the equates list
            equates[k] = v;
			}

			standardDataTypeAssumed = 1;
		}

		else if( token.Equals("MATCHCHAR") )
		{

			// this should be an equals sign
			token.GetNextToken();
			if( !token.Equals("=") ) {
				errormsg = "Expecting '=' after keyword MATCHCHAR but found ";
            errormsg += token.GetToken();
            errormsg += " instead";
				throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
			}

			// this should be the matchchar symbol (single character)
			token.GetNextToken();
			if( token.GetTokenLength() != 1 ) {
				errormsg = "MATCHCHAR symbol should be a single character, but ";
            errormsg += token.GetToken();
            errormsg += " was specified";
				throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
			}
			else if( token.IsPunctuationToken() && !token.IsPlusMinusToken() ) {
				errormsg = "MATCHCHAR symbol specified cannot be a punctuation token (";
            errormsg += token.GetToken();
            errormsg += " was specified) ";
				throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
			}
			else if( token.IsWhitespaceToken() ) {
				errormsg = "MATCHCHAR symbol specified cannot be a whitespace character (";
            errormsg += token.GetToken();
            errormsg += " was specified)";
				throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
			}

			matchchar = token.GetToken()[0];

			ignoreCaseAssumed = 1;
			standardDataTypeAssumed = 1;
		}

		else if( token.Equals("LABELS") )
		{
			labels = true;
			standardDataTypeAssumed = 1;
		}

		else if( token.Equals("NOLABELS") )
		{
			labels = false;
			standardDataTypeAssumed = 1;
		}

		else if( token.Equals("TRANSPOSE") )
		{
			transposing = true;
			standardDataTypeAssumed = 1;
		}

		else if( token.Equals("INTERLEAVE") )
		{
			interleaving = true;
			standardDataTypeAssumed = 1;
		}

		else if( token.Equals("ITEMS") )
		{

			// this should be an equals sign
			token.GetNextToken();
			if( !token.Equals("=") ) {
				errormsg += "Expecting '=' after keyword ITEMS but found ";
            errormsg += token.GetToken();
            errormsg += " instead";
				throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
			}

			// this should be STATES (no other item is supported at this time)
			token.GetNextToken();
			if( !token.Equals("STATES") ) {
				errormsg = "Sorry, only ITEMS=STATES supported at this time";
				throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
			}

			standardDataTypeAssumed = 1;
		}

		else if( token.Equals("STATESFORMAT") )
		{

			// this should be an equals sign
			token.GetNextToken();
			if( !token.Equals("=") ) {
				errormsg = "Expecting '=' after keyword STATESFORMAT but found ";
            errormsg += token.GetToken();
            errormsg += " instead";
				throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
			}

			// this should be STATESPRESENT (no other statesformat is supported at this time)
			token.GetNextToken();
			if( !token.Equals("STATESPRESENT") ) {
				errormsg = "Sorry, only STATESFORMAT=STATESPRESENT supported at this time";
				throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
			}

			standardDataTypeAssumed = 1;
		}

		else if( token.Equals("TOKENS") )
		{
			tokens = true;
			standardDataTypeAssumed = 1;
		}

		else if( token.Equals("NOTOKENS") )
		{
			tokens = false;
			standardDataTypeAssumed = 1;
		}

		else if( token.Equals(";") ) {
			break;
		}
	}

	// perform some last checks before leaving the FORMAT command
	if( !tokens && datatype == continuous ) {
		errormsg = "TOKENS must be defined for DATATYPE=CONTINUOUS";
		throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
	}

	if( tokens && ( datatype == dna || datatype == rna || datatype == nucleotide ) ) {
		errormsg = "TOKENS not allowed for the DATATYPEs DNA, RNA, or NUCLEOTIDE";
		throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
	}
}

/**
 * @method HandleNextState [bool:protected]
 * @param token [NexusToken&] the token used to read from in
 * @param i [int] the taxon index, in range [0..ntax)
 * @param j [int] the character index, in range [0..nchar)
 * @throws XNexus
 *
 * Called from HandleStdMatrix or HandleTransposedMatrix function
 * to read in the next state.  Always returns 1 except in the special
 * case of an interleaved matrix, in which case it returns 0 if a
 * newline character is encountered before the next token.
 */
bool CharactersBlock::HandleNextState( NexusToken& token, int i, int j )
{
	// this should be the state for taxon i and character j
	//
	if( !tokens ) {
		token.SetLabileFlagBit( NexusToken::parentheticalToken );
		token.SetLabileFlagBit( NexusToken::curlyBracketedToken );
		token.SetLabileFlagBit( NexusToken::singleCharacterToken );
	}
	if( interleaving )
		token.SetLabileFlagBit( NexusToken::newlineIsToken );
	token.GetNextToken();

	if( interleaving && token.AtEOL() )
		return false;

	// make sure we didn't run out of file
	//
	if( token.AtEOF() ) {
		errormsg = "Unexpected end of file encountered";
		throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
	}

	// if we didn't run out of file, I see no reason why we should
	// have a zero-length token on our hands
	assert( token.GetTokenLength() > 0 );

	// we've read in the state now, so if this character has been
	// ELIMINATEd, we don't want to go any further with it
	//
	if( j < 0 ) return true;

   // see if any equate macros apply
   //
   nxsstring skey = nxsstring( token.GetToken( respectingCase ) );
   AssocList::iterator p = equates.find( skey );
   if( p != equates.end() ) {
      nxsstring sval = (*p).second;
      token.ReplaceToken( sval.c_str() );
   }

	// handle case of single-character state symbol
	//
	if( !tokens && token.GetTokenLength() == 1 )
	{
		char ch = token.GetToken()[0];

		// check for missing data symbol
		//
		if( ch == missing ) {
			matrix->SetMissing( i, j );
		}

		// check for matchchar symbol
		//
		else if( matchchar != '\0' && ch == matchchar ) {
#if 0
			int maxk = matrix->GetNumStates( i, j );
			for( int k = 0; k < maxk; k++ ) {
				int stateOfFirstTaxon = matrix->GetState( 0, j, k );
				matrix->AddState( i, j, stateOfFirstTaxon );
			}
#else
			matrix->CopyStatesFromFirstTaxon( i, j );
#endif
		}

		// check for gap symbol
		//
		else if( gap != '\0' && ch == gap ) {
			matrix->SetGap( i, j );
		}

		// look up the position of this state in the symbols array
		//
		else {
			int p = PositionInSymbols(ch);
			if( p < 0 ) {
				errormsg = "State specified (";
	            errormsg += token.GetToken();
	            errormsg += ") for taxon ";
	            errormsg += (i+1);
	            errormsg += ", character ";
	            errormsg += (j+1);
	            errormsg += ", not found in list of valid symbols";
				throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
			}
			matrix->AddState( i, j, p );
			matrix->SetPolymorphic( i, j, 0 );
		}
	}

	// handle case of state sets when tokens is not in effect
	//
	else if( !tokens  && token.GetTokenLength() > 1 )
	{
		// token should be in one of the following forms: {acg} {a~g} {a c g} (acg) (a~g) (a c g) 
		nxsstring t = token.GetToken();
		int tlen = t.size();
		int poly = ( t[0] == '(' );
		assert( poly || t[0] == '{' );
		assert( ( poly && t[tlen-1] == ')' ) || ( !poly && t[tlen-1] == '}' ) );
		
		int first_nonblank = 1;
		while( t[first_nonblank] == ' ' || t[first_nonblank] == '\t' )
			first_nonblank++;

		int last_nonblank = tlen-2;
		while( t[last_nonblank] == ' ' || t[last_nonblank] == '\t' )
			last_nonblank--;

		if( t[first_nonblank] == '~' || t[last_nonblank] == '~' ) {
         errormsg = token.GetToken();
         errormsg += " does not represent a valid range of states";
			throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
		}

		int k = 1;
		char* pFirst = symbols;
		int tildeFound = 0;
		for(;;)
		{
			if( t[k] == ')' || t[k] == '}' )
				break;
				
			if( t[k] == ' ' || t[k] == '\t' ) {
				k++;
				continue;
			}

			// t[k] should be either '~' or one of the state symbols
			if( t[k] == '~' ) {
				tildeFound = 1;
			}
			else
			{
				// add state symbol and record if it is the first or last one
				// in case we encounter a tilde
				//
				if( tildeFound )
				{
					// add all states from firstState to t[k]
					// then set tildeFound to 0 again
					pFirst++;
					while( *pFirst != '\0' && *pFirst != t[k] )
               {
                  int p = PositionInSymbols(*pFirst);
                  if( p < 0 ) {
                     errormsg = "State specified (";
                     errormsg += *pFirst;
                     errormsg += ") for taxon ";
                     errormsg += (i+1);
                     errormsg += ", character ";
                     errormsg += (j+1);
                     errormsg += ", not found in list of valid symbols";
                     throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
                  }
						matrix->AddState( i, j, p );
   					pFirst++;
               }

					tildeFound = 0;
				}
				else
				{
               int p = PositionInSymbols(t[k]);
					if( p < 0 ) {
						errormsg = "State specified (";
                  errormsg += t[k];
                  errormsg += ") for taxon ";
                  errormsg += (i+1);
                  errormsg += ", character ";
                  errormsg += (j+1);
                  errormsg += ", not found in list of valid symbols";
						throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
					}
					pFirst = ( symbols + p );
					matrix->AddState( i, j, p );
				}

			} // if( t[k] == '~' ) ... else ... loop

			k++;
		} // for(;;) loop

		matrix->SetPolymorphic( i, j, poly );
	}

	// handle case in which TOKENS was specified in the FORMAT command
	//
	else
	{
		// token should be in one of the following forms: "{"  "a"  "bb"
		int polymorphism = token.Equals("(");
		int uncertainty  = token.Equals("{");

		if( !uncertainty && !polymorphism ) {
			int k = HandleTokenState( token, j );
			matrix->AddState( i, j, k );
		}
		else {
         int tildeFound = 0;
         int first = -1;
         int last;
			for(;;)
			{
				// OPEN ISSUE: What about newlines if interleaving? I'm assuming
				// that the newline must come between characters to count.

				token.SetLabileFlagBit( NexusToken::tildeIsPunctuation );
				token.GetNextToken();

				if( polymorphism && token.Equals(")") ) {
					if( tildeFound ) {
						errormsg = "Range of states still being specified when ')' encountered";
						throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
					}
					break;
				}

				else if( uncertainty && token.Equals("}") ) {
					if( tildeFound ) {
						errormsg = "Range of states still being specified when '}' encountered";
						throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
					}
					break;
				}

				else if( token.Equals("~") ) {
					if( first == -1 ) {
						errormsg = "Tilde character ('~') cannot precede token indicating beginning of range";
						throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
					}
					tildeFound = 1;
				}

				else if( tildeFound ) {
					// Add all states from first+1 to last, then reset tildeFound to 0
               //
					last = HandleTokenState( token, j );

					if( last <= first) {
						errormsg = "Last state in specified range (";
                  errormsg += token.GetToken();
                  errormsg += ") must be greater than the first";
						throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
					}

					for( int k = first+1; k <= last; k++ )
               	matrix->AddState( i, j, k );

					tildeFound = 0;
               first = -1;
				}

				else {
               // Add current state, then set first to that state's value
               // State's value is its position within the list of states
               // for that character
               //
					first = HandleTokenState( token, j );
					matrix->AddState( i, j, first );
				}

			}

			if( polymorphism )
				matrix->SetPolymorphic( i, j, 1 );
		}
	}

	return true;
}

/**
 * @method HandleTokenState [int:protected]
 * @param token [NexusToken&] the token used to read from in
 * @param j [int] the character index, in range [0..nchar)
 * @throws XNexus
 *
 * Called from HandleNextState to read in the next state when 'tokens' is in effect.
 * Looks up state in character states listed for the character to make
 * sure it is a valid state, and returns state's value (0, 1, 2, ...).
 * Note: does NOT handle adding the state's value to matrix. Save the return
 * value, let's call it k, and use the following command to add it to matrix:
 * matrix->AddState( i, j, k );
 */
int CharactersBlock::HandleTokenState( NexusToken& token, int j )
{
	// token should be one of the character states listed for character j
	// in charStates
	//
   if( charStates.find(j) == charStates.end() ) {
		errormsg = "No states were defined for character ";
      errormsg += ( 1 + GetOrigCharIndex(j) );
		throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
	}

   // TO DO: this section is very UGLY - need to find some cleaner way of comparing
   // the token nxsstring to the strings representing valid characters states
   // in the LabelList associated with character j
   //
	LabelListBag::const_iterator bagIter = charStates.find(j);
   LabelList::const_iterator ci_begin = (*bagIter).second.begin();
   LabelList::const_iterator ci_end = (*bagIter).second.end();
	nxsstring t = token.GetToken( respectingCase );
   LabelList::const_iterator cit;
   if( respectingCase )
      cit = find( ci_begin, ci_end, t );
   else
      cit = find_if( ci_begin, ci_end, bind2nd( stri_equal(), t ) );

	if( cit == ci_end ) {
		errormsg = "Character state ";
      errormsg += t;
      errormsg += " not defined for character ";
      errormsg += ( 1 + GetOrigCharIndex(j) );
		throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
	}

	// ok, the state has been identified, so return the state's internal
	// representation. That is, if the list of state labels was
	// "small medium large" and "small" was specified in the data file,
	// state saved in matrix would be 0 (it would be 1 if "medium" were
	// specified in the data file, and 2 if "large" were specified in the
	// data file).
	int k = ( cit - ci_begin );
	return k;
}

/**
 * @method HandleStdMatrix [void:protected]
 * @param token [NexusToken&] the token used to read from in
 * @throws XNexus
 *
 * Called from HandleMatrix function to read in a standard
 * (i.e., non-transposed) matrix.  Interleaving, if applicable,
 * is dealt with herein.
 */
void CharactersBlock::HandleStdMatrix( NexusToken& token )
{
	int i, j, currChar;
	int firstChar = 0;
	int lastChar = ncharTotal;
	int nextFirst;
	int page = 0;

	for(;;)
	{
		//************************************************
		//******** Beginning of loop through taxa ********
		//************************************************

		for( i = 0; i < ntax; i++ )
		{
			if( labels )
			{
				// This should be the taxon label
				//
				token.GetNextToken();

				if( page == 0 && newtaxa )
				{
					// This section:
					// - labels provided
					// - on first (or only) interleave page
					// - no previous TAXA block
					
					// check for duplicate taxon names
					//
					if( taxa.IsAlreadyDefined( token.GetToken() ) ) {
						errormsg = "Data for this taxon (";
						errormsg += token.GetToken();
						errormsg += ") has already been saved";
						throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
					}

					// labels provided and not already stored in the taxa block with
					// the TAXLABELS command; taxa.Reset() and taxa.SetNTax() have
					// were already called, however, when the NTAX subcommand was
					// processed.
					//
					taxa.AddTaxonLabel( token.GetToken() );

					// order of occurrence in TAXA block same as row in matrix
					//
					taxonPos[i] = i;
				}
				else
				{
					// This section:
					// - labels provided
					// - TAXA block provided or has been created already
					// - may be on any (interleave) page					
					
					// Cannot assume taxon in same position in
					// taxa block. Set up taxonPos array so that we can look up
					// the correct row in matrix for any given taxon
					//
					int positionInTaxaBlock;
					try {
						positionInTaxaBlock = taxa.FindTaxon( token.GetToken() );
					}
					catch( out_of_range ) {
						errormsg = "Could not find taxon named ";
						errormsg += token.GetToken();
						errormsg += " among stored taxon labels";
						throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
					}

					if( page == 0 )
					{
						// make sure user has not duplicated data for a single taxon
						//
						if( taxonPos[positionInTaxaBlock] != -1 ) {
							errormsg = "Data for this taxon (";
							errormsg += token.GetToken();
							errormsg += ") has already been saved";
							throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
						}

						// make sure user has kept same relative ordering of taxa in both the TAXA
						// block and the CHARACTERS block
						//
						if( positionInTaxaBlock != i ) { //POL 11-12-99: was --> positionInTaxaBlock >= i
							errormsg = "Relative order of taxa must be the same in both the TAXA and CHARACTERS blocks";
							throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
						}

						taxonPos[i] = positionInTaxaBlock;  // was --> taxonPos[positionInTaxaBlock] = i;
					}
					else
					{
						// make sure user has kept the ordering of taxa the same from
						// one interleave page to the next
						//
						if( taxonPos[positionInTaxaBlock] != i ) {
							errormsg = "Ordering of taxa must be identical to that in first interleave page";
							throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
						}
					}
				}
			}
			else {
				// no labels provided, assume taxon position same as in taxa block
				if( page == 0 )
					taxonPos[i] = i;
			}

			//******************************************************
			//******** Beginning of loop through characters ********
			//******************************************************

			for( currChar = firstChar; currChar < lastChar; currChar++ )
			{
				// it is possible that character currChar has been ELIMINATEd, in
				// which case we need to go through the motions of reading in the
				// data but we don't store it.  The variable j will be our guide
				// when it comes time to store data since j will be -1 for
				// characters that were ELIMINATEd and will be set to the correct
				// row for characters that have not been ELIMINATEd.
				//
				j = charPos[currChar];

				// ok will be 0 only if a newline character is encountered before
				// character j processed
				//
				bool ok = HandleNextState( token, i, j );
				if( interleaving && !ok )
            {
					if( lastChar < ncharTotal && j != lastChar ) {
						errormsg = "Each line within an interleave page must comprise the same number of characters";
						throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
					}

					// currChar should be firstChar in next go around
					nextFirst = currChar;

					// set lastChar to currChar so that we can check to make sure the
					// remaining lines in this interleave page end at the same place
					lastChar = currChar;

					// since j is now equal to lastChar, we are done with this innermost loop
				}
			} // innermost loop (over characters)

		} // middle loop (over taxa)

		firstChar = nextFirst;
		lastChar = ncharTotal;

		// if currChar equals ncharTotal, then we've just finished reading the last
		// interleave page and thus should break from the outer loop
		// Note that if we are not interleaving, this will still work since
		// lastChar is initialized to ncharTotal and never changed
		//
		if( currChar == ncharTotal )
			break;

		page++;

	} // outer loop (over interleave pages)

}

/**
 * @method HandleTransposedMatrix [void:protected]
 * @param token [NexusToken&] the token used to read from in
 * @throws XNexus
 *
 * Called from HandleMatrix function to read in a transposed
 * matrix.  Interleaving, if applicable, is dealt with herein.
 */
void CharactersBlock::HandleTransposedMatrix( NexusToken& token )
{
	int i, j, currChar;
	int firstTaxon = 0;
	int lastTaxon = ntaxTotal;
	int nextFirst;
	int page = 0;

	for(;;)
	{
		//******************************************************
		//******** Beginning of loop through characters ********
		//******************************************************

		for( currChar = 0; currChar < ncharTotal; currChar++ )
		{
			// it is possible that character currChar has been ELIMINATEd, in
			// which case we need to go through the motions of reading in the
			// data but we don't store it.  The variable j will be our guide
			// when it comes time to store data since j will be -1 for
			// characters that were ELIMINATEd and will be set to the correct
			// row for characters that have not been ELIMINATEd.
			//
			j = charPos[currChar];

			if( labels )
			{
				// this should be the character label
				//
				token.GetNextToken();

				if( page == 0 && newchar )
				{
					// check for duplicate character names
					//
               nxsstring s = token.GetToken();
               LabelList::const_iterator iter = find( charLabels.begin(), charLabels.end(), s );
					int charLabelFound = ( iter != charLabels.end() );
					if( charLabelFound ) {
						errormsg = "Data for this character (";
                  errormsg += token.GetToken();
                  errormsg += ") has already been saved";
						throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
					}

					// Labels provided, need to add them to charLabels list.
					// We're not supposed to save anything for this character since it
					// has been ELIMINATEd, but the labels must be saved for purposes of
					// numbering.  Otherwise a more complicated system would be needed
					// wherein an association is set up between character number and
               // character label.  Since this is not done in the case of taxa
               // that are effectively ELIMINATEd when they are included in the
               // TAXA block but not in the CHARACTERS MATRIX command,  I see no
               // reason to not save the full character labels here even for those
               // that have been ELIMINATEd.  Also, for interleaved matrices, it
               // is necessary to have the full labels saved somewhere so that
               // it is possible to detect characters out of order or duplicated.
               //
               charLabels.push_back( token.GetToken() );
            }
				else // either not first interleaved page or character labels not previously defined
            {
               nxsstring s = token.GetToken();
               LabelList::const_iterator iter = find( charLabels.begin(), charLabels.end(), s );
               if( iter == charLabels.end() ) {
                  errormsg = "Could not find character named ";
                  errormsg += token.GetToken();
                  errormsg += " among stored character labels";
                  throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
               }
               int positionInCharLabelsList = ( iter - charLabels.begin() );

               // make sure user has not duplicated data for a single character or
               // changed the order in which characters appear in different interleave
               // pages
               //
               if( positionInCharLabelsList != currChar )
               {
                  if( page == 0 ) {
                     errormsg = "Data for this character (";
                     errormsg += token.GetToken();
                     errormsg += ") has already been saved";
							throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
                  }
                  else {
                     errormsg = "Ordering of characters must be identical to that in first interleave page";
							throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
                  }
               }

            }

			} // if labels conditional

			//************************************************
			//******** Beginning of loop through taxa ********
			//************************************************

			for( i = firstTaxon; i < lastTaxon; i++ )
			{
				if( page == 0 )
				{
					// We are forced to assume that the user did not leave out any
					// taxa, since without taxon labels in the matrix we would
					// have no way of detecting which were left out; thus,
					// ntax == ntaxTotal in this case.  Order of occurrence in
					// TAXA block is the same as the row in matrix.
					//
					taxonPos[i] = i;
				}

				// ok will be 0 only if a newline character is encountered before
				// taxon i processed
				//
				bool ok = HandleNextState( token, i, j );
            if( interleaving && !ok )
            {
               if( lastTaxon < ntaxTotal && i != lastTaxon ) {
                  errormsg = "Each line within an interleave page must comprise the same number of taxa";
                  throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
               }

               // i should be firstChar in next go around
               nextFirst = i;

               // set lastTaxon to i so that we can check to make sure the
               // remaining lines in this interleave page end at the same
               // place
               lastTaxon = i;

               // since i is now equal to lastTaxon, we are done with this innermost loop
            }
         } // innermost loop (over taxa)

      } // middle loop (over characters)

		firstTaxon = nextFirst;
		lastTaxon = ntaxTotal;

		// if i equals ncharTotal, then we've just finished reading the last
		// interleave page and thus should break from the outer loop
		// Note that if we are not interleaving, this will still work since
		// lastTaxon is initialized to ntaxTotal and never changed
		//
		if( i == ntaxTotal )
			break;

		page++;

   } // outer loop (over interleave pages)
}

/**
 * @method HandleMatrix [void:protected]
 * @param token [NexusToken&] the token used to read from in
 * @throws XNexus
 *
 * Called when MATRIX command needs to be parsed from within the
 * CHARACTERS block.  Deals with everything after the token MATRIX
 * up to and including the semicolon that terminates the MATRIX
 * command.
 */
void CharactersBlock::HandleMatrix( NexusToken& token )
{
	int i, j;

	if( ntax == 0 ) {
		errormsg = "Must precede ";
      errormsg += id;
      errormsg += " block with a TAXA block or specify NEWTAXA and NTAX in the DIMENSIONS command";
		throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
	}

	if( ntaxTotal == 0 )
		ntaxTotal = taxa.GetNumTaxonLabels();

	// We use >= rather than just > below because someone might have ELIMINATEd
	// all characters, and we should allow that (even though it is absurd)
	assert( nchar >= 0 );

	if( datatype == continuous ) {
		errormsg = "Sorry, continuous character matrices have not yet been implemented";
		throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
	}

	if( matrix != NULL )
		delete matrix;
	matrix = new DiscreteMatrix( ntax, nchar );
	
	// Allocate memory for (and initialize) the arrays activeTaxon and activeChar.
	// All characters and all taxa are initially active.
	//
	activeTaxon = new bool[ntax];
	for( i = 0; i < ntax; i++ )
		activeTaxon[i] = true;
		
	activeChar = new bool[nchar];
	for( j = 0; j < nchar; j++ )
		activeChar[j] = true;

   // The value of ncharTotal is normally identical to the value of nchar specified
   // in the CHARACTERS block DIMENSIONS command.  If an ELIMINATE command is
   // processed, however, nchar < ncharTotal.  Note that the ELIMINATE command
   // will have already been read by now, and the ELIMINATEd character numbers
   // will be stored in the IntSet eliminated.
   //
   // Note that if an ELIMINATE command has been read, charPos will have already
   // been created; thus, we only need to allocate and initialize charPos if user
   // did not specify an ELIMINATE command
   //
	if( charPos == NULL )
		BuildCharPosArray();

	// The value of ntaxTotal equals the number of taxa specified in the
	// TAXA block, whereas ntax equals the number of taxa specified in
	// the DIMENSIONS command of the CHARACTERS block.  These two numbers
   // will be identical unless some taxa were left out of the MATRIX
   // command of the CHARACTERS block, in which case ntax < ntaxTotal.
   //
	if( taxonPos != NULL )
		delete [] taxonPos;
  	taxonPos = new int[ntaxTotal];
   for( i = 0; i < ntaxTotal; i++ )
      taxonPos[i] = -1;

   if( transposing )
      HandleTransposedMatrix( token );
   else
      HandleStdMatrix( token );

   // If we've gotten this far, presumably it is safe to
   // tell the ASSUMPTIONS block that were ready to take on
   // the responsibility of being the current character-containing
   // block (to be consulted if characters are excluded or included
   // or if taxa are deleted or restored)
   assumptionsBlock.SetCallback(this);

   // this should be the terminating semicolon at the end of the matrix command
   token.GetNextToken();
   if( !token.Equals(";") ) {
      errormsg = "Expecting ';' at the end of the MATRIX command; found ";
      errormsg += token.GetToken();
      errormsg += " instead";
		throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
   }

}

/**
 * @method HandleStatelabels [void:protected]
 * @param token [NexusToken&] the token used to read from in
 * @throws XNexus
 *
 * Called when STATELABELS command needs to be parsed from within the
 * DIMENSIONS block.  Deals with everything after the token STATELABELS
 * up to and including the semicolon that terminates the STATELABELS
 * command. Note that the numbers of states are shifted back one before
 * being stored so that the character numbers in the LabelListAssoc objects
 * are 0-offset rather than being 1-offset as in the Nexus data file.
 */
void CharactersBlock::HandleStatelabels( NexusToken& token )
{
	bool semicolonFoundInInnerLoop = false;

	charStates.erase( charStates.begin(), charStates.end() );

	if( charPos == NULL )
		BuildCharPosArray();
	
	for(;;)
	{
		if( semicolonFoundInInnerLoop )
			break;

		token.GetNextToken();

		if( token.Equals(";") )
			break;

		// token should be the character number; create a new association
		//
		int n = atoi( token.GetToken().c_str() );
		if( n < 1 || n > ncharTotal ) {
			errormsg = "Invalid character number (";
         errormsg += token.GetToken();
         errormsg += ") found in STATELABELS command (either out of range or not interpretable as an integer)";
			throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
		}
		
		for(;;)
		{
			token.GetNextToken();

			if( token.Equals(";") ) {
				semicolonFoundInInnerLoop = true;
				break;
			}

			if( token.Equals(",") ) {
				break;
         }

         // token should be a character state label; add it to the list
         //
         if( !IsEliminated(n-1) ) {
         	int k = GetCharPos(n-1);
	         charStates[k].push_back( token.GetToken() );
	      }

      } // inner loop (grabbing state labels for character n)

	 } // outer loop
}

/**
 * @method HandleTaxlabels [void:protected]
 * @param token [NexusToken&] the token used to read from in
 * @throws XNexus
 *
 * Called when TAXLABELS command needs to be parsed from within the
 * CHARACTERS block.  Deals with everything after the token TAXLABELS
 * up to and including the semicolon that terminates the TAXLABELS
 * command.
 */
void CharactersBlock::HandleTaxlabels( NexusToken& token )
{
	if( !newtaxa ) {
		errormsg = "NEWTAXA must have been specified in DIMENSIONS command to use the TAXLABELS command in a ";
      errormsg += id;
      errormsg += " block";
		throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
	}

	for(;;)
	{
		token.GetNextToken();

      // token should either be ';' or the name of a taxon
      //
		if( token.Equals(";") ) {
			break;
		}
      else {
         // check to make sure user is not trying to read in more
         // taxon labels than there are taxa
         //
         if( taxa.GetNumTaxonLabels() > ntaxTotal ) {
				errormsg = "Number of taxon labels exceeds NTAX specified in DIMENSIONS command";
				throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
         }

         taxa.AddTaxonLabel( token.GetToken() );
      }
	}

   // OPEN ISSUE: Some may object to setting newtaxa to false here, because then the
   // fact that new taxa were specified in this CHARACTERS block rather than in
   // a preceding TAXA block is lost.  This will only be important if we wish to
   // recreate the original data file, which I don't anticipate anyone doing with
   // this code (too difficult to remember all comments, the order of blocks in
   // the file, etc.)

   newtaxa = false;

}

/**
 * @method PositionInSymbols [int:protected]
 * @param ch [int] the symbol character to search for
 *
 * Returns position of ch in symbols array.  The value of respectingCase
 * is used to determine whether the search should be case sensitive or not.
 * Assumes symbols != NULL. Returns -1 if ch is not found in symbols.
 */
int CharactersBlock::PositionInSymbols( char ch )
{
	assert( symbols != NULL );
	int symbolsLength = strlen(symbols);
	int found = 0;
	int i;
	for( i = 0; i < symbolsLength; i++ )
	{
		char char_in_symbols = ( respectingCase ? symbols[i] : (char)toupper(symbols[i]) );
		char char_in_question = ( respectingCase ? ch : (char)toupper(ch) );
		if( char_in_symbols != char_in_question ) continue;
		found = 1;
		break;
	}
	return ( found ? i : -1 );
}

/**
 * @method Read [void:protected]
 * @param token [NexusToken&] the token used to read from in
 * @param in [istream&] the input stream from which to read
 * @throws XNexus
 *
 * This function provides the ability to read everything following the block name
 * (which is read by the Nexus object) to the end or endblock statement.
 * Characters are read from the input stream in. Overrides the
 * abstract virtual function in the base class.
 */
void CharactersBlock::Read( NexusToken& token )
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

	ntax = taxa.GetNumTaxonLabels();

	for(;;)
	{
		token.GetNextToken();

		if( token.Equals("DIMENSIONS") ) {
			HandleDimensions( token, "NEWTAXA", "NTAX", "NCHAR" );
		}
		else if( token.Equals("FORMAT") ) {
			HandleFormat( token );
		}
		else if( token.Equals("ELIMINATE") ) {
			HandleEliminate( token );
		}
		else if( token.Equals("TAXLABELS") ) {
			HandleTaxlabels( token );
		}
		else if( token.Equals("CHARSTATELABELS") ) {
			HandleCharstatelabels( token );
		}
		else if( token.Equals("CHARLABELS") ) {
			HandleCharlabels( token );
		}
		else if( token.Equals("STATELABELS") ) {
			HandleStatelabels( token );
		}
		else if( token.Equals("MATRIX") ) {
			HandleMatrix( token );
		}
		else if( token.Equals("END") ) {
			HandleEndblock( token, "Character" );
			break;
		}
		else if( token.Equals("ENDBLOCK") ) {
			HandleEndblock( token, "Character" );
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
 * This function outputs a brief report of the contents of this
 * CHARACTERS block.  Overrides the abstract virtual function
 * in the base class.
 */
void CharactersBlock::Report( ostream& out )
{
	out << endl;
	out << id << " block contains ";
	if( ntax == 0 )
		out << "no taxa";
	else if( ntax == 1 )
		out << "one taxon";
	else
		out << ntax << " taxa";
	out << " and ";
	if( nchar == 0 )
		out << "no characters";
	else if( nchar == 1 )
		out << "one character";
	else
		out << nchar << " characters";
	out << endl;

	switch( datatype )
	{
		case dna:
			out << "  Data type is \"DNA\"" << endl;
			break;
		case rna:
			out << "  Data type is \"RNA\"" << endl;
			break;
		case nucleotide:
			out << "  Data type is \"nucleotide\"" << endl;
			break;
		case protein:
			out << "  Data type is \"protein\"" << endl;
			break;
		case continuous:
			out << "  Data type is \"continuous\"" << endl;
			break;
		default:
			out << "  Data type is \"standard\"" << endl;
	}

	if( respectingCase )
		out << "  Respecting case" << endl;
	else
		out << "  Ignoring case" << endl;

	if( tokens )
		out << "  Multicharacter tokens allowed in data matrix" << endl;
	else
		out << "  Data matrix entries are expected to be single symbols" << endl;

	if( labels && transposing )
		out << "  Character labels are expected on left side of matrix" << endl;
	else if( labels && !transposing )
		out << "  Taxon labels are expected on left side of matrix" << endl;
	else
		out << "  No labels are expected on left side of matrix" << endl;

	if( charLabels.size() > 0 )
	{
		out << "  Character and character state labels:" << endl;
		for( int k = 0; k < nchar; k++ ) 
		{
			if( charLabels[k].length() == 0 )
				out << '\t' << ( 1 + GetOrigCharIndex(k) ) << '\t' << "(no label provided for this character)" << endl;
			else
				out << '\t' << ( 1 + GetOrigCharIndex(k) ) << '\t' << charLabels[k] << endl;

			// output state labels if any are defined for this character
         LabelListBag::const_iterator cib = charStates.find(k);
         if( cib != charStates.end() )
         {
            int ns = (*cib).second.size();
            for( int m = 0; m < ns; m++ ) {
         		out << "\t\t" << (*cib).second[m] << endl;
            }
         }
		}
	}

	if( transposing && interleaving )
		out << "  Matrix transposed and interleaved" << endl;
	else if( transposing && !interleaving )
		out << "  Matrix transposed but not interleaved" << endl;
	else if( !transposing && interleaving )
		out << "  Matrix interleaved but not transposed" << endl;
	else
		out << "  Matrix neither transposed nor interleaved" << endl;

	out << "  Missing data symbol is '" << missing << '\'' << endl;

	if( matchchar != '\0' )
		out << "  Match character is '" << matchchar << '\'' << endl;
	else
		out << "  No match character specified" << endl;

	if( gap != '\0' )
		out << "  Gap character specified is '" << gap << '\'' << endl;
	else
		out << "  No gap character specified" << endl;

	out << "  Valid symbols are: " << symbols << endl;

	int numEquateMacros = equates.size();
	if( numEquateMacros > 0 ) {
		out << "  Equate macros in effect:" << endl;
      typedef AssocList::const_iterator CI;
		for( CI i = equates.begin(); i != equates.end(); ++i) {
			out << "    " << (*i).first << " = " << (*i).second << endl;
		}
	}
	else
		out << "  No equate macros have been defined" << endl;

	if( ncharTotal == nchar )
		out << "  No characters were eliminated" << endl;
	else {
		out << "  The following characters were eliminated:" << endl;
		IntSet::const_iterator k;
		for( k = eliminated.begin(); k != eliminated.end(); k++ ) {
			out << "    " << ((*k)+1) << endl;
		}
	}

	out << "  The following characters have been excluded:" << endl;
	int k;
	int nx = 0;
	for( k = 0; k < nchar; k++ ) {
		if( activeChar[k] ) continue;
		out << "    " << (k+1) << endl;
		nx++;
	}
	if( nx == 0 )
		out << "    (no characters excluded)" << endl;

	out << "  The following taxa have been deleted:" << endl;
	nx = 0;
	for( k = 0; k < ntax; k++ ) {
		if( activeTaxon[k] ) continue;
		out << "    " << (k+1) << endl;
		nx++;
	}
	if( nx == 0 )
		out << "    (no taxa deleted)" << endl;

	out << "  Data matrix:" << endl;
	DebugShowMatrix( out, "    " );
}

/**
 * @method Reset [void:protected]
 *
 * Sets ntax and nchar to 0 in preparation for reading a new
 * CHARACTERS block.
 */
void CharactersBlock::Reset()
{
	isEmpty        = true;
	ntax           = 0;
	nchar          = 0;
	newchar        = true;
	newtaxa        = false;
	interleaving   = false;
	transposing    = false;
	respectingCase = false;
	labels         = true;
	datatype       = standard;
	missing        = '?';
	gap            = '\0';
	matchchar      = '\0';

	charLabels.erase( charLabels.begin(), charLabels.end() );
	charStates.erase( charStates.begin(), charStates.end() );
	equates.erase( equates.begin(), equates.end() );

	ResetSymbols();

	if( matrix != NULL ) {
		delete matrix;
		matrix = NULL;
	}

	if( charPos != NULL ) {
		delete [] charPos;
		charPos = NULL;
	}

	if( taxonPos != NULL ) {
		delete [] taxonPos;
		taxonPos = NULL;
	}

	if( activeTaxon != NULL ) {
		delete [] activeTaxon;
		activeTaxon = NULL;
	}

	if( activeChar != NULL ) {
		delete [] activeChar;
		activeChar = NULL;
	}

	if( !eliminated.empty() ) {
		eliminated.erase( eliminated.begin(), eliminated.end() );
	}
}

/**
 * @method ResetSymbols [void:protected]
 *
 * Resets standard symbol set after a change in datatype is made. Also
 * flushes equates list and installs standard equate macros for the
 * current datatype.
 */
void CharactersBlock::ResetSymbols()
{
	// define the standard symbols
	switch( datatype )
	{
		case dna:
			strcpy( symbols, "ACGT" );
			break;
		case rna:
			strcpy( symbols, "ACGU" );
			break;
		case nucleotide:
			strcpy( symbols, "ACGT" );
			break;
		case protein:
			strcpy( symbols, "ACDEFGHIKLMNPQRSTVWY*" );
			break;
		default:
			strcpy( symbols, "01" );
	}

	// setup standard equates
   equates.erase( equates.begin(), equates.end() );
//trash   Association* newEquate;
   if( datatype == dna || datatype == rna || datatype == nucleotide )
   {
#if 1
      equates[ nxsstring("R") ] = nxsstring("{AG}");
      equates[ nxsstring("Y") ] = nxsstring("{CT}");
      equates[ nxsstring("M") ] = nxsstring("{AC}");
      equates[ nxsstring("K") ] = nxsstring("{GT}");
      equates[ nxsstring("S") ] = nxsstring("{CG}");
      equates[ nxsstring("W") ] = nxsstring("{AT}");
      equates[ nxsstring("H") ] = nxsstring("{ACT}");
      equates[ nxsstring("B") ] = nxsstring("{CGT}");
      equates[ nxsstring("V") ] = nxsstring("{ACG}");
      equates[ nxsstring("D") ] = nxsstring("{AGT}");
      equates[ nxsstring("N") ] = nxsstring("{ACGT}");
      equates[ nxsstring("X") ] = nxsstring("{ACGT}");
#else
      newEquate = new Association();
      newEquate->SetKey("R");
      newEquate->SetValue("{AG}");
      equates.AddAssociation( newEquate );

      newEquate = new Association();
      newEquate->SetKey("Y");
		newEquate->SetValue("{CT}");
      equates.AddAssociation( newEquate );

      newEquate = new Association();
      newEquate->SetKey("M");
      newEquate->SetValue("{AC}");
		equates.AddAssociation( newEquate );

      newEquate = new Association();
      newEquate->SetKey("K");
      newEquate->SetValue("{GT}");
      equates.AddAssociation( newEquate );

      newEquate = new Association();
      newEquate->SetKey("S");
      newEquate->SetValue("{CG}");
      equates.AddAssociation( newEquate );

      newEquate = new Association();
      newEquate->SetKey("W");
      newEquate->SetValue("{AT}");
      equates.AddAssociation( newEquate );

      newEquate = new Association();
      newEquate->SetKey("H");
      newEquate->SetValue("{ACT}");
      equates.AddAssociation( newEquate );

      newEquate = new Association();
      newEquate->SetKey("B");
		newEquate->SetValue("{CGT}");
      equates.AddAssociation( newEquate );

      newEquate = new Association();
      newEquate->SetKey("V");
      newEquate->SetValue("{ACG}");
      equates.AddAssociation( newEquate );

      newEquate = new Association();
      newEquate->SetKey("D");
      newEquate->SetValue("{AGT}");
      equates.AddAssociation( newEquate );

      newEquate = new Association();
		newEquate->SetKey("N");
      newEquate->SetValue("{ACGT}");
      equates.AddAssociation( newEquate );

		newEquate = new Association();
      newEquate->SetKey("X");
      newEquate->SetValue("{ACGT}");
      equates.AddAssociation( newEquate );
#endif
   }
   else if( datatype == protein )
   {
#if 1
      equates[ nxsstring("B") ] = nxsstring("{DN}");
      equates[ nxsstring("Z") ] = nxsstring("{EQ}");
#else
      newEquate = new Association();
      newEquate->SetKey("B");
      newEquate->SetValue("{DN}");
      equates.AddAssociation( newEquate );

		newEquate = new Association();
      newEquate->SetKey("Z");
      newEquate->SetValue("{EQ}");
      equates.AddAssociation( newEquate );
#endif
   }
}

/**
 * @method RestoreTaxon [void:protected]
 * @param i [int] index of taxon to restore in range [0..ntax)
 *
 * Restores taxon whose 0-offset current index is i.  If taxon
 * is already active, this function has no effect.
 */
void CharactersBlock::RestoreTaxon( int i )
{
	activeTaxon[i] = true;
}

/**
 * @method ShowStateLabels [void:protected]
 * @param out [ostream&] the output stream on which to write
 * @param i [int] the taxon, in range [0..ntax)
 * @param j [int] the character, in range [0..nchar)
 *
 * Looks up the state(s) at row i, column j of matrix and writes it (or them)
 * to out.  If there is uncertainty or polymorphism, the list of states is
 * surrounded by the appropriate set of symbols (i.e., parentheses for polymorphism,
 * curly brackets for uncertainty).  If 'tokens' is in effect, the output takes
 * the form of the defined state labels; otherwise, the correct symbol is
 * looked up in symbols and output.
 */
void CharactersBlock::ShowStateLabels( ostream& out, int i, int j )
{
   if( tokens )
   {
      int n = matrix->GetNumStates( i, j );
      if( n == 0 && matrix->IsGap( i, j ) )
         out << gap;
      else if( n == 0 && matrix->IsMissing( i, j ) )
         out << missing;
      else if( n == 1 ) {
         int s = matrix->GetState( i, j );
         LabelListBag::const_iterator ci = charStates.find(j);
         // OPEN ISSUE: need to eliminate state labels for characters that have
         // been eliminated
         if( ci == charStates.end() )
            out << "  " << s << "[<-no label found]";
         else {
            // show label at index number s in LabelList at ci
            out << "  " << (*ci).second[s];
         }
      }
      else {
         if( matrix->IsPolymorphic( i, j ) )
            out << "  (";
         else
            out << "  {";
         for( int k = 0; k < n; k++ ) {
            int s = matrix->GetState( i, j, k );
            LabelListBag::const_iterator ci = charStates.find(j);
            if( ci == charStates.end() )
               out << "  " << s << "[<-no label found]";
            else {
               // show label at index number s in LabelList at ci
               out << "  " << (*ci).second[s];
            }
         }
         if( matrix->IsPolymorphic( i, j ) )
            out << ')';
         else
            out << '}';
      }

   }
   else
   	ShowStates( out, i, j );
}

/**
 * @method ShowStates [char*:public]
 * @param out [ostream&] the stream on which to show the state(s)
 * @param i [int] the (0-offset) index of the taxon in question
 * @param j [int] the (0-offset) index of the character in question
 *
 * Shows the states for taxon i, character j, on the stream out.  Uses
 * symbols array to translate the states from the way they are stored (as
 * integers) to the symbol used in the original data matrix.
 * Assumes i is in the range [0..nrows) and j is in the range [0..ncols).
 * Also assumes both symbols and data are non-NULL.
 */
void CharactersBlock::ShowStates( ostream& out, int i, int j )
{
	assert( i >= 0 );
	assert( i < ntax );
	assert( j >= 0 );
	assert( j < nchar );

   char s[NCL_MAX_STATES+3];
   WriteStates( matrix->GetDiscreteDatum( i, j ), s, NCL_MAX_STATES+3 );

   out << s;
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
int CharactersBlock::TaxonLabelToNumber( nxsstring s )
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

/**
 * @method WriteStates [void:public]
 * @param d [DiscreteDatum&] the datum to be queried
 * @param s [char*] the buffer to which to print
 * @param slen [int] the length of the buffer s
 *
 * Writes out the state (or states) stored in this DiscreteDatum object
 * to the buffer s using the symbols array to do the necessary
 * translation of the numeric state values to state symbols.  In the
 * case of polymorphism or uncertainty, the list of states will be
 * surrounded by brackets or parentheses (respectively).  Assumes
 * s is long enough to hold everything printed.
 */
void CharactersBlock::WriteStates( DiscreteDatum& d, char* s, int slen )
{

   assert( s != NULL );
   assert( slen > 1 );

   if( matrix->IsMissing(d) ) {
      s[0] = missing;
      s[1] = '\0';
   }
   else if( matrix->IsGap(d) ) {
      s[0] = gap;
      s[1] = '\0';
   }
   else
   {
      assert( symbols != NULL );
      int symbolListLen = strlen( symbols );

      int numStates = matrix->GetNumStates(d);
      int numCharsNeeded = numStates;
      if( numStates > 1 )
         numCharsNeeded += 2;
      assert( slen > numCharsNeeded );

      if( numStates == 1 ) {
         int v = matrix->GetState( d );
         assert( v < symbolListLen );
         s[0] = symbols[v];
         s[1] = '\0';
      }
      else {
         // numStates must be greater than 1
         //
         int i = 0;
         if( matrix->IsPolymorphic(d) )
            s[i++] = '(';
         else
            s[i++] = '{';
         for( int k = 0; k < numStates; k++ ) {
            int v = matrix->GetState( d, k );
            assert( v < symbolListLen );
            s[i++] = symbols[v];
            s[i] = '\0';
         }
         if( matrix->IsPolymorphic(d) )
            s[i++] = ')';
         else
            s[i++] = '}';
         s[i] = '\0';
      }
   }
}


