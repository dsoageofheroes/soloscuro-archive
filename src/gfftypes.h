#ifndef _GFFTYPE_H
#define _GFFTYPE_H

#define GFFTYPE(d,c,b,a) \
  (((long)(a)<<24) + ((long)(b)<<16) + ((long)(c)<<8) + (long)(d))

#define GT_FORM GFFTYPE('F','O','R','M')    // GFF internal chunk
#define GT_GFFI GFFTYPE('G','F','F','I')  // GFFI internal chunk
#define GFF_GFFI GFFTYPE('G','F','F','I')  // GFFI internal chunk
#define GT_GFRE GFFTYPE('G','F','R','E')    // GFF internal chunk
#define GT_GTOC GFFTYPE('G','T','O','C')    // GFF internal chunk
#define GT_ACCL GFFTYPE('A','C','C','L')    // Accelertor resource
#define GT_ADV  GFFTYPE('A','D','V','\x20') // AIL Audio Driver
#define GT_APFM GFFTYPE('A','P','F','M')    // Application frame resource
#define GT_BMAP GFFTYPE('B','M','A','P')    // 3DX Bump map (GIL bitmap table, 1 bitmap)
#define GT_BMP  GFFTYPE('B','M','P','\x20') // Bitmap (GIL bitmap table, 1+ bitmaps)
#define GT_BVOC GFFTYPE('B','V','O','C')    // Background play samples
#define GT_BUTN GFFTYPE('B','U','T','N')    // Button resource
#define GT_CMAP GFFTYPE('C','M','A','P')    // Color map table
#define GT_DADV GFFTYPE('D','A','D','V')    // AIL and .COM drivers (MEL version 1.x only)
#define GT_DATA GFFTYPE('D','A','T','A')
#define GT_DBOX GFFTYPE('D','B','O','X')    // Dialog box resource
#define GT_DRV  GFFTYPE('D','R','V','\x20') // Run-time loadable code/data module
#define GT_EDAT GFFTYPE('E','D','A','T')    // 3DX Environment database
#define GT_EBOX GFFTYPE('E','B','O','X')        // Edit box resource
#define GT_ETAB GFFTYPE('E','T','A','B')    // Object entry table
#define GT_FONT GFFTYPE('F','O','N','T')    // Font (GIL font)
#define GFF_FONT GFFTYPE('F','O','N','T')    // Font (GIL font)
#define GT_FVOC GFFTYPE('F','V','O','C')    // Foreground play samples
#define GT_GMAP GFFTYPE('G','M','A','P')        // Region map flags
#define GT_GPL  GFFTYPE('G','P','L','\x20') // Compiled GPL files
#define GT_GPLX GFFTYPE('G','P','L','X')    // GPL index file (GPLSHELL)
#define GT_ICON GFFTYPE('I','C','O','N')    // Icon resource (GIL bitmap table, 1-4 bitmaps)
#define GT_MAS  GFFTYPE('M','A','S','\x20') // Compiled MAS (GPL master) files
#define GT_MENU GFFTYPE('M','E','N','U')    // Menu resource
#define GT_MONR GFFTYPE('M','O','N','R')    // monsters by region ids and level (used in request.c DARKSUN)
#define GFF_MONR GFFTYPE('M','O','N','R')    // monsters by region ids and level (used in request.c DARKSUN)
#define GT_MGTL GFFTYPE('M','G','T','L')    // Global timbre library
#define GT_MSEQ GFFTYPE('M','S','E','Q')    // XMIDI sequence files (.XMI)
#define GT_OMAP GFFTYPE('O','M','A','P')    // Opacity map (GIL bitmap table, 1 bitmap)
#define GT_PAL  GFFTYPE('P','A','L','\x20') // VGA 256 color palette
#define GFF_PAL  GFFTYPE('P','A','L','\x20') // VGA 256 color palette
#define GT_POBJ GFFTYPE('P','O','B','J')    // PolyMesh object database
#define GT_RMAP GFFTYPE('R','M','A','P')        // Region tile map
#define GT_SCMD GFFTYPE('S','C','M','D')    // Animation script command table
#define GT_SBAR GFFTYPE('S','B','A','R')    // Scroll-bar resource
#define GT_SINF GFFTYPE('S','I','N','F')    // Sound card info
#define GT_SJMP GFFTYPE('S','J','M','P')    // OENGINE animation script jump table
#define GT_TEXT GFFTYPE('T','E','X','T')    // Text resource
#define GFF_TEXT GFFTYPE('T','E','X','T')    // Text resource
#define GT_TILE GFFTYPE('T','I','L','E')    // Tile graphic (GIL bitmap table, 1 bitmap)
#define GT_TMAP GFFTYPE('T','M','A','P')    // Texture map (GIL bitmap table, 1 bitmap)
#define GT_TXRF GFFTYPE('T','X','R','F')        // Texture map (GIL bitmap table, 1 bitmap)
#define GT_WIND GFFTYPE('W','I','N','D')    // Window resource
#define GT_ACF GFFTYPE('A','C','F','\x20')    // Cinematic Binary Script File
#define GT_BMA GFFTYPE('B','M','A','\x20')    // Cinematic Binary File

#define GT_CBMP GFFTYPE('C','B','M','P')     // Color Bit Map
#define GT_WALL GFFTYPE('W','A','L','L')     // WALL?
#define GT_OJFF GFFTYPE('O','J','F','F')     // Object data?
#define GT_RDFF GFFTYPE('R','D','F','F')     // RDFF?
#define GFF_RDFF GFFTYPE('R','D','F','F')     // RDFF?

#define GT_FNFO GFFTYPE('F','N','F','O')     // FNFO: Appears to have the object data for the game.

#define GFF_RDAT GFFTYPE('R','D','A','T') // Names?

// DSL/GPL
#define GT_GPL_INDEX_DATA GFFTYPE('G','P','L','X') // dsl/gpl index data
#define GT_IT1R GFFTYPE('I','T','1','R')
#define GFF_IT1R GFFTYPE('I','T','1','R')
#define GT_NAME GFFTYPE('N','A','M','E')
#define GFF_NAME GFFTYPE('N','A','M','E')
#define GT_ALL	GFFTYPE('A','L','L',' ')

#define GT_VECT GFFTYPE('V', 'E', 'C', 'T') // VECT?   rotate vector?

// sound!
#define GT_MERR GFFTYPE('M','E','R','R')	// text error file
#define GT_PSEQ GFFTYPE('P','S','E','Q')	// PCSPKR
#define GT_FSEQ GFFTYPE('F','S','E','Q')	// FM
#define GT_LSEQ GFFTYPE('L','S','E','Q')	// LAPC
#define GT_GSEQ GFFTYPE('G','S','E','Q')	// GENERAL MIDI
#define GT_CSEQ GFFTYPE('C','S','E','Q')	// Clock Sequences

// Transition art?
#define GT_CMAT GFFTYPE('C','M','A','T')
#define GT_CPAL GFFTYPE('C','P','A','L')
#define GT_PLYL GFFTYPE('P','L','Y','L')
#define GT_VPLY GFFTYPE('V','P','L','Y')

#define GT_SPIN GFFTYPE('S','P','I','N')   // Spell text.
#define GFF_SPIN GFFTYPE('S','P','I','N')   // Spell text.

#define GT_PORT GFFTYPE('P','O','R','T')   // Portrait

#endif
