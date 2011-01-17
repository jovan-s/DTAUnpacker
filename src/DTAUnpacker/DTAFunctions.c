/*  Description:
 *      Implementation of the functions needed to read through DTA files, as well
 *      as decrypt headers.
 *
 *  Author: Jovan Stanojlovic
 */


#include <stdio.h>
#include "DTAFunctions.h"
#include "DTAFormat.h"
#include "Container.h"

/*----------------------------------------------------------------------------
 * Utility function that processes a single file. The function assumes that
 * the position of the stream reader is set at the beginning of the header.
 *
 *  Arguments:      data            Pointer to the APP_DATA object
 *                  error           Error string
 *
 *  Returns TRUE on successful, false otherwise.
 *--------------------------------------------------------------------------*/
static BOOL ProcessFile(APP_DATA *data, char error[ERROR_LENGTH]);

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
void Decrypt(void *buffer, unsigned int byteCount, unsigned int key1, unsigned int key2) {
    unsigned char   *pKey;
    unsigned int    byteCount64;
    __int64         *buffer64;
    unsigned char   *pByte;
    unsigned int    cBytes;
    unsigned int    keys[2] = { key2, key1 };

    /* Process 64-bit chunks first, then process remaining bytes */
    buffer64 = (__int64 *)buffer;
    byteCount64 = byteCount / 8;

    for (; byteCount64; --byteCount64, ++buffer64) {
        unsigned int    ulong;
        unsigned int    *pLong = ((unsigned int *)buffer64);

        ulong = *pLong;
        *pLong = ~((~ulong) ^ key2);
        ++pLong;
        ulong = *pLong;
        *pLong = ~((~ulong) ^ key1);
    }

    pByte = ((unsigned char *)buffer64);
    cBytes = byteCount % 8;
    pKey = ((unsigned char *)keys);

    for (; cBytes; --cBytes, ++pByte, ++pKey) {
        unsigned char byte  = *pByte;
        unsigned char key   = *pKey;

        *pByte = (unsigned char)(~((~byte) ^ key));
    }
}

/*----------------------------------------------------------------------------
 * Opens the .dta file, decrypts it, and unpacks it's contents into a folder.
 * If any errors occur, 'error' string is set, and the function returns FALSE.
 *
 *  Arguments:      data            Pointer to APP_DATA object
 *                  error           Error string
 *
 *  Returns TRUE on success, FALSE otherwise.
 *--------------------------------------------------------------------------*/
BOOL ProcessDTAFile(APP_DATA *data, char error[ERROR_LENGTH]) {
    int key1;
    int key2;
    DWORD result;
    DTA_CONTENT_HEADER c = { 0 };
    DTA_HEADER f = { 0 };
    DTA_FILE_HEADER h = { 0 };

    /* Prepare for opening, attempt to open the file */
    data->dtaSetDtaFirstForce();
    result = data->dtaCreate(data->dtaFile);

    key1 = data->key1;
    key2 = data->key2;

    /* If it succeeded, load the two keys */
    if(result) {
        __asm {
            mov eax, key1           ; Load the first key into the EAX register
            xor eax, 034985762h     ; XOR it with the first magic key
            push eax                
            mov eax, key2           ; Push the second key onto the EAX stack
            xor eax, 039475694h     ; XOR it with the second magic key
            push eax
            mov ecx, result         ; Load the result into the ECX register -- this is demanded by the function
            mov eax, [ecx]          ; Load the address of the function table into EAX.
            call[eax + 0ch]         ; Call the fourth table entry
        }
    } else {
        strncpy_s(error, ERROR_LENGTH, "Failed to open the requested .dta file", ERROR_LENGTH);
        return FALSE;
    }

    data->dtaFileHandle = data->dtaOpen(data->dtaFile, 0);

    if(data->dtaFileHandle == DTA_OPEN_FAILED) {
        strncpy_s(error, ERROR_LENGTH, "The file was mounted successfuly, however opening it failed.", ERROR_LENGTH);
        return FALSE;
    }

    return TRUE;
}

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
BOOL ProcessDTAHeader(APP_DATA *data, char error[ERROR_LENGTH]) {
    int identifier;
    DTA_HEADER header = { 0 };

    /* Confirm ISD0 exists */
    data->dtaRead(data->dtaFileHandle, (char *)&identifier, sizeof(int));

    if(identifier != TRUE_DTA_IDENTIFIER) {
        strncpy_s(error, ERROR_LENGTH, "File did not begin with \"ISD0\"", ERROR_LENGTH);
        return FALSE;
    }

    /* Read and decrypt the rest of the header */
    data->dtaRead(data->dtaFileHandle, (char *)&header, sizeof(DTA_HEADER));
    Decrypt((void*)&header, sizeof(DTA_HEADER), data->key1, data->key2);

    data->numOfFiles = header.numOfFiles;

    /* Advance the position indicator */
    data->dtaSeek(data->dtaFileHandle, header.contentOffset, SEEK_SET);

    return TRUE;
}

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
BOOL ProcessDTAFiles(APP_DATA *data, char error[ERROR_LENGTH]) {
    DTA_CONTENT_HEADER *contentHeaders;

    /* Reserve space for all header, read them in and decrypt them */
    contentHeaders = (DTA_CONTENT_HEADER *)malloc(sizeof(DTA_CONTENT_HEADER) * data->numOfFiles);

    if(contentHeaders == NULL) {
        strncpy_s(error, ERROR_LENGTH, "Could not allocate memory for content headers", ERROR_LENGTH);
        return FALSE;
    }

    data->dtaRead(data->dtaFileHandle, (char *)contentHeaders, sizeof(DTA_CONTENT_HEADER) * data->numOfFiles);
    Decrypt((void *)contentHeaders, sizeof(DTA_CONTENT_HEADER) * data->numOfFiles, data->key1, data->key2);

    /* Read each file */
    while(data->numOfFiles--) {
        int pos = contentHeaders[data->numOfFiles].fileOffset;
        data->dtaSeek(data->dtaFileHandle, pos, SEEK_SET);

        ProcessFile(data, error);
    }

    /* Clean up, successful exit */
    free(contentHeaders);
    return TRUE;
}

/*----------------------------------------------------------------------------
 * Utility function that processes a single file. The function assumes that
 * the position of the stream reader is set at the beginning of the header.
 *
 *  Arguments:      data            Pointer to the APP_DATA object
 *                  error           Error string
 *
 *  Returns TRUE on successful, false otherwise.
 *--------------------------------------------------------------------------*/
static BOOL ProcessFile(APP_DATA *data, char error[ERROR_LENGTH]) {
    DWORD                   fileHandle;
    char                    filename[256 + 1]   = { 0 };
    DTA_FILE_HEADER         fileHeader          = { 0 };

    /* Read in the file header, followed by the filename
       Because we don't know the size of the filename right away, we deal with it
       seperately. */

    data->dtaRead(data->dtaFileHandle, (char *)&fileHeader, sizeof(DTA_FILE_HEADER));
    Decrypt((void *)&fileHeader, sizeof(DTA_FILE_HEADER), data->key1, data->key2);

    data->dtaRead(data->dtaFileHandle, filename, fileHeader.filenameLength);
    Decrypt((void *)filename, fileHeader.filenameLength, data->key1, data->key2);
    filename[fileHeader.filenameLength] = '\0';

    /* Attempt to open the file */
    fileHandle = data->dtaOpen(filename, 0);

    if(fileHandle == DTA_OPEN_FAILED) {
        strncpy_s(error, ERROR_LENGTH, "A file inside the archive could not be opened", ERROR_LENGTH);
        return FALSE;
    }

    /* Read the contents into the buffer */
    if(data->buffer.size < fileHeader.fileSize && !ResizeBuffer(&data->buffer, fileHeader.fileSize)) {
        strncpy_s(error, ERROR_LENGTH, "Allocating memory for a buffer failed", ERROR_LENGTH);
        return FALSE;
    }

    data->dtaRead(fileHandle, data->buffer.buf, fileHeader.fileSize);

    WriteToFile(&data->buffer, fileHeader.fileSize, filename);
    
    data->dtaClose(fileHandle);

    return TRUE;
}
