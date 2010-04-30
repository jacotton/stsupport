#ifndef __DATABLOCK_H
#define __DATABLOCK_H

//
// DataBlock class
//
class DataBlock : public CharactersBlock
{
	protected:
		void Reset();

	public:
		DataBlock( TaxaBlock& tb, AssumptionsBlock& ab );
};

#endif
