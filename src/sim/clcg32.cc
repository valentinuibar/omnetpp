//==========================================================================
//  CLCG32.CC - part of
//                         OMNeT++
//              Discrete System Simulation in C++
//
// Contents:
//   class cLCG32
//
//==========================================================================

/*--------------------------------------------------------------*
  Copyright (C) 2002-2004 Andras Varga

  This file is distributed WITHOUT ANY WARRANTY. See the file
  `license' for details on this and other legal matters.
*--------------------------------------------------------------*/

#include "cenvir.h"
#include "defs.h"
#include "util.h"
#include "crng.h"
#include "cexception.h"


#define LCG32_MAX  0x7ffffffeL  /* = 2^31-2 */

/**
 * Implements a 32-bit (2^31-2 cycle length) linear congruential random
 * number generator.
 *    - Range:              1 ... 2^31-2
 *    - Period length:      2^31-2
 *    - Method:             x=(x * 7^5) mod (2^31-1)
 *    - Required hardware:  exactly 32-bit integer aritmetics
 *    - To check:           if  x[0]=1  then  x[10000]=1,043,618,065
 *
 * Source:  Raj Jain: The Art of Computer Systems Performance Analysis
 * (John Wiley & Sons, 1991) pp 441-444, 455.
 */
class SIM_API cLCG32 : public cRNG
{
  protected:
    long seed;

  public:
    cLCG32() {}
    virtual ~cLCG32() {}

    /**
     * Sets up the RNG.
     */
    virtual void initialize(int runnumber, int id, cConfiguration *cfg);

    /** Random integer in the range [0,intRandMax()] */
    virtual unsigned long intRand();

    /** Maximum value that can be returned by intRand() */
    virtual unsigned long intRandMax();

    /** Random integer in [0,n], n < intRandMax() */
    virtual unsigned long intRand(unsigned long n);

    /** Random double on the [0,1) interval */
    virtual double doubleRand();

    /** Random double on the (0,1) interval */
    virtual double doubleRandNonz();

    /** Random double on the [0,1] interval */
    virtual double doubleRandIncl1();
};

Register_Class(cLCG32);

void cLCG32::initialize(int runnumber, int id, cConfiguration *cfg)
{
    seed = 0; //FIXME
}

unsigned long cLCG32::intRand()
{
    const long int a=16807, q=127773, r=2836;
    seed=a*(seed%q) - r*(seed/q);
    if (seed<=0) seed+=LCG32_MAX+1;
    return seed-1; // shift range [1..2^31-2] to [0..2^31-3]
}

unsigned long cLCG32::intRandMax()
{
    return LCG32_MAX-1; // 2^31-3
}

unsigned long cLCG32::intRand(unsigned long n)
{
    // code from MersenneTwister.h, Richard J. Wagner rjwagner@writeme.com

    // Find which bits are used in n
    // Optimized by Magnus Jonsson (magnus@smartelectronix.com)
    unsigned long used = n;
    used |= used >> 1;
    used |= used >> 2;
    used |= used >> 4;
    used |= used >> 8;
    used |= used >> 16;

    // Draw numbers until one is found in [0,n]
    unsigned long i;
    do
        i = intRand() & used;  // toss unused bits to shorten search
    while( i > n );
    return i;
}

double cLCG32::doubleRand()
{
    return (double)intRand() * (1.0/LCG32_MAX);
}

double cLCG32::doubleRandNonz()
{
    return (double)(intRand()+1) * (1.0/(LCG32_MAX+1));
}

double cLCG32::doubleRandIncl1()
{
    return (double)intRand() * (1.0/(LCG32_MAX-1));
}


