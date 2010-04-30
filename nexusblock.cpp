#include "nexusdefs.h"
#include "nexustoken.h"
#include "nexus.h"

/**
 * @class      NexusBlock
 * @file       nexus.h
 * @file       nexusblock.cpp
 * @author     Paul O. Lewis
 * @copyright  Copyright © 1999. All Rights Reserved.
 * @variable   isDisabled [bool:private] true if this block is currently disabled
 * @variable   isEmpty [bool:private] true if this object is currently storing data
 * @variable   next [NexusBlock*:protected] pointer to next block in list
 * @variable   id [nxsstring:protected] holds name of block (e.g., "DATA", "TREES", etc.)
 * @see        Nexus
 * @see        NexusReader
 * @see        NexusToken
 *
 * This is the base class from which all Nexus block classes are derived.
 * The abstract virtual function Read must be overridden for each derived
 * class to provide the ability to read everything following the block name
 * (which is read by the Nexus object) to the end or endblock statement.
 * Derived classes must provide their own data storage and access functions.
 */

/**
 * @constructor
 *
 * Default constructor
 * Initializes 'next' and 'nexus' data members to NULL, and 'isEmpty' and
 * 'isEnabled' to true.
 */
NexusBlock::NexusBlock() : next(NULL), nexus(NULL), isEmpty(true), isEnabled(true)
{
}

/**
 * @destructor
 *
 * Does nothing.
 */
NexusBlock::~NexusBlock()
{
}

/**
 * @method CharLabelToNumber [int:protected]
 * @param s [nxsstring] the character label to be translated to character number
 *
 * This base class version simply returns -1, but a derived class should
 * override this function if it needs to construct and run a SetReader
 * object to read a set involving characters.  The SetReader object may
 * need to use this function to look up a character label encountered in
 * the set.  A class that overrides this method should return the
 * character index in the range [1..nchar]; i.e., add one to the 0-offset
 * index.
 */
int NexusBlock::CharLabelToNumber( nxsstring /*s*/ )
{
   return 0;
}

/**
 * @method Disable [void:public]
 *
 * Sets the value of isEnabled to false.  A NexusBlock
 * can be disabled (by calling this method) if blocks of that type 
 * are to be skipped during execution of the nexus file.  
 * If a disabled block is encountered, the virtual 
 * Nexus::SkippingDisabledBlock function is called.
 */
void NexusBlock::Disable()
{
   isEnabled = false;
}

/**
 * @method Enable [void:public]
 *
 * Sets the value of isEnabled to true.  A NexusBlock
 * can be disabled (by calling Disable) if blocks of that type 
 * are to be skipped during execution of the nexus file.  
 * If a disabled block is encountered, the virtual 
 * Nexus::SkippingDisabledBlock function is called.
 */
void NexusBlock::Enable()
{
   isEnabled = true;
}

/**
 * @method IsEnabled [bool:public]
 *
 * Returns value of isEnabled, which can be controlled through
 * use of the Enable and Disable member functions.  A NexusBlock
 * should be disabled if blocks of that type are to be skipped
 * during execution of the nexus file.  If a disabled block is
 * encountered, the virtual Nexus::SkippingDisabledBlock function
 * is called.
 */
bool NexusBlock::IsEnabled()
{
   return isEnabled;
}

/**
 * @method IsEmpty [bool:public]
 *
 * Returns true if Read function has not been called since the last Reset.
 * This base class version simply returns the value of the data member
 * isEmpty.  If you derive a new block class from NexusBlock, be sure
 * to set isEmpty to true in your Reset function and isEmpty to false in your
 * Read function.
 */
bool NexusBlock::IsEmpty()
{
   return isEmpty;
}

/**
 * @method Read [virtual void:protected]
 * @param token [NexusToken&] the NexusToken to use for reading block
 * @param in [istream&] the input stream from which to read
 *
 * This abstract virtual function must be overridden for each derived
 * class to provide the ability to read everything following the block name
 * (which is read by the Nexus object) to the end or endblock statement.
 * Characters are read from the input stream in.  Note that to get output
 * comments displayed, you must derive a class from NexusToken, override
 * the member function OutputComment to display a supplied comment, and
 * then pass a reference to an object of the derived class to this function.
 */
//	virtual void Read( NexusToken& token, istream& in ) = 0;

/**
 * @method Reset [virtual void:protected]
 *
 * This abstract virtual function must be overridden for each derived
 * class to completely reset the block object in preparation for reading
 * in another block of this type.  This function is called by the Nexus
 * object just prior to calling the block object's Read function.
 */
//	virtual void Reset() = 0;

/**
 * @method GetID [nxsstring:public]
 *
 * Returns the id nxsstring.
 */
nxsstring NexusBlock::GetID()
{
	return id;
}

/**
 * @method Report [virtual void:public]
 * @param out [ostream&] the output stream to which the report is sent
 *
 * Provides a brief report of the contents of the block.
 */
//	virtual void Report( ostream& out ) = 0;

/**
 * @method SetNexus [virtual void:public]
 * @param nxsptr [Nexus*] pointer to a Nexus object
 *
 * Sets the nexus data member of the NexusBlock object to
 * nxsptr.
 */
void NexusBlock::SetNexus( Nexus* nxsptr )
 {
 	nexus = nxsptr;
 }
 
/**
 * @method SkippingCommand [virtual void:public]
 * @param commandName [nxsstring] the name of the command being skipped
 *
 * This function is called when an unknown command named commandName is
 * about to be skipped.  This version of the function does nothing (i.e.,
 * no warning is issued that a command was unrecognized.  Override this
 * virtual function in a derived class to provide such warnings to the
 * user.
 */
void NexusBlock::SkippingCommand( nxsstring /* commandName */ )
{
}

/**
 * @method TaxonLabelToNumber [int:protected]
 * @param s [nxsstring] the taxon label to be translated to a taxon number
 *
 * This base class version simply returns 0, but a derived class should
 * override this function if it needs to construct and run a SetReader
 * object to read a set involving taxa.  The SetReader object may
 * need to use this function to look up a taxon label encountered in
 * the set.  A class that overrides this method should return the
 * taxon index in the range [1..ntax]; i.e., add one to the 0-offset
 * index.
 */
int NexusBlock::TaxonLabelToNumber( nxsstring /*s*/ )
{
   return 0;
}


