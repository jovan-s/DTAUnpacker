/*  Description:
 *      Contains the prototypes of the functions needed when the DLL is loaded and
 *      GetProcAddress() is called.
 *
 *  Author: Jovan Stanojlovic
 */

#ifndef DTA_FUNCTIONS_H_
#define DTA_FUNCTIONS_H_

#include <windows.h>
#include "Container.h"

/* Length of an error string */
#define ERROR_LENGTH    128

/* Error codes */
#define DTA_OPEN_FAILED 0xFFFFFFFF      /* Calling dtaOpen() failed */ 

/*
 * Make things look cleaner. Use these strings when looking for the procedure inside the dll.
 */

#define FUNC_DTABIN2TEXT            "dtaBin2Text"
#define FUNC_DTACLOSE               "_dtaClose@4"
#define FUNC_DTACREATE              "_dtaCreate@4"
#define FUNC_DTADELETE              "dtaDelete"
#define FUNC_DTADUMPMEMORYLEAKS     "dtaDumpMemoryLeaks"
#define FUNC_DTAGETTIME             "dtaGetTime"
#define FUNC_DTAOPEN                "_dtaOpen@8"
#define FUNC_DTAOPENWRITE           "dtaOpenWrite"
#define FUNC_DTAREAD                "_dtaRead@12"
#define FUNC_DTASEEK                "_dtaSeek@12"
#define FUNC_DTASETDTAFIRSTFORCE    "_dtaSetDtaFirstForce@0"
#define FUNC_DTAWRITE               "dtaWrite"

/*
 * Function pointers used along with the rw_data.dll to decompress/decrypt the .dta files.
 */

/*--------------------------------------------------------------------------
 * Forces the files to be loaded from the disk first rather than the DTA
 * file. Call this function before doing any other operations.
 *-------------------------------------------------------------------------*/
typedef void (CALLBACK *FPDtaSetDtaFirstForce)();

/*--------------------------------------------------------------------------
 * Mounts a .DTA file so that further operations can be performed (such as
 * reading and writing).
 *
 *  Arguments:      filename            .DTA file to mount
 *
 *  Returns a non-zero value on success.
 *-------------------------------------------------------------------------*/
typedef DWORD (CALLBACK *FPDtaCreate)(char *filename);

/*--------------------------------------------------------------------------
 * Opens a file from a .DTA archive. If the call is successful, a handle to
 * the file is returned.
 *
 *  Arguments:      filename            File inside the archive
 *                  d                   Unknown, should be 0
 *
 *  Returns a handle to the file.
 *-------------------------------------------------------------------------*/
typedef DWORD (CALLBACK *FPDtaOpen)(char *filename, DWORD d);

/*--------------------------------------------------------------------------
 * Reads 'byteCount' bytes from a file inside an archive and stores them
 * into the 'buffer'. Returns the amount of bytes successfuly read.
 *
 *  Arguments:      fileHandle          Handle to the file to read
 *                  buffer              Buffer to store data
 *                  byteCount           Size of data
 *
 *  Returns number of bytes read.
 *-------------------------------------------------------------------------*/
typedef DWORD (CALLBACK *FPDtaRead)(DWORD fileHandle, char* buffer, DWORD byteCount);

/*--------------------------------------------------------------------------
 * Closes a file inside the archive given the file's handle.
 *
 *  Arguments:      fileHandle          File to close
 *-------------------------------------------------------------------------*/
typedef void (CALLBACK *FPDtaClose)(DWORD fileHandle);

/*----------------------------------------------------------------------------
 * Move the stream positioner of a file. This function mimmicks fseek, and
 * fseek's 'origin' values can be used.
 *
 *  Arguments:      fileHandle          File to traverse
 *                  offset              Relative offset from 'origin'
 *                  origin              SEEK_SET - Offset from beginning of file
 *                                      SEEK_CUR - Offset from current position
 *                                      SEEK_END - Offset from end of file
 *--------------------------------------------------------------------------*/
typedef void (CALLBACK *FPDtaSeek)(DWORD fileHandle, DWORD offset, DWORD origin);


/*
 * Structure used to simplify things and manage function pointers as well as keys.
 */
typedef struct t_data {
    HMODULE                 hDTADLL;

    /* DLL Functions */
    FPDtaSetDtaFirstForce   dtaSetDtaFirstForce;
    FPDtaCreate             dtaCreate;
    FPDtaOpen               dtaOpen;
    FPDtaRead               dtaRead;
    FPDtaClose              dtaClose;
    FPDtaSeek               dtaSeek;

    /* .dta file information */
    char                    dtaFile[256];
    DWORD                   dtaFileHandle;
    unsigned int            key1;
    unsigned int            key2;
    DWORD                   numOfFiles;

    /* Memory controller */
    BUF_CONTAINER           buffer;
} APP_DATA;

/*----------------------------------------------------------------------------
 * Opens the .dta file, decrypts it, and unpacks it's contents into a folder.
 * If any errors occur, 'error' string is set, and the function returns FALSE.
 *
 *  Arguments:      data            Pointer to APP_DATA object
 *                  error           Error string
 *
 *  Returns TRUE on success, FALSE otherwise.
 *--------------------------------------------------------------------------*/
BOOL ProcessDTAFile(APP_DATA *data, char error[ERROR_LENGTH]);

/*----------------------------------------------------------------------------
 * Reads the initial DTA header (including the magic "ISD0" identifier) and
 * returns TRUE if read correctly. After this function is called, the file
 * position indicator will be set at the beginning of the content header. If
 * any errors occur, 'error' string is set, and the function returns FALSE.
 *
 *  Arguments:      data            Pointer to APP_DATA object
 *                  error           Error string
 *
 *  Returns TRUE on success, FALSE otherwise.
 *--------------------------------------------------------------------------*/
BOOL ProcessDTAHeader(APP_DATA *data, char error[ERROR_LENGTH]);

/*----------------------------------------------------------------------------
 * Reads the invidiual DTA files, and creates a new file on the hard drive
 * where the file is finally written. If any errors occur, 'error' string is
 * set and the function returns FALSE.
 *
 *  Arguments:      data            Pointer to APP_DATA object
 *                  error           Error string
 *
 *  Returns TRUE on success, FALSE otherwise.
 *--------------------------------------------------------------------------*/
BOOL ProcessDTAFiles(APP_DATA *data, char error[ERROR_LENGTH]);

/*--------------------------------------------------------------------------
 * Decrypts 'buffer' of size 'byteCount' using 'key1' and 'key2' as
 * decryption keys.
 *
 * These keys are more or less specific to the file.
 *
 * Arguments:       buffer          Data to decrypt
 *                  byteCount       Size of data
 *                  key1            First decryption key
 *                  key2            Second decryption key
 *-------------------------------------------------------------------------*/
void Decrypt(void *buffer, unsigned int byteCount, unsigned int key1, unsigned int key2);

#endif
