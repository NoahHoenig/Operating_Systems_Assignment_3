/*
 *  Helper bit functions
 *  CS480-1001
 *  4-4-2023
 *  @author  Noah Hoenig - Gabe Nelson
 *  @redID   824412349 - 823746450
 */
#include "bit_operations.h"
#include <cmath>

unsigned int calculateBitmask(unsigned int numBits,int totalBits){
    unsigned int aMask = 1;
    for(unsigned int b = 1; b < numBits; b++)
    {
        aMask = aMask << 1;
        aMask = aMask | 1;
    }   
    int leftShift = totalBits - numBits;
    aMask = aMask << leftShift;
    return aMask;
}
unsigned int calculateBitShift(unsigned int numBits,int totalBits){
    return totalBits - numBits;
}

int calculateEntryCount(int numBits){
    return pow(2,numBits);
}