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

#ifndef GRLIB_REMOVE_WIDE_FONT_SUPPORT
//*****************************************************************************
//
// A pointer to the application-passed structure containing default values for
// various text rendering properties.
//
//*****************************************************************************
static tGrLibDefaults const *g_psGrLibDefaults;

//
// Default text mapping functions.  This table allows the use of 8 bit encoded
// source text with either Unicode encoded wide fonts or legacy fonts
// which are assumed to contain ISO8859-1 text (ASCII + western European).
//
const tCodePointMap g_psDefaultCodePointMapTable[] =
{
    {CODEPAGE_ISO8859_1, CODEPAGE_UNICODE, GrMapISO8859_1_Unicode},
    {CODEPAGE_ISO8859_1, CODEPAGE_ISO8859_1, GrMapISO8859_1_Unicode},
    {CODEPAGE_UTF_8, CODEPAGE_UNICODE, GrMapUTF8_Unicode},
    {CODEPAGE_UTF_16, CODEPAGE_UNICODE, GrMapUTF16LE_Unicode},
};

#define NUM_DEFAULT_CODEPOINT_MAPS (sizeof(g_psDefaultCodePointMapTable) /    \
                                    sizeof(tCodePointMap))

//*****************************************************************************
//
//! Initializes graphics library default text rendering values.
//!
//! \param pDefaults points to a structure containing default values to use.
//!
//! This function allows an application to set global default values that the
//! graphics library will use when initializing any graphics context.  These
//! values set the source text codepage, the rendering function to use for
//! strings and mapping functions used to allow extraction of the correct
//! glyphs from fonts.
//!
//! If this function is not called by an application, the graphics library
//! assumes that text strings are ISO8859-1 encoded and that the default string
//! renderer is used.
//!
//! \return None.
//
//*****************************************************************************
void
GrLibInit(const tGrLibDefaults *pDefaults)
{
    ASSERT(pDefaults);

    //
    // Remember the pointer to the defaults structure.
    //
    g_psGrLibDefaults = pDefaults;
}

#endif

//*****************************************************************************
//
//! Initializes a drawing context.
//!
//! \param pContext is a pointer to the drawing context to initialize.
//! \param pDisplay is a pointer to the tDisplayInfo structure that describes
//! the display driver to use.
//!
//! This function initializes a drawing context, preparing it for use.  The
//! provided display driver will be used for all subsequent graphics
//! operations, and the default clipping region will be set to the extent of
//! the screen.
//!
//! \return None.
//
//*****************************************************************************
void
GrContextInit(tContext *pContext, const tDisplay *pDisplay)
{
    //
    // Check the arguments.
    //
    ASSERT(pContext);
    ASSERT(pDisplay);

    //
    // Set the size of the context.
    //
    pContext->lSize = sizeof(tContext);

    //
    // Save the pointer to the display.
    //
    pContext->pDisplay = pDisplay;

    //
    // Initialize the extent of the clipping region to the extents of the
    // screen.
    //
    pContext->sClipRegion.sXMin = 0;
    pContext->sClipRegion.sYMin = 0;
    pContext->sClipRegion.sXMax = pDisplay->usWidth - 1;
    pContext->sClipRegion.sYMax = pDisplay->usHeight - 1;

    //
    // Provide a default color and font.
    //
    pContext->ulForeground = 0;
    pContext->ulBackground = 0;
    pContext->pFont = 0;

#ifndef GRLIB_REMOVE_WIDE_FONT_SUPPORT
    //
    // Set defaults for all text rendering options.
    //
    if(g_psGrLibDefaults)
    {
        pContext->pfnStringRenderer = g_psGrLibDefaults->pfnStringRenderer;
        pContext->pCodePointMapTable = g_psGrLibDefaults->pCodePointMapTable;
        pContext->usCodepage = g_psGrLibDefaults->usCodepage;
        pContext->ucNumCodePointMaps = g_psGrLibDefaults->ucNumCodePointMaps;
        pContext->ucReserved = g_psGrLibDefaults->ucReserved;
    }
    else
    {
        pContext->pfnStringRenderer = GrDefaultStringRenderer;
        pContext->pCodePointMapTable = g_psDefaultCodePointMapTable;
        pContext->usCodepage = CODEPAGE_UTF_8;
        pContext->ucNumCodePointMaps = NUM_DEFAULT_CODEPOINT_MAPS;
        pContext->ucReserved = 0;
    }
    pContext->ucCodePointMap = 0;
#endif
}

//*****************************************************************************
//
//! Sets the extents of the clipping region.
//!
//! \param pContext is a pointer to the drawing context to use.
//! \param pRect is a pointer to the structure containing the extents of the
//! clipping region.
//!
//! This function sets the extents of the clipping region.  The clipping region
//! is not allowed to exceed the extents of the screen, but may be a portion of
//! the screen.
//!
//! The supplied coordinate are inclusive; \e sXMin of 1 and \e sXMax of 1 will
//! define a clipping region that will display only the pixels in the X = 1
//! column.  A consequence of this is that the clipping region must contain
//! at least one row and one column.
//!
//! \return None.
//
//*****************************************************************************
void
GrContextClipRegionSet(tContext *pContext, const tRectangle *pRect)
{
    unsigned long ulW, ulH;

    //
    // Check the arguments.
    //
    ASSERT(pContext);
    ASSERT(pRect);

    //
    // Get the width and height of the display.
    //
    ulW = DpyWidthGet(pContext->pDisplay);
    ulH = DpyHeightGet(pContext->pDisplay);

    //
    // Set the extents of the clipping region, forcing them to reside within
    // the extents of the screen.
    //
    pContext->sClipRegion.sXMin = ((pRect->sXMin < 0) ? 0 :
                                   ((pRect->sXMin >= ulW) ? (ulW - 1) :
                                    pRect->sXMin));
    pContext->sClipRegion.sYMin = ((pRect->sYMin < 0) ? 0 :
                                   ((pRect->sYMin >= ulH) ? (ulH - 1) :
                                    pRect->sYMin));
    pContext->sClipRegion.sXMax = ((pRect->sXMax < 0) ? 0 :
                                   ((pRect->sXMax >= ulW) ? (ulW - 1) :
                                    pRect->sXMax));
    pContext->sClipRegion.sYMax = ((pRect->sYMax < 0) ? 0 :
                                   ((pRect->sYMax >= ulH) ? (ulH - 1) :
                                    pRect->sYMax));
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
