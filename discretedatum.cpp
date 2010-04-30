#include "nexusdefs.h"
#include "discretedatum.h"

/**
 * @class      DiscreteDatum
 * @file       discretedatum.h
 * @file       discretedatum.cpp
 * @author     Paul O. Lewis
 * @copyright  Copyright © 1999. All Rights Reserved.
 * @variable   polymorphic [int:public] if true, additional states represent polymorphism rather than uncertainty
 * @variable   states [int*:public] holds information about state
 * @see        DiscreteMatrix
 * @see        NexusReader
 *
 * Class for holding discrete states in a matrix.  Note that there is no way to access
 * the variables of this class since they are all private and there are no public
 * access functions.  This class is designed to be manipulated by the class 
 * <a href="DiscreteMatrix.html">DiscreteMatrix</a>, which is the only class that 
 * has been designated a friend of DiscreteDatum.
 *
 * <p>The variable states is NULL if there is missing data, and non-NULL for any other state.
 * If states is non-NULL, the first cell is used to store the number of states.  This will
 * be 0 if the state is the gap state, 1 if the state is unambiguous and nonpolymorphic
 * (and not the gap state of course), and 2 or higher if there is either polymorphism or
 * uncertainty.  If polymorphism or uncertainty apply, it becomes necessary to store information
 * about which of these two situations holds.  Thus, the last cell in the array is set to
 * either 1 (polymorphism) or 0 (uncertainty).  While a little complicated, this scheme has
 * the following benefits:
 * <ol>
 * <li> if the state is missing, the only memory allocated is for a pointer (states)
 * <li> if the state is unambiguous and not polymorphic, no storage is used for keeping
 *   track of whether polymorphism or uncertainty holds
 * <li> it allows for a virtually unlimited number of states, which is important if it is
 *   to be general enough to store microsatellite data for an AllelesBlock object, for
 *   example.
 * </ol>
 *
 * <p>Supposing the gap symbol is '-', the missing data symbol is '?', and the
 * symbols list is "ACGT", the following table shows the status of the states
 * variable under several different possible data matrix entries:
 * <table>
 * <tr> <th> Matrix entry <th> contents of states
 * <tr> <td align="center"> ?            <td align="center"> NULL
 * <tr> <td align="center"> -            <td align="center"> <table border=1> <tr> <td>0</td> </table>
 * <tr> <td align="center"> G            <td align="center"> <table border=1> <tr> <td>1</td> <td>2</td> </table>
 * <tr> <td align="center"> (AG) polymorphic <td align="center"> <table border=1> <tr> <td>2</td> <td>0</td> <td>2</td> <td>1</td> </table>
 * <tr> <td align="center"> {AG} ambiguous   <td align="center"> <table border=1> <tr> <td>2</td> <td>0</td> <td>2</td> <td>0</td> </table>
 * </table>
 */

/**
 * @constructor
 *
 * Sets states to NULL and polymorphic to 0.
 */
DiscreteDatum::DiscreteDatum()
{
   states = NULL;
}

/**
 * @destructor
 *
 * Deletes memory associated with states (if any was allocated).
 */
DiscreteDatum::~DiscreteDatum()
{
   if( states != NULL )
      delete [] states;
}

/**
 * @method CopyFrom [int:public]
 * @param other [const DiscreteDatum&] the source DiscreteDatum object 
 *
 * Makes this DiscreteDatum object an exact copy of other.  Useful for
 * dealing with matchchar symbols in a matrix.
 */
void DiscreteDatum::CopyFrom( const DiscreteDatum& other )
{
	if( states != NULL ) {
		delete [] states;
		states = NULL;
	}
	
	if( other.states == NULL )
		return;
		
	int sz = other.states[0];
	if( sz == 0 ) 
	{
		// other.states indicates the 'gap' state is present
		//
		states = new int[1];
		states[0] = 0;
	}
	else if( sz == 1 )
	{
		// other.states indicates state is unambiguous and non-polymorphic
		//
		states = new int[2];
		states[0] = 1;
		states[1] = other.states[1];
	}
	else
	{
		// other.states indicates ambiguity or polymorphism is present
		//
		states = new int[sz+2];
		states[0] = sz;
		for( int i = 1; i <= sz; i++ )
			states[i] = other.states[i];

		// copy the polymorphism indicator element
		//
		states[sz+1] = other.states[sz+1];
	}
}
