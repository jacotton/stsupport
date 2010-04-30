#ifndef __SETREADER_H
#define __SETREADER_H

class SetReader
{
   NexusBlock& block;
   NexusToken& token;
   IntSet& nxsset;
   int max;
   int settype;

public:
   enum { generic = 1, charset, taxset }; // used for settype

private:
   int GetTokenValue();

protected:
   bool AddRange( int first, int last, int modulus = 0 );

public:
   SetReader( NexusToken& t, int maxValue, IntSet& iset, NexusBlock& nxsblk, int type );

   bool Run();
};

#endif
