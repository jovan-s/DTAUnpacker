/*  Description:
 *      Implementation of the resizable buffer. VC++ generates warnings due to
 *      "unsafe" functions. Those are turned off in the project settings.
 *
 *  Author: Jovan Stanojlovic
 */

#include <stdlib.h>
#include <windows.h>
#include <stdio.h>
#include "Container.h"

/*----------------------------------------------------------------------------
 * Initializes the buffer to the given 'size'. Returns TRUE if successful,
 * FALSE otherwise.
 *
 *  Arguments:          buf             Pointer to the container
 *                      size            Initial size
 *--------------------------------------------------------------------------*/
BOOL InitBuffer(BUF_CONTAINER *buf, size_t size) {
    if((buf->buf = (char *)calloc(size, sizeof(char))) == NULL)
        return FALSE;

    buf->size = size;

    return TRUE;
}

/*----------------------------------------------------------------------------
 * Resizes a BUF_CONTAINER object to 'newSize'. Returns TRUE if successful,
 * FALSE otherwise.
 *
 *  Arguments:          buf             Pointer to the container
 *                      newSize         Size of the new buffer
 *--------------------------------------------------------------------------*/
BOOL ResizeBuffer(BUF_CONTAINER *buf, size_t newSize) {
    char *tempBuf;

    if((tempBuf = (char *)realloc(buf->buf, newSize)) == NULL)
        return FALSE;

    buf->buf  = tempBuf;
    buf->size = newSize;

    return TRUE;
}

/*----------------------------------------------------------------------------
 * Performs a block write operation from 'data' of size 'n' to the buffer.
 * The buffer may be resized during this operation to fit all the data, and if
 * it fails to resize, the function returns FALSE; otherwise, returns TRUE.
 *
 *  Arguments:          buf             Pointer to the container
 *                      data            Data to write
 *                      n               Size of data
 *--------------------------------------------------------------------------*/
BOOL WriteToBuffer(BUF_CONTAINER *buf, char *data, size_t n) {
    /* Need to resize? */
    if(buf->size < n && !ResizeBuffer(buf, n))
        return FALSE;

    memcpy((void *)buf->buf, (void *)data, n);

    return TRUE;
}

/*----------------------------------------------------------------------------
 * Writes data of size 'n' from the beginning of the buffer to 'filename'.
 * The function will create subdirectories if required. Returns TRUE if
 * successful, FALSE otherwise.
 *
 *  Arguments:          buf             Pointer to the container
 *                      n               Number of bytes to write
 *                      filename        Name of file to write to
 *--------------------------------------------------------------------------*/
BOOL WriteToFile(BUF_CONTAINER *buf, size_t n, char *filename) {
    char fullname[256 + 1] = { 0 };
    char *dirseek;

    dirseek = strtok(filename, "\\");

    while(dirseek != NULL) {
        strcat(fullname, dirseek);
        dirseek = strtok(NULL, "\\");

        if(dirseek != NULL) {
            CreateDirectory(fullname, NULL);
            strcat(fullname, "\\");
        } else {
            DWORD written;
            HANDLE hFile = CreateFile(fullname, GENERIC_ALL, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

            if(hFile == INVALID_HANDLE_VALUE)
                return FALSE;

            WriteFile(hFile, (void *)buf->buf, (DWORD)n, &written, NULL); 
            CloseHandle(hFile);
        }
    }


    return TRUE;
}

/*----------------------------------------------------------------------------
 * Releases the memory used by the buffer. After this, the buffer is no
 * longer usable.
 *
 *  Arguments:          buf             Pointer to the container
 *--------------------------------------------------------------------------*/
void ReleaseBuffer(BUF_CONTAINER *buf) {
    free(buf->buf);
}
