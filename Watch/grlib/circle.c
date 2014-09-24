/* --COPYRIGHT--,BSD
 * Copyright (c) 2012, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * --/COPYRIGHT--*/

#include "grlib.h"

//*****************************************************************************
//
//! \addtogroup primitives_api
//! @{
//
//*****************************************************************************

//*****************************************************************************
//
//! Draws a circle.
//!
//! \param pContext is a pointer to the drawing context to use.
//! \param lX is the X coordinate of the center of the circle.
//! \param lY is the Y coordinate of the center of the circle.
//! \param lRadius is the radius of the circle.
//!
//! This function draws a circle, utilizing the Bresenham circle drawing
//! algorithm.  The extent of the circle is from \e lX - \e lRadius to \e lX +
//! \e lRadius and \e lY - \e lRadius to \e lY + \e lRadius, inclusive.
//!
//! \return None.
//
//*****************************************************************************
void
GrCircleDraw(const tContext *pContext, long lX, long lY, long lRadius)
{
    long lA, lB, lD, lX1, lY1;

    //
    // Check the arguments.
    //
    ASSERT(pContext);

    //
    // Initialize the variables that control the Bresenham circle drawing
    // algorithm.
    //
    lA = 0;
    lB = lRadius;
    lD = 3 - (2 * lRadius);

    //
    // Loop until the A delta is greater than the B delta, meaning that the
    // entire circle has been drawn.
    //
    while(lA <= lB)
    {
        //
        // Determine the row when subtracting the A delta.
        //
        lY1 = lY - lA;

        //
        // See if this row is within the clipping region.
        //
        if((lY1 >= pContext->sClipRegion.sYMin) &&
           (lY1 <= pContext->sClipRegion.sYMax))
        {
            //
            // Determine the column when subtracting the B delta.
            //
            lX1 = lX - lB;

            //
            // If this column is within the clipping region, then draw a pixel
            // at that position.
            //
            if((lX1 >= pContext->sClipRegion.sXMin) &&
               (lX1 <= pContext->sClipRegion.sXMax))
            {
                GrPixelDraw(pContext, lX1, lY1);
            }

            //
            // Determine the column when adding the B delta.
            //
            lX1 = lX + lB;

            //
            // If this column is within the clipping region, then draw a pixel
            // at that position.
            //
            if((lX1 >= pContext->sClipRegion.sXMin) &&
               (lX1 <= pContext->sClipRegion.sXMax))
            {
                GrPixelDraw(pContext, lX1, lY1);
            }
        }

        //
        // Determine the row when adding the A delta.
        //
        lY1 = lY + lA;

        //
        // See if this row is within the clipping region, and the A delta is
        // not zero (otherwise, it will be the same row as when the A delta was
        // subtracted).
        //
        if((lY1 >= pContext->sClipRegion.sYMin) &&
           (lY1 <= pContext->sClipRegion.sYMax) &&
           (lA != 0))
        {
            //
            // Determine the column when subtracting the B delta.
            //
            lX1 = lX - lB;

            //
            // If this column is within the clipping region, then draw a pixel
            // at that position.
            //
            if((lX1 >= pContext->sClipRegion.sXMin) &&
               (lX1 <= pContext->sClipRegion.sXMax))
            {
                GrPixelDraw(pContext, lX1, lY1);
            }

            //
            // Determine the column when adding the B delta.
            //
            lX1 = lX + lB;

            //
            // If this column is within the clipping region, then draw a pixel
            // at that position.
            //
            if((lX1 >= pContext->sClipRegion.sXMin) &&
               (lX1 <= pContext->sClipRegion.sXMax))
            {
                GrPixelDraw(pContext, lX1, lY1);
            }
        }

        //
        // Only draw the complementary pixels if the A and B deltas are
        // different (otherwise, they describe the same set of pixels).
        //
        if(lA != lB)
        {
            //
            // Determine the row when subtracting the B delta.
            //
            lY1 = lY - lB;

            //
            // See if this row is within the clipping region.
            //
            if((lY1 >= pContext->sClipRegion.sYMin) &&
               (lY1 <= pContext->sClipRegion.sYMax))
            {
                //
                // Determine the column when subtracting the a delta.
                //
                lX1 = lX - lA;

                //
                // If this column is within the clipping region, then draw a
                // pixel at that position.
                //
                if((lX1 >= pContext->sClipRegion.sXMin) &&
                   (lX1 <= pContext->sClipRegion.sXMax))
                {
                    GrPixelDraw(pContext, lX1, lY1);
                }

                //
                // Only draw the mirrored pixel if the A delta is non-zero
                // (otherwise, it will be the same pixel).
                //
                if(lA != 0)
                {
                    //
                    // Determine the column when adding the A delta.
                    //
                    lX1 = lX + lA;

                    //
                    // If this column is within the clipping region, then draw
                    // a pixel at that position.
                    //
                    if((lX1 >= pContext->sClipRegion.sXMin) &&
                       (lX1 <= pContext->sClipRegion.sXMax))
                    {
                        GrPixelDraw(pContext, lX1, lY1);
                    }
                }
            }

            //
            // Determine the row when adding the B delta.
            //
            lY1 = lY + lB;

            //
            // See if this row is within the clipping region.
            //
            if((lY1 >= pContext->sClipRegion.sYMin) &&
               (lY1 <= pContext->sClipRegion.sYMax))
            {
                //
                // Determine the column when subtracting the A delta.
                //
                lX1 = lX - lA;

                //
                // If this column is within the clipping region, then draw a
                // pixel at that position.
                //
                if((lX1 >= pContext->sClipRegion.sXMin) &&
                   (lX1 <= pContext->sClipRegion.sXMax))
                {
                    GrPixelDraw(pContext, lX1, lY1);
                }

                //
                // Only draw the mirrored pixel if the A delta is non-zero
                // (otherwise, it will be the same pixel).
                //
                if(lA != 0)
                {
                    //
                    // Determine the column when adding the A delta.
                    //
                    lX1 = lX + lA;

                    //
                    // If this column is within the clipping region, then draw
                    // a pixel at that position.
                    //
                    if((lX1 >= pContext->sClipRegion.sXMin) &&
                       (lX1 <= pContext->sClipRegion.sXMax))
                    {
                        GrPixelDraw(pContext, lX1, lY1);
                    }
                }
            }
        }

        //
        // See if the error term is negative.
        //
        if(lD < 0)
        {
            //
            // Since the error term is negative, adjust it based on a move in
            // only the A delta.
            //
            lD += (4 * lA) + 6;
        }
        else
        {
            //
            // Since the error term is non-negative, adjust it based on a move
            // in both the A and B deltas.
            //
            lD += (4 * (lA - lB)) + 10;

            //
            // Decrement the B delta.
            //
            lB -= 1;
        }

        //
        // Increment the A delta.
        //
        lA++;
    }
}

//*****************************************************************************
//
//! Draws a filled circle.
//!
//! \param pContext is a pointer to the drawing context to use.
//! \param lX is the X coordinate of the center of the circle.
//! \param lY is the Y coordinate of the center of the circle.
//! \param lRadius is the radius of the circle.
//!
//! This function draws a filled circle, utilizing the Bresenham circle drawing
//! algorithm.  The extent of the circle is from \e lX - \e lRadius to \e lX +
//! \e lRadius and \e lY - \e lRadius to \e lY + \e lRadius, inclusive.
//!
//! \return None.
//
//*****************************************************************************
void
GrCircleFill(const tContext *pContext, long lX, long lY, long lRadius)
{
    long lA, lB, lD, lX1, lX2, lY1;

    //
    // Check the arguments.
    //
    ASSERT(pContext);

    //
    // Initialize the variables that control the Bresenham circle drawing
    // algorithm.
    //
    lA = 0;
    lB = lRadius;
    lD = 3 - (2 * lRadius);

    //
    // Loop until the A delta is greater than the B delta, meaning that the
    // entire circle has been filled.
    //
    while(lA <= lB)
    {
        //
        // Determine the row when subtracting the A delta.
        //
        lY1 = lY - lA;

        //
        // See if this row is within the clipping region.
        //
        if((lY1 >= pContext->sClipRegion.sYMin) &&
           (lY1 <= pContext->sClipRegion.sYMax))
        {
            //
            // Determine the column when subtracting the B delta, and move it
            // to the left edge of the clipping region if it is to the left of
            // the clipping region.
            //
            lX1 = lX - lB;
            if(lX1 < pContext->sClipRegion.sXMin)
            {
                lX1 = pContext->sClipRegion.sXMin;
            }

            //
            // Determine the column when adding the B delta, and move it to the
            // right edge of the clipping region if it is to the right of the
            // clipping region.
            //
            lX2 = lX + lB;
            if(lX2 > pContext->sClipRegion.sXMax)
            {
                lX2 = pContext->sClipRegion.sXMax;
            }

            //
            // Draw a horizontal line if this portion of the circle is within
            // the clipping region.
            //
            if(lX1 <= lX2)
            {
                GrLineDrawH(pContext, lX1, lX2, lY1);
            }
        }

        //
        // Determine the row when adding the A delta.
        //
        lY1 = lY + lA;

        //
        // See if this row is within the clipping region, and the A delta is
        // not zero (otherwise, this describes the same row of the circle).
        //
        if((lY1 >= pContext->sClipRegion.sYMin) &&
           (lY1 <= pContext->sClipRegion.sYMax) &&
           (lA != 0))
        {
            //
            // Determine the column when subtracting the B delta, and move it
            // to the left edge of the clipping region if it is to the left of
            // the clipping region.
            //
            lX1 = lX - lB;
            if(lX1 < pContext->sClipRegion.sXMin)
            {
                lX1 = pContext->sClipRegion.sXMin;
            }

            //
            // Determine the column when adding the B delta, and move it to the
            // right edge of the clipping region if it is to the right of the
            // clipping region.
            //
            lX2 = lX + lB;
            if(lX2 > pContext->sClipRegion.sXMax)
            {
                lX2 = pContext->sClipRegion.sXMax;
            }

            //
            // Draw a horizontal line if this portion of the circle is within
            // the clipping region.
            //
            if(lX1 <= lX2)
            {
                GrLineDrawH(pContext, lX1, lX2, lY1);
            }
        }

        //
        // Only draw the complementary lines if the B delta is about to change
        // and the A and B delta are different (otherwise, they describe the
        // same set of pixels).
        //
        if((lD >= 0) && (lA != lB))
        {
            //
            // Determine the row when subtracting the B delta.
            //
            lY1 = lY - lB;

            //
            // See if this row is within the clipping region.
            //
            if((lY1 >= pContext->sClipRegion.sYMin) &&
               (lY1 <= pContext->sClipRegion.sYMax))
            {
                //
                // Determine the column when subtracting the A delta, and move
                // it to the left edge of the clipping regino if it is to the
                // left of the clipping region.
                //
                lX1 = lX - lA;
                if(lX1 < pContext->sClipRegion.sXMin)
                {
                    lX1 = pContext->sClipRegion.sXMin;
                }

                //
                // Determine the column when adding the A delta, and move it to
                // the right edge of the clipping region if it is to the right
                // of the clipping region.
                //
                lX2 = lX + lA;
                if(lX2 > pContext->sClipRegion.sXMax)
                {
                    lX2 = pContext->sClipRegion.sXMax;
                }

                //
                // Draw a horizontal line if this portion of the circle is
                // within the clipping region.
                //
                if(lX1 <= lX2)
                {
                    GrLineDrawH(pContext, lX1, lX2, lY1);
                }
            }

            //
            // Determine the row when adding the B delta.
            //
            lY1 = lY + lB;

            //
            // See if this row is within the clipping region.
            //
            if((lY1 >= pContext->sClipRegion.sYMin) &&
               (lY1 <= pContext->sClipRegion.sYMax))
            {
                //
                // Determine the column when subtracting the A delta, and move
                // it to the left edge of the clipping region if it is to the
                // left of the clipping region.
                //
                lX1 = lX - lA;
                if(lX1 < pContext->sClipRegion.sXMin)
                {
                    lX1 = pContext->sClipRegion.sXMin;
                }

                //
                // Determine the column when adding the A delta, and move it to
                // the right edge of the clipping region if it is to the right
                // of the clipping region.
                //
                lX2 = lX + lA;
                if(lX2 > pContext->sClipRegion.sXMax)
                {
                    lX2 = pContext->sClipRegion.sXMax;
                }

                //
                // Draw a horizontal line if this portion of the circle is
                // within the clipping region.
                //
                if(lX1 <= lX2)
                {
                    GrLineDrawH(pContext, lX1, lX2, lY1);
                }
            }
        }

        //
        // See if the error term is negative.
        //
        if(lD < 0)
        {
            //
            // Since the error term is negative, adjust it based on a move in
            // only the A delta.
            //
            lD += (4 * lA) + 6;
        }
        else
        {
            //
            // Since the error term is non-negative, adjust it based on a move
            // in both the A and B deltas.
            //
            lD += (4 * (lA - lB)) + 10;

            //
            // Decrement the B delta.
            //
            lB -= 1;
        }

        //
        // Increment the A delta.
        //
        lA++;
    }
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
