/*  Description:
 *      Contains the DTA file format structures used by the dtaRead function,
 *      along with utility functions to simplify processing
 *
 *      The magic identifier of a DTA file is the first four bytes, which must be ISD0.
 *      The rest of the content varies from file to file, and is encrypted.
 *
 *  Author: Jovan Stanojlovic
 */
#ifndef DTA_FILE_FORMAT_H_
#define DTA_FILE_FORMAT_H_

/* The 4-byte sequence that a DTA file begins with */
#define TRUE_DTA_IDENTIFIER (('I') + ('S' << 8) + ('D' << 16) + ('0' << 24))

/*
 * This is the initial header, it contains 4 fields:
 *
 *  4 bytes     - Number of files in this archive
 *  4 bytes     - Beginning offset of the content table
 *  4 bytes     - Size of the content table
 *  4 bytes     - [Unknown]
 */
typedef struct t_dtaheader {
    DWORD   numOfFiles;
    DWORD   contentOffset;
    DWORD   contentSize;
    DWORD   extra;
} DTA_HEADER;

/*
 * This is the contents header, it contains 4 fields as well:
 *
 *  4 bytes     - [Unknown]
 *  4 bytes     - Beginning offset of the file data
 *  4 bytes     - [Unknown]
 *  16 bytes    - Filename hint
 */
typedef struct t_dtacontentheader {
    DWORD   extra1;
    DWORD   fileOffset;
    DWORD   extra2;
    char    filename[16];
} DTA_CONTENT_HEADER;

/*
 *
 *
 */
typedef struct t_dtafileheader {
    DWORD           extra1;
    DWORD           extra2;
    DWORD           extra3;
    DWORD           extra4;
    DWORD           fileSize;
    DWORD           extra5;
    unsigned char   filenameLength;
    char            extra6[7];
  /*char            *filename; Processed seperately */
} DTA_FILE_HEADER;

#endif
