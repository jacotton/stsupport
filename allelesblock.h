#ifndef __ALLELESBLOCK_H
#define __ALLELESBLOCK_H

#define MAX_ALLELES 255

class AllelesBlock : public CharactersBlock
{
   // Adding a new data member? Don't forget to:
   // 1. Describe it in the class header comment at the top of "allelesblock.cpp"
   // 2. Initialize it (unless it is self-initializing) in the constructor
   //    and reinitialize it in the Reset function
   // 3. Describe the initial state in the constructor documentation
   // 4. Delete memory allocated to it in both the destructor and Reset function
   // 5. Report it in some way in the Report function
protected:
	virtual void Read( NexusToken& token );
	virtual void Reset();

public:
   enum datapoints { standard = 1, fraglen };

public:
	class XAllMissingData {};
	
protected:
	IntSet haploid;

private:
   bool alleles_fixed;
   int* indivCount;
	datapoints datapoint;

protected:
	virtual void DebugShowMatrix( ostream& out, char* marginText = NULL );
	virtual void HandleFormat( NexusToken& token );
	virtual bool HandleNextGenotype( NexusToken& token, int i, int k
   	, bool stopOnNewline = true );
   virtual int  HandleAllele( NexusToken& token, int c );
   void HandleHaploid( NexusToken& token );
	virtual void HandleStdMatrix( NexusToken& token );
	virtual void HandleTransposedMatrix( NexusToken& token );
	virtual void HandleMatrix( NexusToken& token );
   int SplitInt( int x, int y );

public:
	AllelesBlock( TaxaBlock& tb, AssumptionsBlock& ab );
   virtual ~AllelesBlock();

	virtual void Report( ostream& out );
	
	// Methods for reporting the data
	//
	nxsstring GetLocusLabel( int locus );
	nxsstring GetAlleleLabel( int locus, int allele );

   // Methods for obtaining information about the data
   //
	void FocalAlleleCount( int focal_allele, int locus, int pop
		, int& n_AA, int& n_Aa, int& n_aa );
   int MostCommonAllele( int locus, int pop = -1 );
   int AlleleCount( int allele, int locus, int pop = -1 );
   double AlleleFrequency( int allele, int locus, int pop = -1 );
   int GenotypeCount( int allele1, int allele2, int locus, int pop = -1 );

	int GetNumHaploid();
   bool IsHaploid( int i );
   bool IsHaploidOrig( int origLocusIndex );

	int NumberOfAlleles( int locus, int pop = -1 );
   int SampleSize( int locus, int pop = -1 );
	int GetIndivCount( int pop );
   int GetGene( int pop, int indiv, int locus, int gene );
};

#endif

