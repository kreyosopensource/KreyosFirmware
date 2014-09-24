//******************************************************************************
//
//   Description: This file contains a function to caluculate sine and cosine
//                using CORDIC rotation on a 16 bit MSP430. The function takes
//                any angle in degrees and outputs the answers in Q.15 fixed
//                point format[(floating point) = (fixed point)/ 2^15].
//
//                In creating this file, I referenced the 2004 presentation of
//                fixed point two's complement CORDIC arithmetic presentation
//                by Titi Trandafir of Microtrend Systems which contained an
//                assembly language program utilizing CORDIC. I also referenced
//                the CORDIC wikipedia page.
//
//   T. Brower
//   Version    1.00
//   Feb 2011
//     IAR Embedded Workbench Kickstart (Version: 5.20.1)
//
//Copyright (c) 2011 Theo Brower
//
//Permission is hereby granted, free of charge, to any person obtaining a
//copy of this software and associated documentation files (the "Software"),
//to deal in the Software without restriction, including without limitation
//the rights to use, copy, modify, merge, publish, distribute, sublicense,
//and/or sell copies of the Software, and to permit persons to whom the
//Software is furnished to do so, subject to the following conditions:
//The above copyright notice and this permission notice shall be included
//in all copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
//OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
//THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
//IN THE SOFTWARE.
//******************************************************************************

// Table of arctan's for use with CORDIC algorithm
// Store in decimal representation N = ((2^16)*angle_deg) / 180
#define ATAN_TAB_N 16
static const int atantable[ATAN_TAB_N] = {  0x4000,   //atan(2^0) = 45 degrees
                                0x25C8,   //atan(2^-1) = 26.5651
                                0x13F6,   //atan(2^-2) = 14.0362
                                0x0A22,   //7.12502
                                0x0516,   //3.57633
                                0x028B,   //1.78981
                                0x0145,   //0.895174
                                0x00A2,   //0.447614
                                0x0051,   //0.223808
                                0x0029,   //0.111904
                                0x0014,   //0.05595
                                0x000A,   //0.0279765
                                0x0005,   //0.0139882
                                0x0003,   //0.0069941
                                0x0002,   //0.0035013
                                0x0001    //0.0017485
};

// Function to computer sine/cosine using CORDIC
// Inputs:
//  theta = any (integer) angle in degrees
//  iterations =  number of iterations for CORDIC algorithm, up to 16,
//                the ideal value seems to be 13
//  *sin_result = pointer to where you want the sine result
//  *cos_result = pointer to where you want the cosine result

void cordic_sincos(int theta,
                   char iterations,
                   int *sin_result,
                   int *cos_result){
  int sigma, s, x1, x2, y, i, quadAdj, shift;
  const int *atanptr = atantable;

  //Limit iterations to number of atan values in our table
  iterations = (iterations > ATAN_TAB_N) ? ATAN_TAB_N : iterations;

  //Shift angle to be in range -180 to 180
  while(theta < -180) theta += 360;
  while(theta > 180) theta -= 360;

  //Shift angle to be in range -90 to 90
  if (theta < -90){
    theta = theta + 180;
    quadAdj = -1;
  } else if (theta > 90){
    theta = theta - 180;
    quadAdj = -1;
  } else{
    quadAdj = 1;
  }

  //Shift angle to be in range -45 to 45
  if (theta < -45){
    theta = theta + 90;
    shift = -1;
  } else if (theta > 45){
    theta = theta - 90;
    shift = 1;
  } else{
    shift = 0;
  }

  //convert angle to decimal representation N = ((2^16)*angle_deg) / 180
  if(theta < 0){
    theta = -theta;
    theta = ((unsigned int)theta<<10)/45;   //Convert to decimal representation of angle
    theta = (unsigned int)theta<<4;
    theta = -theta;
  } else{
    theta = ((unsigned int)theta<<10)/45;   //Convert to decimal representation of angle
    theta = (unsigned int)theta<<4;
  }

  //Initial values
  x1 = 0x4DBA;    //this will be the cosine result,
                  //initially the magic number 0.60725293
  y = 0;          //y will contain the sine result
  s = 0;          //s will contain the final angle
  sigma = 1;      //direction from target angle

  for (i=0; i<iterations; i++){
    sigma = (theta - s) > 0 ? 1 : -1;
    if(sigma < 0){
      x2 = x1 + (y >> i);
      y = y - (x1 >> i);
      x1 = x2;
      s -= *atanptr++;
    } else{
      x2 = x1 - (y >> i);
      y = y + (x1 >> i);
      x1 = x2;
      s += *atanptr++;
    }
  }

  //Correct for possible overflow in cosine result
  if(x1 < 0) x1 = -x1;

  //Push final values to appropriate registers
  if(shift > 0){
    *sin_result = x1;
    *cos_result = -y;
  } else if (shift < 0){
    *sin_result = -x1;
    *cos_result = y;
  } else {
    *sin_result = y;
    *cos_result = x1;
  }

  //Adjust for sign change if angle was in quadrant 3 or 4
  *sin_result = quadAdj * *sin_result;
  *cos_result = quadAdj * *cos_result;
}
