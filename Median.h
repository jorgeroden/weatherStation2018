/*
  Median.cpp - Educational purpose.
  This library calculates the median value
  of a list of measurements stored in a vector.
  Quicksort algorith is implemented to sort 
  the measurements in ascending order. See link
  (https://en.wikipedia.org/wiki/Quicksort). 
  
  Created by Jorge Munoz, Dec the 2nd 2017.
  Released into the public domain.

*/

#ifndef Median_h
#define Median_h

#include "Arduino.h"

class Median
{
  public:
    Median();
    
    float median(float *values, int low, int high); 
    void sortascend(float *values, int low, int high);
    int partition(float *values, int low, int high);
    void swap(float *n1, float *n2);
  
    
};

#endif

