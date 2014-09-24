#include <string.h>
#include <stdio.h>
#include "spiflash.h"
#include "../grlib.h"

#ifndef CASSERT
#define CASSERT(exp, name) typedef int dummy##name [(exp) ? 1 : -1];
#endif

//*****************************************************************************
//
// The number of font block headers that we cache when a font is opened.
//
//*****************************************************************************
#define MAX_FONT_BLOCKS 16

//*****************************************************************************
//
// The amount of memory set aside to hold compressed data for a single glyph.
// Fonts for use with the graphics library limit compressed glyphs to 256 bytes.
// If you are sure your fonts contain small glyphs, you could reduce this to
// save space.
//
//*****************************************************************************
#define MAX_GLYPH_SIZE 128

#define SPI_OFFSET 0

typedef int FILEHANDLE;
#define BufferRead(pBuffer, ReadAddr, NumByteToRead) \
    SPI_FLASH_BufferRead_Raw(pBuffer, ReadAddr, NumByteToRead)

CASSERT(sizeof(tFontWide) == 8, tFontWide);
CASSERT(sizeof(tFontBlock) == 12, tFontBlock);

//*****************************************************************************
//
// Instance data for a single loaded font.
//
//*****************************************************************************
typedef struct
{
    //
    // The CFSfs file object associated with the font.
    //
    FILEHANDLE sFile;

    //
    // The font header as read from the file.
    //
    tFontWide sFontHeader;

    //
    // Storage for the font block table.
    //
    tFontBlock pBlocks[MAX_FONT_BLOCKS];

    //
    // A marker indicating whether or not the structure is in use.
    //
    tBoolean bInUse;

    //
    // The codepoint of the character whose glyph data is currently stored in
    // pucGlyphStore.
    //
    unsigned long ulCurrentGlyph;

    //
    // Storage for the compressed data of the latest glyph.  In a more complex
    // implementation, you would likely want to cache this data to reduce
    // slow disk interaction.
    //
    unsigned char pucGlyphStore[MAX_GLYPH_SIZE];
}
tFontFile;

//*****************************************************************************
//
// Instance data for a single loaded font.  This implementation only supports
// a single font open at any one time.  If you wanted to implement something
// more general, a memory manager could be used to allocate these structures
// dynamically in CFSWrapperFontLoad().
//
//*****************************************************************************
tFontFile g_sFontFile;


//*****************************************************************************
//
// Error reasons returned by ChangeDirectory().
//
//*****************************************************************************
#define NAME_TOO_LONG_ERROR 1
#define OPENDIR_ERROR       2

//*****************************************************************************
//
// Internal function prototypes.
//
//*****************************************************************************
static void CFSWrapperFontInfoGet(unsigned char *pucFontId,
                                  unsigned char *pucFormat,
                                  unsigned char *pucWidth,
                                  unsigned char *pucHeight,
                                  unsigned char *pucBaseline);
static const unsigned char *CFSWrapperFontGlyphDataGet(
                                  unsigned char *pucFontId,
                                  unsigned long ulCodePoint,
                                  unsigned char *pucWidth);
static unsigned short CFSWrapperFontCodepageGet(unsigned char *pucFontId);
static unsigned short CFSWrapperFontNumBlocksGet(unsigned char *pucFontId);
static unsigned long CFSWrapperFontBlockCodepointsGet(
                                 unsigned char *pucFontId,
                                 unsigned short usBlockIndex,
                                 unsigned long *pulStart);

//*****************************************************************************
//
// Access function pointers required to complete the tFontWrapper structure
// for this font.
//
//*****************************************************************************
static const tFontAccessFuncs g_sCFSFontAccessFuncs =
{
    CFSWrapperFontInfoGet,
    CFSWrapperFontGlyphDataGet,
    CFSWrapperFontCodepageGet,
    CFSWrapperFontNumBlocksGet,
    CFSWrapperFontBlockCodepointsGet
};

const tFontWrapper g_sFontUnicode =
{
    FONT_FMT_WRAPPED,
    (void*)&g_sFontFile,
    &g_sCFSFontAccessFuncs
};

//*****************************************************************************
//
// Returns information about a font previously loaded using CFSFontWrapperLoad.
//
//*****************************************************************************
static void
CFSWrapperFontInfoGet(unsigned char *pucFontId, unsigned char *pucFormat,
                      unsigned char *pucWidth, unsigned char *pucHeight,
                      unsigned char *pucBaseline)
{
    tFontFile *pFont;

    //
    // Parameter sanity check.
    //
    ASSERT(pucFontId);
    ASSERT(pucFormat);
    ASSERT(pucWidth);
    ASSERT(pucHeight);
    ASSERT(pucBaseline);

    //
    // Get a pointer to our instance data.
    //
    pFont = (tFontFile *)pucFontId;

    ASSERT(pFont->bInUse);

    //
    // Return the requested information.
    //
    *pucFormat = pFont->sFontHeader.ucFormat;
    *pucWidth =  pFont->sFontHeader.ucMaxWidth;
    *pucHeight =  pFont->sFontHeader.ucHeight;
    *pucBaseline =  pFont->sFontHeader.ucBaseline;
}

//*****************************************************************************
//
// Returns the codepage used by the font whose handle is passed.
//
//*****************************************************************************
static unsigned short
CFSWrapperFontCodepageGet(unsigned char *pucFontId)
{
    tFontFile *pFont;

    //
    // Parameter sanity check.
    //
    ASSERT(pucFontId);

    //
    // Get a pointer to our instance data.
    //
    pFont = (tFontFile *)pucFontId;

    ASSERT(pFont->bInUse);

    //
    // Return the codepage identifier from the font.
    //
    return(pFont->sFontHeader.usCodepage);
}

//*****************************************************************************
//
// Returns the number of glyph blocks supported by a particular font.
//
//*****************************************************************************
static unsigned short
CFSWrapperFontNumBlocksGet(unsigned char *pucFontId)
{
    tFontFile *pFont;

    //
    // Parameter sanity check.
    //
    ASSERT(pucFontId);

    //
    // Get a pointer to our instance data.
    //
    pFont = (tFontFile *)pucFontId;

    ASSERT(pFont->bInUse);

    //
    // Return the number of glyph blocks contained in the font.
    //
    return(pFont->sFontHeader.usNumBlocks);
}

//*****************************************************************************
//
// Read a given font block header from the provided file.
//
//*****************************************************************************
static tBoolean
CFSWrapperFontBlockHeaderGet(FILEHANDLE pFile, tFontBlock *pBlock,
                             unsigned long ulIndex)
{
    BufferRead((uint8_t*)pBlock,  
                         sizeof(tFontWide) + (sizeof(tFontBlock) * ulIndex),
                         sizeof(tFontBlock));
    //
    // If we get here, we experienced an error so return a bad return code.
    //
    return(true);
}

//*****************************************************************************
//
// Returns information on the glyphs contained within a given font block.
//
//*****************************************************************************
static unsigned long
CFSWrapperFontBlockCodepointsGet(unsigned char *pucFontId,
                                 unsigned short usBlockIndex,
                                 unsigned long *pulStart)
{
    tFontBlock sBlock;
    tFontFile *pFont;
    tBoolean bRetcode;

    //
    // Parameter sanity check.
    //
    ASSERT(pucFontId);

    //
    // Get a pointer to our instance data.
    //
    pFont = (tFontFile *)pucFontId;

    ASSERT(pFont->bInUse);

    //
    // Have we been passed a valid block index?
    //
    if(usBlockIndex >= pFont->sFontHeader.usNumBlocks)
    {
        //
        // No - return an error.
        //
        return(0);
    }

    //
    // Is this block header cached?
    //
    if(usBlockIndex < MAX_FONT_BLOCKS)
    {
        //
        // Yes - return the information from our cached copy.
        //
        *pulStart = pFont->pBlocks[usBlockIndex].ulStartCodepoint;
        return(pFont->pBlocks[usBlockIndex].ulNumCodepoints);
    }
    else
    {
        //
        // We don't have the block header cached so read it from the
        // SDCard.  First move the file pointer to the expected position.
        //
        bRetcode = CFSWrapperFontBlockHeaderGet(pFont->sFile, &sBlock,
                                                usBlockIndex);
        if(bRetcode)
        {
            *pulStart = sBlock.ulStartCodepoint;
            return(sBlock.ulNumCodepoints);
        }
        else
        {
            printf("Error reading block header!\n");
        }
    }

    //
    // If we get here, something horrible happened so return a failure.
    //
    *pulStart = 0;
    return(0);
}

//*****************************************************************************
//
// Retrieves the data for a particular font glyph.  This function returns
// a pointer to the glyph data in linear, random access memory if the glyph
// exists or NULL if not.
//
//*****************************************************************************
static const unsigned char *
CFSWrapperFontGlyphDataGet(unsigned char *pucFontId,
                           unsigned long ulCodepoint,
                           unsigned char *pucWidth)
{
    tFontFile *pFont;
    unsigned long ulLoop, ulGlyphOffset, ulTableOffset;

    tFontBlock sBlock;
    tFontBlock *pBlock;
    tBoolean bRetcode;

    //
    // Parameter sanity check.
    //
    ASSERT(pucFontId);
    ASSERT(pucWidth);

    //
    // If passed a NULL codepoint, return immediately.
    //
    if(!ulCodepoint)
    {
        return(0);
    }

    //
    // Get a pointer to our instance data.
    //
    pFont = (tFontFile *)pucFontId;

    ASSERT(pFont->bInUse);

    //
    // Look for the trivial case - do we have this glyph in our glyph store
    // already?
    //
    if(pFont->ulCurrentGlyph == ulCodepoint)
    {
        //
        // We struck gold - we already have this glyph in our buffer.  Return
        // the width (from the second byte of the data) and a pointer to the
        // glyph data.
        //
        *pucWidth = pFont->pucGlyphStore[1];
        return(pFont->pucGlyphStore);
    }

    //
    // First find the block that contains the glyph we've been asked for.
    //
    for(ulLoop = 0; ulLoop < pFont->sFontHeader.usNumBlocks; ulLoop++)
    {
        if(ulLoop < MAX_FONT_BLOCKS)
        {
            pBlock = &pFont->pBlocks[ulLoop];
        }
        else
        {
            bRetcode = CFSWrapperFontBlockHeaderGet(pFont->sFile, &sBlock,
                                                    ulLoop);
            pBlock = &sBlock;
            if(!bRetcode)
            {
                //
                // We failed to read the block header so return an error.
                //
                return(0);
            }
        }

        //
        // Does the requested character exist in this block?
        //
        if((ulCodepoint >= (pBlock->ulStartCodepoint)) &&
          ((ulCodepoint < (pBlock->ulStartCodepoint +
            pBlock->ulNumCodepoints))))
        {
            //
            // The glyph is in this block. Calculate the offset of it's
            // glyph table entry in the file.
            //
            ulTableOffset = pBlock->ulGlyphTableOffset +
                            ((ulCodepoint - pBlock->ulStartCodepoint) *
                            sizeof(unsigned long));
            
            BufferRead((uint8_t*)&ulGlyphOffset, 
                                 ulTableOffset,
                                 sizeof(unsigned long));

            //
            // Return if there was an error or if the offset is 0 (which
            // indicates that the character is not included in the font.
            //
            if(!ulGlyphOffset)
            {
                return(0);
            }

            //
            // Move the file pointer to the start of the glyph data remembering
            // that the glyph table offset is relative to the start of the
            // block not the start of the file (so we add the table offset
            // here).
            //
            BufferRead(pFont->pucGlyphStore,
                                 pBlock->ulGlyphTableOffset + ulGlyphOffset,
                                 1);
            //
            // Now read the glyph data.
            //
            BufferRead(pFont->pucGlyphStore + 1, 
                                 pBlock->ulGlyphTableOffset + ulGlyphOffset + 1,
                                 pFont->pucGlyphStore[0] - 1);

            //
            // If we get here, things are good. Return a pointer to the glyph
            // store data.
            //
            pFont->ulCurrentGlyph = ulCodepoint;
            *pucWidth = pFont->pucGlyphStore[1];
            return(pFont->pucGlyphStore);
        }
    }

    //
    // If we get here, the codepoint doesn't exist in the font so return an
    // error.
    //
    return(0);
}

//*****************************************************************************
//
// Prepares the CFS file system font wrapper for use.
//
// This function must be called before any attempt to use a font stored on the
// CFS file system.  It initializes CFSfs for use.
//
// \return Returns \b true on success or \b false on failure.
//
//*****************************************************************************
tBoolean
CFSFontWrapperInit(void)
{
    //
    // All is well so tell the caller.
    //
    return(true);
}

//*****************************************************************************
//
// Prepares a font in the CFSfs file system for use by the graphics library.
//
// This function must be called to prepare a font for use by the graphics
// library.  It opens the font file whose name is passed and reads the
// header information.  The value returned should be written into the
// pucFontId field of the tFontWrapper structure that will be passed to
// graphics library.
//
// This is a very simple (and slow) implementation.  More complex wrappers
// may also initialize a glyph cache here.
//
// \return On success, returns a non-zero pointer identifying the font.  On
// error, zero is returned.
//
//*****************************************************************************
unsigned char *
CFSFontWrapperLoad()
{
    //
    // Make sure a font is not already open.
    //
    if(g_sFontFile.bInUse)
    {
        //
        // Oops - someone tried to load a new font without unloading the
        // previous one.
        //
        printf("Another font is already loaded!\n");
        return(0);
    }

    //
    // We opened the file successfully.  Does it seem to contain a valid
    // font?  Read the header and see.
    //
    BufferRead((uint8_t*)&g_sFontFile.sFontHeader,
                         0,
                         sizeof(tFontWide));
    
    {
        //
        // We read the font header.  Is the format correct? We only support
        // wide fonts via wrappers.
        //
        if((g_sFontFile.sFontHeader.ucFormat != FONT_FMT_WIDE_UNCOMPRESSED) &&
           (g_sFontFile.sFontHeader.ucFormat != FONT_FMT_WIDE_PIXEL_RLE))
        {
            //
            // This is not a supported font format.
            //
            printf("Unrecognized font format. Failing "
                       "CFSFontWrapperLoad.\n");
            return(0);
        }

        //
        // The format seems to be correct so read as many block headers as we
        // have storage for.
        //
        int ulToRead = (g_sFontFile.sFontHeader.usNumBlocks > MAX_FONT_BLOCKS) ?
                    MAX_FONT_BLOCKS * sizeof(tFontBlock) :
                    g_sFontFile.sFontHeader.usNumBlocks * sizeof(tFontBlock);

        BufferRead((uint8_t*)&g_sFontFile.pBlocks, sizeof(tFontWide), ulToRead);
        {
            //
            // All is well.  Tell the caller the font was opened successfully.
            //
            printf("\n$$OK FONT\n");
            g_sFontFile.bInUse = true;
            return((unsigned char *)&g_sFontFile);
        }
    }
}

//*****************************************************************************
//
// Frees a font and cleans up once an application has finished using it.
//
// This function releases all resources allocated during a previous call to
// CFSFontWrapperLoad().  The caller must not make any further use of the
// font following this call unless another call to CFSFontWrapperLoad() is
// made.
//
// \return None.
//
//*****************************************************************************
void CFSFontWrapperUnload(unsigned char *pucFontId)
{
    tFontFile *pFont;

    //
    // Parameter sanity check.
    //
    ASSERT(pucFontId);

    //
    // Get a pointer to our instance data.
    //
    pFont = (tFontFile *)pucFontId;

    //
    // Make sure a font is already open.  If not, just return.
    //
    if(!pFont->bInUse)
    {
        return;
    }

    //
    // Close the font file.
    //
    printf("Unloading font... \n");

    //
    // Clean up our instance data.
    //
    pFont->bInUse = false;
    pFont->ulCurrentGlyph = 0;
}