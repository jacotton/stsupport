#ifndef __TAXABLOCK_H
#define __TAXABLOCK_H

//
// TaxaBlock class
//
class TaxaBlock : public NexusBlock
{
	friend class DataBlock;
	friend class AllelesBlock;
	friend class CharactersBlock;
	friend class DistancesBlock;

   // Adding a new data member? Don't forget to:
   // 1. Describe it in the class header comment at the top of "taxablock.cpp"
   // 2. Initialize it (unless it is self-initializing) in the constructor
   //    and reinitialize it in the Reset function
   // 3. Describe the initial state in the constructor documentation
   // 4. Delete memory allocated to it in both the destructor and Reset function
   // 5. Report it in some way in the Report function

	int ntax;
	LabelList taxonLabels;

public:
   class nosuchtaxon {}; // exception potentially thrown by FindTaxon

private:
   void SetNtax( int n );

protected:
	void Read( NexusToken& token );
	void Reset();

public:
	TaxaBlock();
   virtual ~TaxaBlock();

   void  AddTaxonLabel( nxsstring s );
   void  ChangeTaxonLabel( int i, nxsstring s );
	int   FindTaxon( nxsstring label );
   bool  IsAlreadyDefined( nxsstring label );
   int   GetMaxTaxonLabelLength();
	int   GetNumTaxonLabels();
	nxsstring GetTaxonLabel( int i );
	void  Report( ostream& out );
};

#endif
