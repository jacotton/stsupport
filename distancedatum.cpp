#include "distancedatum.h"

/**
 * @class      DistanceDatum
 * @file       distancedatum.h
 * @file       distancedatum.cpp
 * @author     Paul O. Lewis
 * @copyright  Copyright © 1999. All Rights Reserved.
 * @variable   value [double:private] the pairwise distance value stored
 * @see        DistancesBlock
 *
 * This class stores pairwise distance values. It has no public access functions,
 * reflecting the fact that it is manipulated strictly by its only friend class,
 * the DistancesBlock class.
 */

/**
 * @constructor
 *
 * Default constructor. Initializes value to 0.0 and distanceFlags to 0.
 */
DistanceDatum::DistanceDatum() : missing(1), value(0.0)
{
}

/**
 * @destructor
 *
 * Does nothing.
 */
DistanceDatum::~DistanceDatum()
{
}

