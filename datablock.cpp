#include "nexusdefs.h"
#include "discretedatum.h"
#include "discretematrix.h"
#include "nexustoken.h"
#include "nexus.h"
#include "taxablock.h"
#include "charactersblock.h"
#include "datablock.h"

/**
 * @class      DataBlock
 * @file       datablock.h
 * @file       datablock.cpp
 * @author     Paul O. Lewis
 * @copyright  Copyright © 1999. All Rights Reserved.
 * @see        Association
 * @see        AssocList
 * @see        CharactersBlock
 * @see        DiscreteDatum
 * @see        DiscreteMatrix
 * @see        LabelList
 * @see        LabelListAssoc
 * @see        LabelListBag
 * @see        Nexus
 * @see        NexusBlock
 * @see        NexusReader
 * @see        NexusToken
 * @see        SetReader
 * @see        TaxaBlock
 *
 * This class handles reading and storage for the Nexus block DATA.
 * It is derived from the CharactersBlock class, and differs from
 * CharactersBlock only in name and the fact that newtaxa is initially
 * true rather than false.
 */

/**
 * @constructor
 *
 * Performs the following initializations:
 * <table>
 * <tr><th align="left">Variable <th> <th align="left"> Initial Value
 * <tr><td> id             <td>= <td> "DATA"
 * <tr><td> newtaxa        <td>= <td> true
 * </table>
 */
DataBlock::DataBlock( TaxaBlock& tb, AssumptionsBlock& ab )
	: CharactersBlock( tb, ab )
{
	id = "DATA";
	newtaxa = true;
}

/**
 * @method Reset [void:protected]
 *
 * Calls Reset function of the parent class (CharactersBlock) and
 * resets newtaxa to true in preparation for reading another DATA block.
 */
void DataBlock::Reset()
{
	CharactersBlock::Reset();
	newtaxa = true;
   taxa.Reset();
}


