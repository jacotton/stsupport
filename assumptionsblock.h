#ifndef __ASSUMPTIONSBLOCK_H
#define __ASSUMPTIONSBLOCK_H

class AssumptionsBlock : public NexusBlock
{
   // Adding a new data member? Don't forget to:
   // 1. Describe it in the class header comment at the top of "emptyblock.cpp"
   // 2. Initialize it (unless it is self-initializing) in the constructor
   //    and reinitialize it in the Reset function
   // 3. Describe the initial state in the constructor documentation
   // 4. Delete memory allocated to it in both the destructor and Reset function
   // 5. Report it in some way in the Report function

	TaxaBlock& taxa;
   CharactersBlock* charBlockPtr;

protected:
   IntSetMap charsets;
   IntSetMap taxsets;
   IntSetMap exsets;

   nxsstring def_charset;
   nxsstring def_taxset;
   nxsstring def_exset;

protected:
   void HandleCharset( NexusToken& token );
   void HandleEndblock( NexusToken& token );
   void HandleExset( NexusToken& token );
   void HandleTaxset( NexusToken& token );
	virtual void Read( NexusToken& token );
	virtual void Reset();
	virtual int TaxonLabelToNumber( nxsstring s );

public:
	AssumptionsBlock( TaxaBlock& t );
   virtual ~AssumptionsBlock();

   void SetCallback( CharactersBlock* p );

   int       GetNumCharSets();
   void      GetCharSetNames( LabelList& names );
   IntSet&   GetCharSet( nxsstring nm );
   nxsstring GetDefCharSetName();

   int       GetNumTaxSets();
   void      GetTaxSetNames( LabelList& names );
   IntSet&   GetTaxSet( nxsstring nm );
   nxsstring GetDefTaxSetName();

   int       GetNumExSets();
   void      GetExSetNames( LabelList& names );
   IntSet&   GetExSet( nxsstring nm );
   nxsstring GetDefExSetName();
   void      ApplyExSet( nxsstring nm );

	virtual void Report( ostream& out );
};

#endif

