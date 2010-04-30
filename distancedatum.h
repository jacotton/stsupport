#ifndef __DISTANCEDATUM_H
#define __DISTANCEDATUM_H

class DistanceDatum
{
   double value;
   int missing;

   friend class DistancesBlock;

public:
   DistanceDatum();
   ~DistanceDatum();
};

#endif
