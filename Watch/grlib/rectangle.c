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
// Make sure min and max are defined.
//
//*****************************************************************************
#ifndef min
#define min(a, b)               (((a) < (b)) ? (a) : (b))
#endif

#ifndef max
#define max(a, b)               (((a) < (b)) ? (b) : (a))
#endif

//*****************************************************************************
//
//! Draws a rectangle.
//!
//! \param pContext is a pointer to the drawing context to use.
//! \param pRect is a pointer to the structure containing the extents of the
//! rectangle.
//!
//! This function draws a rectangle.  The rectangle will extend from \e lXMin
//! to \e lXMax and \e lYMin to \e lYMax, inclusive.
//!
//! \return None.
//
//*****************************************************************************
void
GrRectDraw(const tContext *pContext, const tRectangle *pRect)
{
    //
    // Check the arguments.
    //
    ASSERT(pContext);
    ASSERT(pRect);

    //
    // Draw a line across the top of the rectangle.
    //
    GrLineDrawH(pContext, pRect->sXMin, pRect->sXMax, pRect->sYMin);

    //
    // Return if the rectangle is one pixel tall.
    //
    if(pRect->sYMin == pRect->sYMax)
    {
        return;
    }

    //
    // Draw a line down the right side of the rectangle.
    //
    GrLineDrawV(pContext, pRect->sXMax, pRect->sYMin + 1, pRect->sYMax);

    //
    // Return if the rectangle is one pixel wide.
    //
    if(pRect->sXMin == pRect->sXMax)
    {
        return;
    }

    //
    // Draw a line across the bottom of the rectangle.
    //
    GrLineDrawH(pContext, pRect->sXMax - 1, pRect->sXMin, pRect->sYMax);

    //
    // Return if the rectangle is two pixels tall.
    //
    if((pRect->sYMin + 1) == pRect->sYMax)
    {
        return;
    }

    //
    // Draw a line up the left side of the rectangle.
    //
    GrLineDrawV(pContext, pRect->sXMin, pRect->sYMax - 1, pRect->sYMin + 1);
}

//*****************************************************************************
//
//! Draws a filled rectangle.
//!
//! \param pContext is a pointer to the drawing context to use.
//! \param pRect is a pointer to the structure containing the extents of the
//! rectangle.
//!
//! This function draws a filled rectangle.  The rectangle will extend from
//! \e lXMin to \e lXMax and \e lYMin to \e lYMax, inclusive.  The clipping of
//! the rectangle to the clipping rectangle is performed within this routine;
//! the display driver's rectangle fill routine is used to perform the actual
//! rectangle fill.
//!
//! \return None.
//
//*****************************************************************************
void
GrRectFill(const tContext *pContext, const tRectangle *pRect)
{
    tRectangle sTemp;

    //
    // Check the arguments.
    //
    ASSERT(pContext);
    ASSERT(pRect);

    //
    // Swap the X coordinates if sXMin is greater than sXMax.
    //
    if(pRect->sXMin > pRect->sXMax)
    {
        sTemp.sXMin = pRect->sXMax;
        sTemp.sXMax = pRect->sXMin;
    }
    else
    {
        sTemp.sXMin = pRect->sXMin;
        sTemp.sXMax = pRect->sXMax;
    }

    //
    // Swap the Y coordinates if sYMin is greater than sYMax.
    //
    if(pRect->sYMin > pRect->sYMax)
    {
        sTemp.sYMin = pRect->sYMax;
        sTemp.sYMax = pRect->sYMin;
    }
    else
    {
        sTemp.sYMin = pRect->sYMin;
        sTemp.sYMax = pRect->sYMax;
    }

    //
    // Now that the coordinates are ordered, return without drawing anything if
    // the entire rectangle is out of the clipping region.
    //
    if((sTemp.sXMin > pContext->sClipRegion.sXMax) ||
       (sTemp.sXMax < pContext->sClipRegion.sXMin) ||
       (sTemp.sYMin > pContext->sClipRegion.sYMax) ||
       (sTemp.sYMax < pContext->sClipRegion.sYMin))
    {
        return;
    }

    //
    // Clip the X coordinates to the edges of the clipping region if necessary.
    //
    if(sTemp.sXMin < pContext->sClipRegion.sXMin)
    {
        sTemp.sXMin = pContext->sClipRegion.sXMin;
    }
    if(sTemp.sXMax > pContext->sClipRegion.sXMax)
    {
        sTemp.sXMax = pContext->sClipRegion.sXMax;
    }

    //
    // Clip the Y coordinates to the edges of the clipping region if necessary.
    //
    if(sTemp.sYMin < pContext->sClipRegion.sYMin)
    {
        sTemp.sYMin = pContext->sClipRegion.sYMin;
    }
    if(sTemp.sYMax > pContext->sClipRegion.sYMax)
    {
        sTemp.sYMax = pContext->sClipRegion.sYMax;
    }

    //
    // Call the low level rectangle fill routine.
    //
    DpyRectFill(pContext->pDisplay, &sTemp, pContext->ulForeground);
}

//*****************************************************************************
//
//! Determines if two rectangles overlap.
//!
//! \param psRect1 is a pointer to the first rectangle.
//! \param psRect2 is a pointer to the second rectangle.
//!
//! This function determines whether two rectangles overlap.  It assumes that
//! rectangles \e psRect1 and \e psRect2 are valid with \e sXMin < \e sXMax and
//! \e sYMin < \e sYMax.
//!
//! \return Returns 1 if there is an overlap or 0 if not.
//
//*****************************************************************************
long
GrRectOverlapCheck(tRectangle *psRect1, tRectangle *psRect2)
{
    if((psRect1->sXMax < psRect2->sXMin) ||
       (psRect2->sXMax < psRect1->sXMin) ||
       (psRect1->sYMax < psRect2->sYMin) ||
       (psRect2->sYMax < psRect1->sYMin))
    {
        return(0);
    }
    else
    {
        return(1);
    }
}

//*****************************************************************************
//
//! Determines the intersection of two rectangles.
//!
//! \param psRect1 is a pointer to the first rectangle.
//! \param psRect2 is a pointer to the second rectangle.
//! \param psIntersect is a pointer to a rectangle which will be written with
//! the intersection of \e psRect1 and \e psRect2.
//!
//! This function determines if two rectangles overlap and, if they do,
//! calculates the rectangle representing their intersection.  If the rectangles
//! do not overlap, 0 is returned and \e psIntersect is not written.
//!
//! \return Returns 1 if there is an overlap or 0 if not.
//
//*****************************************************************************
long
GrRectIntersectGet(tRectangle *psRect1, tRectangle *psRect2,
                   tRectangle *psIntersect)
{
    //
    // Make sure we were passed valid rectangles.
    //
    if((psRect1->sXMax <= psRect1->sXMin) ||
       (psRect1->sYMax <= psRect1->sYMin) ||
       (psRect2->sXMax <= psRect2->sXMin) ||
       (psRect2->sYMax <= psRect2->sYMin))
    {
        return(0);
    }

    //
    // Make sure that there is an intersection between the two rectangles.
    //
    if(!GrRectOverlapCheck(psRect1, psRect2))
    {
        return(0);
    }

    //
    // The rectangles do intersect so determine the rectangle of the
    // intersection.
    //
    psIntersect->sXMin = max(psRect1->sXMin, psRect2->sXMin);
    psIntersect->sXMax = min(psRect1->sXMax, psRect2->sXMax);
    psIntersect->sYMin = max(psRect1->sYMin, psRect2->sYMin);
    psIntersect->sYMax = min(psRect1->sYMax, psRect2->sYMax);

    return(1);
}

//*****************************************************************************
//
//! Draws a filled round rectangle.
//!
//! \param pContext is a pointer to the drawing context to use.
//! \param pRect is a pointer to the structure containing the extents of the
//! rectangle.
//!
//! This function draws a filled rectangle.  The rectangle will extend from
//! \e lXMin to \e lXMax and \e lYMin to \e lYMax, inclusive.  The clipping of
//! the rectangle to the clipping rectangle is performed within this routine;
//! the display driver's rectangle fill routine is used to perform the actual
//! rectangle fill.
//!
//! \return None.
//
//*****************************************************************************
void GrRectFillRound(const tContext *pContext, const tRectangle *pRect, long lRadius)
{
  GrCircleFill(pContext, pRect->sXMin + lRadius, pRect->sYMin + lRadius, lRadius);
  GrCircleFill(pContext, pRect->sXMin + lRadius, pRect->sYMax - lRadius, lRadius);
  GrCircleFill(pContext, pRect->sXMax - lRadius, pRect->sYMin + lRadius, lRadius);
  GrCircleFill(pContext, pRect->sXMax - lRadius, pRect->sYMax - lRadius, lRadius);

  tRectangle rect;
  rect.sYMin = pRect->sYMin;
  rect.sYMax = pRect->sYMax;
  rect.sXMin = pRect->sXMin + lRadius;
  rect.sXMax = pRect->sXMax - lRadius;
  GrRectFill(pContext, &rect);
  rect.sYMin = pRect->sYMin + lRadius;
  rect.sYMax = pRect->sYMax - lRadius;
  rect.sXMin = pRect->sXMin;
  rect.sXMax = pRect->sXMin + lRadius;
  GrRectFill(pContext, &rect);
  rect.sYMin = pRect->sYMin + lRadius;
  rect.sYMax = pRect->sYMax - lRadius;
  rect.sXMin = pRect->sXMax - lRadius;
  rect.sXMax = pRect->sXMax;
  GrRectFill(pContext, &rect);
}
//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
