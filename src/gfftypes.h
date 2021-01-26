#ifndef _GFFTYPE_H
#define _GFFTYPE_H

#define GFFTYPE(d,c,b,a) \
  (((long)(a)<<24) + ((long)(b)<<16) + ((long)(c)<<8) + (long)(d))

#define GFF_FORM GFFTYPE('F','O','R','M')    // GFF internal chunk
#define GFF_GFFI GFFTYPE('G','F','F','I')  // GFFI internal chunk
#define GFF_GFFI GFFTYPE('G','F','F','I')  // GFFI internal chunk
#define GFF_GFRE GFFTYPE('G','F','R','E')    // GFF internal chunk
#define GFF_GFFOC GFFTYPE('G','T','O','C')    // GFF internal chunk
#define GFF_ACCL GFFTYPE('A','C','C','L')    // Accelertor resource
#define GFF_ADV  GFFTYPE('A','D','V','\x20') // AIL Audio Driver
#define GFF_APFM GFFTYPE('A','P','F','M')    // Application frame resource
#define GFF_BMAP GFFTYPE('B','M','A','P')    // 3DX Bump map (GIL bitmap table, 1 bitmap)
#define GFF_BMP  GFFTYPE('B','M','P','\x20') // Bitmap (GIL bitmap table, 1+ bitmaps)
#define GFF_BMP  GFFTYPE('B','M','P','\x20') // Bitmap (GIL bitmap table, 1+ bitmaps)
#define GFF_BVOC GFFTYPE('B','V','O','C')    // Background play samples
#define GFF_BUTN GFFTYPE('B','U','T','N')    // Button resource
#define GFF_CMAP GFFTYPE('C','M','A','P')    // Color map table
#define GFF_DADV GFFTYPE('D','A','D','V')    // AIL and .COM drivers (MEL version 1.x only)
#define GFF_DATA GFFTYPE('D','A','T','A')
#define GFF_DBOX GFFTYPE('D','B','O','X')    // Dialog box resource
#define GFF_DRV  GFFTYPE('D','R','V','\x20') // Run-time loadable code/data module
#define GFF_EDAT GFFTYPE('E','D','A','T')    // 3DX Environment database
#define GFF_EBOX GFFTYPE('E','B','O','X')        // Edit box resource
#define GFF_ETAB GFFTYPE('E','T','A','B')    // Object entry table
#define GFF_ETAB GFFTYPE('E','T','A','B')    // Object entry table
#define GFF_FONT GFFTYPE('F','O','N','T')    // Font (GIL font)
#define GFF_FONT GFFTYPE('F','O','N','T')    // Font (GIL font)
#define GFF_FVOC GFFTYPE('F','V','O','C')    // Foreground play samples
#define GFF_GMAP GFFTYPE('G','M','A','P')        // Region map flags
#define GFF_GMAP GFFTYPE('G','M','A','P')        // Region map flags
#define GFF_GPL  GFFTYPE('G','P','L','\x20') // Compiled GPL files
#define GFF_GPL  GFFTYPE('G','P','L','\x20') // Compiled GPL files
#define GFF_GPLX GFFTYPE('G','P','L','X')    // GPL index file (GPLSHELL)
#define GFF_ICON GFFTYPE('I','C','O','N')    // Icon resource (GIL bitmap table, 1-4 bitmaps)
#define GFF_ICON GFFTYPE('I','C','O','N')    // Icon resource (GIL bitmap table, 1-4 bitmaps)
#define GFF_MAS  GFFTYPE('M','A','S','\x20') // Compiled MAS (GPL master) files
#define GFF_MAS  GFFTYPE('M','A','S','\x20') // Compiled MAS (GPL master) files
#define GFF_MENU GFFTYPE('M','E','N','U')    // Menu resource
#define GFF_MONR GFFTYPE('M','O','N','R')    // monsters by region ids and level (used in request.c DARKSUN)
#define GFF_MONR GFFTYPE('M','O','N','R')    // monsters by region ids and level (used in request.c DARKSUN)
#define GFF_MGFFL GFFTYPE('M','G','T','L')    // Global timbre library
#define GFF_MSEQ GFFTYPE('M','S','E','Q')    // XMIDI sequence files (.XMI)
#define GFF_OMAP GFFTYPE('O','M','A','P')    // Opacity map (GIL bitmap table, 1 bitmap)
#define GFF_PAL  GFFTYPE('P','A','L','\x20') // VGA 256 color palette
#define GFF_PAL  GFFTYPE('P','A','L','\x20') // VGA 256 color palette
#define GFF_POBJ GFFTYPE('P','O','B','J')    // PolyMesh object database
#define GFF_RMAP GFFTYPE('R','M','A','P')        // Region tile map
#define GFF_RMAP GFFTYPE('R','M','A','P')        // Region tile map
#define GFF_SCMD GFFTYPE('S','C','M','D')    // Animation script command table
#define GFF_SCMD GFFTYPE('S','C','M','D')    // Animation script command table
#define GFF_SBAR GFFTYPE('S','B','A','R')    // Scroll-bar resource
#define GFF_SINF GFFTYPE('S','I','N','F')    // Sound card info
#define GFF_SJMP GFFTYPE('S','J','M','P')    // OENGINE animation script jump table
#define GFF_TEXT GFFTYPE('T','E','X','T')    // Text resource
#define GFF_TEXT GFFTYPE('T','E','X','T')    // Text resource
#define GFF_TILE GFFTYPE('T','I','L','E')    // Tile graphic (GIL bitmap table, 1 bitmap)
#define GFF_TILE GFFTYPE('T','I','L','E')    // Tile graphic (GIL bitmap table, 1 bitmap)
#define GFF_TMAP GFFTYPE('T','M','A','P')    // Texture map (GIL bitmap table, 1 bitmap)
#define GFF_TXRF GFFTYPE('T','X','R','F')        // Texture map (GIL bitmap table, 1 bitmap)
#define GFF_WIND GFFTYPE('W','I','N','D')    // Window resource
#define GFF_ACF GFFTYPE('A','C','F','\x20')    // Cinematic Binary Script File
#define GFF_BMA GFFTYPE('B','M','A','\x20')    // Cinematic Binary File

#define GFF_CBMP GFFTYPE('C','B','M','P')     // Color Bit Map
#define GFF_WALL GFFTYPE('W','A','L','L')     // WALL?
#define GFF_OJFF GFFTYPE('O','J','F','F')     // Object data?
#define GFF_OJFF GFFTYPE('O','J','F','F')     // Object data?
#define GFF_RDFF GFFTYPE('R','D','F','F')     // RDFF?
#define GFF_RDFF GFFTYPE('R','D','F','F')     // RDFF?

#define GFF_FNFO GFFTYPE('F','N','F','O')     // FNFO: Appears to have the object data for the game.

#define GFF_RDAT GFFTYPE('R','D','A','T') // Names?

// DSL/GPL
#define GFF_GPL_INDEX_DATA GFFTYPE('G','P','L','X') // dsl/gpl index data
#define GFF_IT1R GFFTYPE('I','T','1','R')
#define GFF_IT1R GFFTYPE('I','T','1','R')
#define GFF_NAME GFFTYPE('N','A','M','E')
#define GFF_NAME GFFTYPE('N','A','M','E')
#define GFF_ALL	GFFTYPE('A','L','L',' ')

#define GFF_VECT GFFTYPE('V', 'E', 'C', 'T') // VECT?   rotate vector?

// sound!
#define GFF_MERR GFFTYPE('M','E','R','R')	// text error file
#define GFF_MERR GFFTYPE('M','E','R','R')	// text error file
#define GFF_PSEQ GFFTYPE('P','S','E','Q')	// PCSPKR
#define GFF_FSEQ GFFTYPE('F','S','E','Q')	// FM
#define GFF_LSEQ GFFTYPE('L','S','E','Q')	// LAPC
#define GFF_GSEQ GFFTYPE('G','S','E','Q')	// GENERAL MIDI
#define GFF_CSEQ GFFTYPE('C','S','E','Q')	// Clock Sequences

// Transition art?
#define GFF_CMAT GFFTYPE('C','M','A','T')
#define GFF_CPAL GFFTYPE('C','P','A','L')
#define GFF_PLYL GFFTYPE('P','L','Y','L')
#define GFF_VPLY GFFTYPE('V','P','L','Y')

#define GFF_SPIN GFFTYPE('S','P','I','N')   // Spell text.

#define GFF_PORT GFFTYPE('P','O','R','T')   // Portrait

#define GFF_SPST GFFTYPE('S','P','S','T') // spell list bit-mask
#define GFF_PSST GFFTYPE('P','S','S','T') // psionc list bytes
#define GFF_CHAR GFFTYPE('C','H','A','R') // saved character slot.

#endif
