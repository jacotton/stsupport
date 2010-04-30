#include "nexusdefs.h"
#include "xnexus.h"
#include "nexustoken.h"
#include "nexus.h"

#define NCL_NAME_AND_VERSION  "NCL version 2.01"
#define NCL_COPYRIGHT         "Copyright (c) 2000 by Paul O. Lewis"
#ifdef __BORLANDC__
	#define NCL_HOMEPAGEURL       "http:\/\/lewis.eeb.uconn.edu\/lewishome\/software.html"
#else
	#define NCL_HOMEPAGEURL       "http://lewis.eeb.uconn.edu/lewishome/software.html"
#endif

/**
 * @class      Nexus
 * @file       nexus.h
 * @file       nexus.cpp
 * @author     Paul O. Lewis
 * @copyright  Copyright © 1999. All Rights Reserved.
 * @variable   blockList [NexusBlock*:protected] pointer to first block in list of blocks
 * @see        NexusBlock
 * @see        NexusReader
 * @see        NexusToken
 * @see        XNexus
 *
 * This is the class that orchestrates the reading of a Nexus data file.
 * An object of this class should be created, and objects of any block classes
 * that are expected to be needed should be added to blockList using the Add
 * member function. The Execute member function is then called, which reads the
 * data file until encountering a block name, at which point the correct block
 * is looked up in blockList and that object's Read method called.
 */

/**
 * @constructor
 *
 * Default constructor
 * Initializes the blockList data member to NULL.
 */
Nexus::Nexus() : blockList(NULL)
{
}

/**
 * @destructor
 *
 * Does nothing.
 */
Nexus::~Nexus()
{
}

/**
 * @method Add [void:public]
 * @param newBlock [NexusBlock*] a pointer to an existing block object
 *
 * Adds newBlock to the end of the list of NexusBlock objects growing
 * from blockList.  If blockList points to NULL, this function sets
 * blockList to point to newBlock. Calls SetNexus method of newBlock
 * to inform newBlock of the Nexus object that now owns it.  This 
 * is useful when the newBlock object needs to communicate with the
 * outside world through the Nexus object, such as when it issues
 * progress reports as it is reading the contents of its block.
 */
void Nexus::Add( NexusBlock* newBlock )
{
	assert( newBlock != NULL );

	newBlock->SetNexus(this);
	if( !blockList )
		blockList = newBlock;
	else {
		// add new block to end of list
		NexusBlock* curr;
		for( curr = blockList; curr && curr->next; )
			curr = curr->next;
		assert( curr && !curr->next );
		curr->next = newBlock;
	}
}

/**
 * @method BlockListEmpty [bool:public]
 *
 * If blockList data member still equals NULL, returns true;
 * otherwise, returns false.  The blockList will not be equal
 * to NULL if the Add function has been called to add a block
 * object to the list.
 */
bool Nexus::BlockListEmpty()
{
	return ( blockList == NULL ? true : false );
}

/**
 * @method DebugReportBlock [virtual void:protected]
 * @param nexusBlock [NexusBlock&] the block that should be reported
 *
 * This function was created for purposes of debugging a new NexusBlock.
 * This version does nothing; to create an active DebugReportBlock function,
 * override this version in the derived class and call the Report function
 * of nexusBlock.  This function is called whenever the main Nexus Execute function
 * encounters the [&spillall] command comment between blocks in the data file.
 * The Execute function goes through all blocks and passes them, in turn, to this
 * DebugReportBlock function so that their contents are displayed. Placing the
 * [&spillall] command comment between different versions of a block allows
 * multiple blocks of the same type to be tested using one long data file.
 * Say you are interested in testing whether the normal, transpose, and
 * interleave format of a matrix can all be read correctly.  If you put three
 * versions of the block in the data file one after the other, the second one
 * will wipe out the first, and the third one will wipe out the second, unless
 * you have a way to report on each one before the next one is read.  This
 * function provides that ability.
 */
void Nexus::DebugReportBlock( NexusBlock& /* nexusBlock */ )
{
   // Derive me and uncomment the following line in the derived version
   // nexusBlock.Report(out);
	// Note that your derived Nexus object must have its own ostream object (out)
}

/**
 * @method Detach [void:public]
 * @param oldBlock [NexusBlock*] a pointer to an existing block object
 *
 * Detaches oldBlock from the list of NexusBlock objects growing
 * from blockList.  If blockList itself points to oldBlock, this
 * function sets blockList to point to oldBlock->next.
 * Note: oldBlock is not deleted, it is simple detached from the
 * linked list. No harm is done in Detaching a block pointer
 * that has already been detached previously; if oldBlock is
 * not found in the block list, Detach simply returns quietly.
 * If oldBlock is found, its SetNexus object is called to set
 * the Nexus pointer to NULL, indicating that it is no longer
 * owned by (i.e., attached to) a Nexus object.
 */
void Nexus::Detach( NexusBlock* oldBlock )
{
   assert( oldBlock != NULL );
   
	// Should call BlockListEmpty function first to make sure
	// there are blocks to detach
	//
   assert( blockList != NULL );
   
	if( blockList == oldBlock ) {
		blockList = oldBlock->next;
  	   oldBlock->SetNexus(NULL);
	}
	else {
		// try to find oldBlock in list and detach if found
		NexusBlock* curr = blockList;
		NexusBlock* currNext = blockList->next;
		for( ; currNext != NULL && currNext != oldBlock; ) {
			currNext = currNext->next;
		}
		if( currNext == oldBlock ) {
			curr->next = currNext->next;
   	   currNext->next = NULL;
   	   oldBlock->SetNexus(NULL);
      }
	}
}

/**
 * @method EnteringBlock [virtual void:public]
 * @param blockName [char*] the name of the block being entered
 *
 * This function is called when a block named blockName has just
 * been entered and is about to be read.  Override this pure virtual
 * function to provide an indication of progress as the Nexus file
 * is being read.
 */
//	virtual void EnteringBlock( char* blockName ) = 0;

/**
 * @method Execute [void:public]
 * @param token [NexusToken&] the token object used to grab Nexus tokens
 * @param notifyStartStop [bool] if true, ExecuteStarting and ExecuteStopping will be called (true by default)
 *
 * Reads the Nexus data file from the input stream provided by token.  This function
 * is responsible for reading through the name of a each block.  Once
 * it has read a block name, it searches blockList for a block object
 * to handle reading the remainder of the block's contents. The block
 * object is responsible for reading the end or endblock command as well
 * as the trailing semicolon.  This function also handles reading
 * comments that are outside of blocks, as well as the initial #NEXUS
 * keyword.  The notifyStartStop argument is provided in case you do not
 * wish the ExecuteStart and ExecuteStop functions to be called.  These functions
 * are primarily used for creating and destroying a dialog box to show progress,
 * and nested Execute calls can thus cause problems (e.g., a dialog box is
 * destroyed when the inner Execute calls ExecuteStop and the outer Execute
 * still expects the dialog box to be available).  Specifying notifyStartStop
 * false for all the nested Execute calls thus allows the outermost Execute call
 * to control creation and destruction of the dialog box.
 */
void Nexus::Execute( NexusToken& token, bool notifyStartStop /* = true */ )
{
	char id_str[256];
	
	bool disabledBlock = false;
	nxsstring errormsg;

   try {
   	token.GetNextToken();
   }
   catch( XNexus x ) {
      NexusError( token.errormsg, 0, 0, 0 );
      return;
   }
   
	if( !token.Equals("#NEXUS") ) {
      errormsg = "Expecting #NEXUS to be the first token in the file, but found ";
      errormsg += token.GetToken();
      errormsg += " instead";
		NexusError( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
      return;
	}
	
	if( notifyStartStop )
		ExecuteStarting();
	
	for(;;)
	{
		token.SetLabileFlagBit( NexusToken::saveCommandComments );

		token.GetNextToken();
		
		if( token.AtEOF() )
			break;

		if( token.Equals("BEGIN") )
		{
			disabledBlock = false;
			token.GetNextToken();

			NexusBlock* curr;
			for( curr = blockList; curr != NULL; curr = curr->next )
			{
				if( !token.Equals( curr->GetID() ) )
					continue;

				if( !curr->IsEnabled() ) {
					disabledBlock = true;
					SkippingDisabledBlock( token.GetToken() );
					continue;
				}

				strcpy( id_str, curr->GetID().c_str() );
				EnteringBlock( id_str /*curr->GetID()*/ );
            curr->Reset();
				try {
					curr->Read( token );
				}
				catch( XNexus x ) {
					NexusError( curr->errormsg, x.pos, x.line, x.col );
	            curr->Reset();
               return;
				}
				ExitingBlock( id_str /*curr->GetID()*/ );
				break;
			}

			if( curr == NULL )
			{
				token.BlanksToUnderscores();
            nxsstring currBlock = token.GetToken();
				if( !disabledBlock ) 
					SkippingBlock( currBlock );
				for(;;)
				{
					token.GetNextToken();
					if( token.Equals("END") || token.Equals("ENDBLOCK") ) {
						token.GetNextToken();
						if( !token.Equals(";") ) {
							errormsg = "Expecting ';' after END or ENDBLOCK command, but found ";
                     errormsg += token.GetToken();
                     errormsg += " instead";
							NexusError( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
                     return;
						}
						break;
					}
					if( token.AtEOF() ) {
						errormsg = "Encountered end of file before END or ENDBLOCK in block ";
                  errormsg += currBlock;
						NexusError( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
                  return;
					}
				}
			} // if token not found amongst known block IDs
		} // if token equals BEGIN

		else if( token.Equals("&SHOWALL") ) {

			NexusBlock* curr;
			for( curr = blockList; curr != NULL; curr = curr->next )
			{
				DebugReportBlock(*curr);
			}

		}

		else if( token.Equals("&LEAVE") ) {
         break;
		}

	} // for(;;)
	
	if( notifyStartStop )
		ExecuteStopping();
}

/**
 * @method NCLCopyrightNotice [char*:public]
 *
 * Returns a string containing the copyright notice
 * for the Nexus Class Library, useful for
 * reporting the use of this library by programs
 * that interact with the user.
 */
char* Nexus::NCLCopyrightNotice()
{
	return NCL_COPYRIGHT;
}

/**
 * @method NCLHomePageURL [char*:public]
 *
 * Returns a string containing the URL for the
 * Nexus Class Library Home Page on the World
 * Wide Web.
 */
char* Nexus::NCLHomePageURL()
{
	return NCL_HOMEPAGEURL;
}

/**
 * @method NCLNameAndVersion [char*:public]
 *
 * Returns a string containing the name and current
 * version of the Nexus Class Library, useful for
 * reporting the use of this library by programs
 * that interact with the user.
 */
char* Nexus::NCLNameAndVersion()
{
	return NCL_NAME_AND_VERSION;
}

/**
 * @method NexusError [virtual void:public]
 * @param msg [nxsstring&] the error message to be displayed
 * @param pos [streampos] the current file position
 * @param line [long] the current file line
 * @param col [long] the current column within the current file line
 *
 * This function is called whenever there is an error detected while reading
 * a Nexus file.  Override this pure virtual function to display the error
 * message in the most appropriate way for the platform you are supporting.
 */
//	virtual void NexusError( nxsstring& msg, streampos pos, long line, long col ) = 0;

/**
 * @method OutputComment [virtual void:public]
 * @param comment [nxsstring] a comment to be shown on the output
 *
 * This function may be used to report progess while reading through
 * a file. For example, the AllelesBlock class uses this function to
 * report the name of the population it is currently reading so the
 * user doesn't think the program has hung on large data sets.
 */
//	virtual void OutputComment( nxsstring comment ) = 0;
	
/**
 * @method SkippingBlock [virtual void:public]
 * @param blockName [nxsstring] the name of the block being skipped
 *
 * This function is called when an unknown block named blockName is
 * about to be skipped.  Override this pure virtual function to
 * provide an indication of progress as the Nexus file is being read.
 */
//	virtual void SkippingBlock( nxsstring blockName ) = 0;

/**
 * @method SkippingDisabledBlock [virtual void:public]
 * @param blockName [nxsstring] the name of the disabled block being skipped
 *
 * This function is called when a disabled block named blockName is
 * encountered in a NEXUS data file being executed. Override this pure
 * virtual function to handle this event in an appropriate manner.  For
 * example, the program may wish to inform the user that a data block
 * was encountered in what is supposed to be a tree file.
 */
//	virtual void SkippingDisabledBlock( nxsstring blockName ) = 0;


