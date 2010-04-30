#ifndef __DISTANCESBLOCK_H
#define __DISTANCESBLOCK_H

class DistancesBlock : public NexusBlock
{
   TaxaBlock& taxa;

   int newtaxa;
   int ntax;
   int nchar;

   int diagonal;
   int interleave;
   int labels;

   int triangle;
   enum { upper = 1, lower = 2, both = 3 };

   char missing;

   DistanceDatum** matrix;
   int* taxonPos;

protected:
   void HandleDimensionsCommand( NexusToken& token );
   void HandleFormatCommand( NexusToken& token );
   void HandleMatrixCommand( NexusToken& token );
   int  HandleNextPass( NexusToken& token, int& offset );
   void HandleTaxlabelsCommand( NexusToken& token );
   void Read( NexusToken& token );
   void Reset();

public:
   DistancesBlock( TaxaBlock& t );
   ~DistancesBlock();

   double GetDistance( int i, int j );
   char   GetMissingSymbol();
   int    GetNchar();
   int    GetNtax();
   int    GetTriangle();
   int    IsBoth();
   int    IsDiagonal();
   int    IsInterleave();
   int    IsLabels();
   int    IsLowerTriangular();
   int    IsMissing( int i, int j );
   int    IsUpperTriangular();
	void   Report( ostream& out );
   void   SetDistance( int i, int j, double d );
   void   SetMissing( int i, int j );
   void   SetNchar( int i );
};

#endif

