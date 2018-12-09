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

#include "Median.h"

Median::Median(){};

void Median::swap(float *n1, float *n2){
  
  float temp = *n1;
  *n1 = *n2;
  *n2 = temp;
  
  }

int Median::partition(float *values, int low, int high)
 {
    float pivot = values[high];
    int i = (low - 1);
    for (int j = low; j <= high - 1; j++)
    {
      if (values[j] <= pivot)
      {
        i++;
        Median::swap(&values[i], &values[j]);
        }
    }
    Median::swap(&values[i + 1], &values[high]);
    return (i + 1);
  } 
   
void Median::sortascend(float *values, int low, int high)
{
  
  if (low < high){
      int p = partition(values, low, high);
      Median::sortascend(values, low, p - 1);  
      Median::sortascend(values, p + 1, high);       
    }
   
 }

float Median::median(float *values, int low, int high){

    
    float m;
    Median::sortascend(values, low, high);
    
    if ((high) % 2 == 0)
    {//even
        m = values[round (high / 2)];
      
      } 


     else{//odd
        float a = values[round (high / 2)];
        float b = values[round (high / 2) + 1];
        m = (a + b) / 2;
        
      }
  
    return m;
  }
  
  
  
