#ifndef __CHARACTERSBLOCK_H
#define __CHARACTERSBLOCK_H

//
// CharactersBlock class
//
class CharactersBlock : public NexusBlock
{
   // Adding a new data member? Don't forget to:
   // 1. Describe it in the class header comment at the top of "charactersblock.cpp"
   // 2. Initialize it (unless it is self-initializing) in the constructor
   //    and reinitialize it in the Reset function
   // 3. Describe the initial state in the constructor documentation
   // 4. Delete memory allocated to it in both the destructor and Reset function
   // 5. Report it in some way in the Report function

friend class AssumptionsBlock;

protected:
	TaxaBlock& taxa;
	AssumptionsBlock& assumptionsBlock;

	int ntax;
	int ntaxTotal;
	int nchar;
	int ncharTotal;

	bool newtaxa;
	bool newchar;

	bool respectingCase;
	bool transposing;
	bool interleaving;
	bool tokens;
	bool labels;

	char missing;
	char gap;
	char matchchar;

	char* symbols;

	AssocList equates;

	DiscreteMatrix* matrix;
	int* charPos;
	int* taxonPos;
	IntSet eliminated;

	bool* activeChar;
	bool* activeTaxon;

	LabelList charLabels;
	LabelListBag charStates;

public:
	enum datatypes { standard = 1, dna, rna, nucleotide, protein, continuous };

private:
	datatypes datatype;

protected:
	void BuildCharPosArray( bool check_eliminated = false );
   int  IsInSymbols( char ch );
	void HandleCharlabels( NexusToken& token );
	void HandleCharstatelabels( NexusToken& token );
	void HandleDimensions( NexusToken& token, nxsstring newtaxaLabel, nxsstring ntaxLabel, nxsstring ncharLabel );
	void HandleEliminate( NexusToken& token );
	void HandleEndblock( NexusToken& token, nxsstring charToken );
	virtual void HandleFormat( NexusToken& token );
	virtual void HandleMatrix( NexusToken& token );
	virtual bool HandleNextState( NexusToken& token, int i, int c );
	void HandleStatelabels( NexusToken& token );
	virtual void HandleStdMatrix( NexusToken& token );
	void HandleTaxlabels( NexusToken& token );
   virtual int  HandleTokenState( NexusToken& token, int c );
	virtual void HandleTransposedMatrix( NexusToken& token );
   int  PositionInSymbols( char ch );
	virtual void Read( NexusToken& token );
	virtual void Reset();
	void ResetSymbols();
   void ShowStates( ostream& out, int i, int j );
   void WriteStates( DiscreteDatum& d, char* s, int slen );

public:
	CharactersBlock( TaxaBlock& tb, AssumptionsBlock& ab );
	virtual ~CharactersBlock();

	int ApplyDelset( IntSet& delset );
	int ApplyExset( IntSet& exset );
	int ApplyIncludeset( IntSet& inset );
	int ApplyRestoreset( IntSet& restoreset );
   virtual int CharLabelToNumber( nxsstring s );
   virtual int TaxonLabelToNumber( nxsstring s );

	virtual void DebugShowMatrix( ostream& out, char* marginText = NULL );
	nxsstring GetCharLabel( int i );
   int   GetCharPos( int origCharIndex );
   int   GetTaxPos( int origTaxonIndex );
	int   GetDataType();
	char  GetGapSymbol();
   int   GetInternalRepresentation( int i, int j, int k = 0 );
	char  GetMatchcharSymbol();
   virtual int GetMaxObsNumStates();
	char  GetMissingSymbol();
	int   GetNTax();
	int   GetNChar();
	int   GetNCharTotal();
	int   GetNTaxTotal();
	int 	GetNumActiveChar();
	int 	GetNumActiveTaxa();
   int   GetNumEliminated();
   int   GetNumEquates();
	int   GetNumMatrixCols();
	int   GetNumMatrixRows();
	int   GetNumStates( int i, int j );
	virtual int GetObsNumStates( int j );
   int   GetOrigCharIndex( int j );
   int   GetOrigCharNumber( int j );
   int   GetOrigTaxonIndex( int j );
   int   GetOrigTaxonNumber( int j );
	char  GetState( int i, int j, int k = 0 );
	nxsstring GetStateLabel( int i, int j );
	char* GetSymbols();
   nxsstring GetTaxonLabel( int i );
	bool  IsGapState( int i, int j );
	bool  IsInterleave();
	bool  IsLabels();
	bool  IsMissingState( int i, int j );
	bool  IsPolymorphic( int i, int j );
	bool  IsRespectCase();
	bool  IsTokens();
	bool  IsTranspose();

	bool  IsEliminated( int origCharIndex );

	void  ExcludeCharacter( int i );
	void  IncludeCharacter( int i );
	bool  IsActiveChar( int j );
	bool  IsExcluded( int j );
	
	void  DeleteTaxon( int i );
	void  RestoreTaxon( int i );
	bool  IsActiveTaxon( int i );
	bool  IsDeleted( int i );
	
	bool* GetActiveTaxonArray();
	bool* GetActiveCharArray();

	virtual void  Report( ostream& out );
   void  ShowStateLabels( ostream& out, int i, int c );
};

#endif
