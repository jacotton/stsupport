#include "nexusdefs.h"
#include "discretedatum.h"
#include "discretematrix.h"

/**
 * @class      DiscreteMatrix
 * @file       discretematrix.h
 * @file       discretematrix.cpp
 * @author     Paul O. Lewis
 * @copyright  Copyright © 1999. All Rights Reserved.
 * @variable   data [Datum**:public] storage for the data
 * @variable   ncols [int:public] number of rows (taxa) in the data matrix
 * @variable   nrows [int:public] number of columns (characters) in the data matrix
 * @see        AllelesBlock
 * @see        CharactersBlock
 * @see        DataBlock
 * @see        DiscreteDatum
 * @see        NexusReader
 *
 * Class providing storage for the discrete data types (dna, rna, nucleotide,
 * standard, and protein) inside a DATA or CHARACTERS block.  This class is
 * also used to store the data for an ALLELES block.  Maintains a matrix in
 * which each cell is an object of the class DiscreteDatum.  DiscreteDatum
 * stores the state for a particular combination of taxon and character as
 * an integer.  Ordinarily, there will be a single state recorded for each
 * taxon/character combination, but exceptions exist if there is polymorphism
 * for this taxon/character or if there is uncertainty about the state (e.g.,
 * in dna data, the data file might have contained an R or Y entry).  Please
 * consult the documentation for the <a href="DiscreteDatum.html">DiscreteDatum</a>
 * class for the details about how states are stored.
 *
 * <p>For data stored in an ALLELES block, rows of the matrix correspond to
 * individuals and columns to loci.  Each DiscreteDatum int must therefore
 * store information about both genes at a single locus for a single individual
 * in the case of diploid data.  To do this, two macros HIWORD and LOWORD are
 * used to divide up the int value into two words.  A maximum of 255 distinct
 * allelic forms can be accommodated by this scheme, assuming at minimum a
 * 32-bit architecture.  Because it is not known in advance how many rows are
 * going to be necessary, The DiscreteMatrix class provides the AddRows
 * method, which expands the number of rows allocated for the matrix while
 * preserving data already stored.
 */

/**
 * @constructor
 *
 * Performs the following initializations:
 * <table>
 * <tr><th align="left">Variable <th> <th align="left"> Initial Value
 * <tr><td> nrows   <td>= <td> rows
 * <tr><td> ncols    <td>= <td> cols
 * </table>
 * <p> In addition, memory is allocated for data (each element of the matrix
 * data is a DiscreteDatum object which can do its own initialization).
 */
DiscreteMatrix::DiscreteMatrix( int rows, int cols )	: nrows(rows), ncols(cols)
{
	int i;

	data = new DiscreteDatum*[nrows];
	for( i = 0; i < nrows; i++ )
		data[i] = new DiscreteDatum[ncols];
}

/**
 * @destructor
 *
 * Deletes memory allocated in the constructor for data and symbols.
 */
DiscreteMatrix::~DiscreteMatrix()
{
	int i;

	if( data != NULL ) {
		for( i = 0; i < nrows; i++ )
			delete [] data[i];
		delete [] data;
	}
}

/**
 * @method AddRows [void:private]
 * @param nAddRows [int] the number of additional rows to allocate
 *
 * Allocates memory for nAddRows additional rows and updates the variable
 * nrows.  Data already stored in data is not destroyed; the newly-allocated
 * rows are added at the bottom of the existing matrix.
 */
void DiscreteMatrix::AddRows( int nAddRows )
{
	int i;
   int new_nrows = nrows + nAddRows;
	DiscreteDatum** new_data = new DiscreteDatum*[new_nrows];
	for( i = 0; i < nrows; i++ )
		new_data[i] = data[i];
   delete [] data;
   data = new_data;
	for( i = nrows; i < new_nrows; i++ )
		data[i] = new DiscreteDatum[ncols];
   nrows = new_nrows;
}

/**
 * @method AddState [void:public]
 * @param i [int] the (0-offset) index of the taxon in question
 * @param j [int] the (0-offset) index of the character in question
 * @param value [int] the state to be added
 *
 * Adds state directly to the DiscreteDatum object at data[i][j].
 * Assumes i is in the range [0..nrows) and j is in the range [0..ncols).
 * The value parameter is assumed to be either zero or a positive integer.
 * Calls private member function AddState to do the real work; look at
 * the documentation for that function for additional details.
 */
void DiscreteMatrix::AddState(  int i, int j, int value )
{
	assert( i >= 0 );
	assert( i < nrows );
	assert( j >= 0 );
	assert( j < ncols );
	assert( data != NULL );
	assert( value >= 0 );

	AddState( data[i][j], value );
}

/**
 * @method AddState [void:private]
 * @param d [DiscreteDatum&] the DiscreteDatum object affected
 * @param value [int] the additional state to be added
 *
 * Adds an additional state to the array states of d.  If states is NULL,
 * allocates memory for two integers and assigns 1 to the first and value
 * to the second.  If states is non-NULL, allocates a new int array long
 * enough to hold states already present plus the new one being added here,
 * then deletes the old states array.  Assumes that we are not trying to
 * set either the missing state or the gap state here; the functions
 * SetMissing or SetGap, respectively, should be used for those purposes.
 * Also assumes that we do not want to "overwrite" the state.  This function
 * adds states to those already present; use SetState to overwrite the state.
 */
void DiscreteMatrix::AddState( DiscreteDatum& d, int value )
{
	int oldns = GetNumStates(d);
	int k, newlen;

	int* tmp = d.states;

	if( IsMissing(d) ) {
		d.states = new int[2];
		d.states[0] = 1;
		d.states[1] = value;
	}
	else if( IsGap(d) ) {
		d.states = new int[2];
		d.states[0] = 1;
		d.states[1] = value;
	}
	else if( oldns == 1 ) {
		d.states = new int[4];
		d.states[0] = 2;
		d.states[1] = tmp[1];
		d.states[2] = value;
		d.states[3] = 0;  // assume not polymorphic unless told otherwise
	}
	else {
		newlen = oldns + 3;
		d.states = new int[newlen];
		d.states[0] = oldns+1;
		for( k = 1; k < oldns+1; k++ )
			d.states[k] = tmp[k];
		d.states[newlen-2] = value;
		d.states[newlen-1] = 0;  // assume not polymorphic unless told otherwise
	}

	if( tmp != NULL )
		delete [] tmp;
}

/**
 * @method CopyStatesFromFirstTaxon [long:public]
 * @param i [int] the (0-offset) index of the taxon in question
 * @param j [int] the (0-offset) index of the character in question
 *
 * Sets state of taxon i and character j to state of first taxon for character j.
 * Assumes i is in the range [0..nrows) and j is in the range [0..ncols).
 * Also assumes both data and symbols are non-NULL.
 */
void DiscreteMatrix::CopyStatesFromFirstTaxon( int i, int j )
{
	assert( i >= 0 );
	assert( i < nrows );
	assert( j >= 0 );
	assert( j < ncols );
	assert( data != NULL );
	
	data[i][j].CopyFrom( data[0][j] );
}

/**
 * @method DebugSaveMatrix [int:public]
 * @param out [ostream&] the stream on which to dump the matrix contents
 *
 * Performs a dump of the current contents of the data matrix stored in
 * the variable "data"
 */
void DiscreteMatrix::DebugSaveMatrix( ostream& out, int colwidth /* = 12 */ )
{
   out << endl;
   out << "nrows = " << nrows << endl;
   out << "ncols = " << ncols << endl;
   for( int i = 0; i < nrows; i++ ) {
      for( int j = 0; j < ncols; j++ ) {
         if( IsMissing(i, j) )
            out << setw(colwidth) << '?';
         else if( IsGap(i, j) )
            out << setw(colwidth) << '-';
         else
            out << setw(colwidth) << GetState(i, j);
      }
      out << endl;
   }
}

/**
 * @method DuplicateRow [int:public]
 * @param row [int] the row to be duplicated
 * @param count [int] the total number of copies needed
 * @param startCol [int] the starting column (inclusive) in the range of columns to be duplicated
 * @param endCol [int] the ending column (inclusive) in the range of columns to be duplicated
 *
 * Duplicates columns startCol to endCol in row row of the matrix.  If additional
 * storage is needed to accommodate the duplication, this is done automatically
 * through the use of the AddRows method.  Note that count includes the row already
 * present, so if count is 10, then 9 more rows will actually be added to the matrix
 * to make a total of 10 identical rows.  The parameters startCol and endCol default
 * to 0 and ncols, so if duplication of the entire row is needed, these need not
 * be explicitly specified in the call to DuplicateRow.
 *
 * <p>Return value is number of additional rows allocated to matrix (0 if no
 * rows needed to be allocated).
 */
int DiscreteMatrix::DuplicateRow( int row, int count
   , int startCol /* = 0 */, int endCol /* = -1 */ )
{
	assert( data != NULL );
	assert( row >= 0 );
	assert( row < nrows );
	assert( startCol >= 0 );
	assert( startCol < ncols );
   if( endCol == -1 )
      endCol = ncols-1;
	assert( endCol > startCol );
	assert( endCol < ncols );

   // expand matrix (if necessary) to accommodate additional rows
   //
   int nNewRows = 0;
   if( row + count > nrows ) {
      nNewRows = row + count - nrows;
      AddRows( nNewRows );
   }

   for( int i = 1; i < count; i++ ) {
      for( int col = startCol; col <= endCol; col++ ) {
         data[row+i][col] = data[row][col];
      }
   }

   return nNewRows;
}

/**
 * @method Flush [void:public]
 *
 * Deletes all cells of data and resets nrows and ncols to 0.
 */
void DiscreteMatrix::Flush()
{
	// delete what is there now
	if( data != NULL )
	{
		int i;
		for( i = 0; i < nrows; i++ )
			delete [] data[i];
		delete [] data;
	}

	nrows = 0;
	ncols = 0;
}

/**
 * @method GetDiscreteDatum [DiscreteDatum&:private]
 * @param i [int] the row of the matrix
 * @param j [int] the column of the matrix
 *
 * Assumes that i is in the range [0..nrows) and j is in the range [0..ncols).
 * Returns reference to the DiscreteDatum object at row i, column j of matrix.
 */
DiscreteDatum& DiscreteMatrix::GetDiscreteDatum( int i, int j )
{
	assert( i >= 0 );
	assert( i < nrows );
	assert( j >= 0 );
	assert( j < ncols );
	assert( data != NULL );

   return data[i][j];
}

/**
 * @method GetNumStates [int:public]
 * @param i [int] the (0-offset) index of the taxon in question
 * @param j [int] the (0-offset) index of the character in question
 *
 * Returns number of states for taxon i and character j.
 * Assumes i is in the range [0..nrows) and j is in the range [0..ncols).
 */
int DiscreteMatrix::GetNumStates( int i, int j )
{
	assert( i >= 0 );
	assert( i < nrows );
	assert( j >= 0 );
	assert( j < ncols );
	assert( data != NULL );

   return GetNumStates( data[i][j] );
}

/**
 * @method GetNumStates [int:private]
 * @param d [DiscreteDatum&] the datum in question
 *
 * Returns total number of states assigned to d.  This function will return
 * 0 for both gap and missing states.
 */
int DiscreteMatrix::GetNumStates( DiscreteDatum& d )
{
   if( d.states == NULL )
      return 0;

	return d.states[0];
}

/**
 * @method GetObsNumStates [int:public]
 * @param j [int] the (0-offset) index of the character in question
 *
 * Returns number of states for character j over all taxa.  
 * Note: this function is rather slow, as it must walk through 
 * each row, adding the states encountered to a set, then finally  
 * returning the size of the set.  Thus, if this function is called 
 * often, it would be advisable to initialize an array using this 
 * function, then refer to the array subsequently.
 * Assumes j is in the range [0..ncols).
 */
int DiscreteMatrix::GetObsNumStates( int j )
{
	assert( j >= 0 );
	assert( j < ncols );
	assert( data != NULL );

   set< int, less<int> > stateset;
   for( int i = 0; i < nrows; i++ ) {
      DiscreteDatum& d = data[i][j];
      int ns = GetNumStates(d);
      if( ns == 0 ) continue;
      for( int k = 0; k < ns; k++ )
         stateset.insert( GetState( d, k ) );
   }

   return stateset.size();
}

/**
 * @method GetState [int:private]
 * @param i [int] the row of the matrix
 * @param j [int] the column of the matrix
 * @param k [int] the state to return (use default of 0 if only one state present)
 *
 * Assumes that i is in the range [0..nrows) and j is in the range [0..ncols).
 * Also assumes that cell i, j holds at least one state (i.e., it is not the
 * gap or missing states). Use the function GetNumStates to determine the 
 * number of states present (k must be less than this number).
 */
int DiscreteMatrix::GetState( int i, int j, int k /* = 0 */ )
{
	assert( i >= 0 );
	assert( i < nrows );
	assert( j >= 0 );
	assert( j < ncols );
	assert( data != NULL );

	return GetState( data[i][j], k );
}

/**
 * @method GetState [int:private]
 * @param d [DiscreteDatum&] the datum in question
 * @param i [int] the number of the state
 *
 * Returns the internal int representation of the state stored in d
 * at position i of the array states. Assumes that the state is not the
 * missing or gap state.  Use IsMissing and IsGap prior to calling
 * this function to ensure this function will succeed.  The default
 * value for i is 0, so calling GetState(d) will return the first
 * state, whether or not there are multiple states stored.  Assumes
 * that i is in the range [ 0 .. d.states[0] ).
 */
int DiscreteMatrix::GetState( DiscreteDatum& d, int i /* = 0 */ )
{
   assert( !IsMissing(d) );
	assert( !IsGap(d) );
	assert( i >= 0 );
	assert( i < d.states[0] );
	return d.states[i+1];
}

/**
 * @method IsGap [int:public]
 * @param i [int] the (0-offset) index of the taxon in question
 * @param j [int] the (0-offset) index of the character in question
 *
 * Returns 1 if the state for taxon i, character j, is set to the
 * gap symbol, 0 otherwise.  Assumes i is in the range [0..nrows) and j is
 * in the range [0..ncols).
 */
int DiscreteMatrix::IsGap( int i, int j )
{
	assert( i >= 0 );
	assert( i < nrows );
	assert( j >= 0 );
	assert( j < ncols );
	assert( data != NULL );

	return IsGap( data[i][j] );
}

/**
 * @method IsGap [int:private]
 * @param d [DiscreteDatum&] the datum in question
 *
 * Returns 1 if the gap state is stored, 0 otherwise.
 */
int DiscreteMatrix::IsGap( DiscreteDatum& d )
{
   if( d.states == NULL || d.states[0] > 0 )
      return 0;
   else
      return 1;
}

/**
 * @method IsMissing [int:public]
 * @param i [int] the (0-offset) index of the taxon in question
 * @param j [int] the (0-offset) index of the character in question
 *
 * Returns 1 if the state for taxon i, character j, is set to the
 * missing data symbol, 0 otherwise.  Assumes i is in the range [0..nrows)
 * and j is in the range [0..ncols).
 */
int DiscreteMatrix::IsMissing( int i, int j )
{
	assert( i >= 0 );
	assert( i < nrows );
	assert( j >= 0 );
	assert( j < ncols );
	assert( data != NULL );

   return IsMissing( data[i][j] );
}

/**
 * @method IsMissing [int:private]
 * @param d [DiscreteDatum&] the datum in question
 *
 * Returns 1 if the missing state is stored, 0 otherwise.
 */
int DiscreteMatrix::IsMissing( DiscreteDatum& d )
{
   if( d.states == NULL )
      return 1;
   else
      return 0;
}

/**
 * @method IsPolymorphic [int:public]
 * @param i [int] the (0-offset) index of the taxon in question
 * @param j [int] the (0-offset) index of the character in question
 *
 * Returns 1 if character j is polymorphic in taxon i, 0 otherwise.  Assumes
 * i is in the range [0..nrows) and j is in the range [0..ncols).
 */
int DiscreteMatrix::IsPolymorphic( int i, int j )
{
	assert( i >= 0 );
	assert( i < nrows );
	assert( j >= 0 );
	assert( j < ncols );
	assert( data != NULL );

   return IsPolymorphic( data[i][j] );
}

/**
 * @method IsPolymorphic [int:private]
 * @param d [DiscreteDatum&] the datum in question
 *
 * Returns 1 if the number of states is greater than 1 and polymorphism
 * has been specified.  Returns 0 if the state stored is the missing state,
 * the gap state, or if the number of states is 1.
 */
int DiscreteMatrix::IsPolymorphic( DiscreteDatum& d )
{
   if( d.states == NULL || d.states[0] < 2 )
      return 0;

   int nstates = d.states[0];
   int ncells = nstates + 2;
   return d.states[ncells-1];
}

/**
 * @method Reset [void:public]
 * @param rows [int] the new number of rows (taxa)
 * @param cols [int] the new number of columns (characters)
 *
 * Deletes all cells of data and flags and reallocates memory to create
 * a new matrix object with nrows = rows and ncols = cols.
 */
void DiscreteMatrix::Reset( int rows, int cols )
{
	int i;

   assert( rows > 0 );
   assert( cols > 0 );

	// delete what is there now
	if( data != NULL ) {
		for( i = 0; i < nrows; i++ )
			delete [] data[i];
		delete [] data;
	}

	nrows = rows;
	ncols = cols;

	// create new data matrix
	data = new DiscreteDatum*[nrows];
	for( i = 0; i < nrows; i++ ) {
		data[i] = new DiscreteDatum[ncols];
	}
}

/**
 * @method SetGap [void:public]
 * @param i [int] the (0-offset) index of the taxon in question
 * @param j [int] the (0-offset) index of the character in question
 *
 * Sets state stored at data[i][j] to the gap state.
 * Assumes i is in the range [0..nrows) and j is in the range
 * [0..ncols).
 */
void DiscreteMatrix::SetGap( int i, int j )
{
	assert( i >= 0 );
	assert( i < nrows );
	assert( j >= 0 );
	assert( j < ncols );
	assert( data != NULL );

	SetGap( data[i][j] );
}

/**
 * @method SetGap [void:private]
 * @param d [DiscreteDatum&] the datum in question
 *
 * Assigns the gap state to d, erasing any previously stored information.
 * The gap state is designated internally as a states array one element
 * long, with the single element set to the value 0.
 */
void DiscreteMatrix::SetGap( DiscreteDatum& d )
{
   if( d.states != NULL )
      delete [] d.states;
   d.states = new int[1];
   d.states[0] = 0;
}

/**
 * @method SetMissing [void:public]
 * @param i [int] the (0-offset) index of the taxon in question
 * @param j [int] the (0-offset) index of the character in question
 *
 * Sets state stored at data[i][j] to the missing state.
 * Assumes i is in the range [0..nrows) and j is in the range
 * [0..ncols).
 */
void DiscreteMatrix::SetMissing( int i, int j )
{
	assert( i >= 0 );
	assert( i < nrows );
	assert( j >= 0 );
	assert( j < ncols );
	assert( data != NULL );

   SetMissing( data[i][j] );
}

/**
 * @method SetMissing [void:private]
 * @param d [DiscreteDatum&] the datum in question
 *
 * Assigns the missing state to d, erasing any previously stored information.
 * The missing state is stored internally as a NULL value for the states array.
 */
void DiscreteMatrix::SetMissing( DiscreteDatum& d )
{
	if( d.states != NULL )
		delete [] d.states;
	d.states = NULL;
}

/**
 * @method SetPolymorphic [void:public]
 * @param value [int] specify 1 if taxon at row i is polymorphic at character in column j, 0 if uncertain which state applies
 *
 * Sets polymorphism state of taxon i and character j to value.  Value is
 * 1 by default, so calling SetPolymorphic(i, j) with no value specified
 * will set polymorphic to 1 in data[i][j]. Assumes i is in the range [0..nrows)
 * and j is in the range [0..ncols).  Also assumes that the number of states
 * stored is greater than 1.
 */
void DiscreteMatrix::SetPolymorphic( int i, int j, int value /* = 1 */ )
{
	assert( i >= 0 );
	assert( i < nrows );
	assert( j >= 0 );
	assert( j < ncols );
	assert( data != NULL );
	assert( value == 0 || value == 1 );

   SetPolymorphic( data[i][j], value );
}

/**
 * @method SetPolymorphic [void:private]
 * @param d [DiscreteDatum&] the datum in question
 * @param value [int] specify 1 if polymorphic, 0 if uncertain
 *
 * Sets the polymorphism cell (last cell in d.states) to value.
 * Warning: has no effect if there are fewer than 2 states stored!
 */
void DiscreteMatrix::SetPolymorphic( DiscreteDatum& d, int value )
{
   if( d.states == NULL || d.states[0] < 2 )
      return;

   int nstates = d.states[0];
   int ncells = nstates + 2;
   d.states[ncells-1] = value;
}

/**
 * @method SetState [long:public]
 * @param i [int] the (0-offset) index of the taxon in question
 * @param j [int] the (0-offset) index of the character in question
 * @param stateSymbol [char] the state to assign
 *
 * Sets state of taxon row and character col to stateSymbol.
 * Assumes i is in the range [0..nrows) and j is in the range [0..ncols).
 * Also assumes both data and symbols are non-NULL.  Finally, it is also
 * assumed that this function will not be called if stateSymbol represents
 * the missing or gap state, in which case the functions SetMissing or
 * SetGap, respectively, should be called instead.
 */
void DiscreteMatrix::SetState( int i, int j, int value )
{
	assert( i >= 0 );
	assert( i < nrows );
	assert( j >= 0 );
	assert( j < ncols );
	assert( data != NULL );

   SetState( data[i][j], value );
}

/**
 * @method SetState [void:private]
 * @param d [DiscreteDatum&] the datum in question
 * @param value [int] the value to assign for the state
 *
 * Assigns value to the 2nd cell in states (1st cell in states
 * array set to 1 to indicate that there is only one state).
 * Warning: if there are already one or more states (including
 * the gap state) assigned to d, they will be forgotten.  Use
 * the function AddState if you want to preserve states already
 * stored in d.  Assumes state being set is not the missing state
 * nor the gap state; use SetMissing or SetGap, respectively, to
 * do this.
 */
void DiscreteMatrix::SetState( DiscreteDatum& d, int value )
{
	if( d.states != NULL )
		delete [] d.states;
	d.states = new int[2];
	d.states[0] = 1;
	d.states[1] = value;
}


