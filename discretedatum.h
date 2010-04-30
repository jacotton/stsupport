#ifndef __DISCRETEDATUM_H
#define __DISCRETEDATUM_H

//
// DiscreteDatum class
//
class DiscreteDatum
{
   int* states;

   friend class DiscreteMatrix;

public:

   DiscreteDatum();
   ~DiscreteDatum();

	void CopyFrom( const DiscreteDatum& other );
};

#endif
