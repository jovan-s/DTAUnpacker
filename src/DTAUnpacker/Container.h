/*  Description:
 *      Interface to a resizable buffer that is wrapped inside a struct.
 *      This lets the file data that is being read get stored in a single place,
 *      without using too much dynamic memory.
 *
 *  Author: Jovan Stanojlovic
 */
#ifndef CONTAINER_H_
#define CONTAINER_H_

#include <windows.h>

/*
 * A struct used to keep a buffer. This is mainly used to minimize memory usage.
 */
typedef struct t_bufcontainer {
    size_t  size;
    char    *buf;
} BUF_CONTAINER;

/*----------------------------------------------------------------------------
 * Initializes the buffer to the given 'size'. Returns TRUE if successful,
 * FALSE otherwise.
 *
 *  Arguments:          buf             Pointer to the container
 *                      size            Initial size
 *--------------------------------------------------------------------------*/
BOOL InitBuffer(BUF_CONTAINER *buf, size_t size);

/*----------------------------------------------------------------------------
 * Resizes a BUF_CONTAINER object to 'newSize'. Returns TRUE if successful,
 * FALSE otherwise.
 *
 *  Arguments:          buf             Pointer to the container
 *                      newSize         Size of the new buffer
 *--------------------------------------------------------------------------*/
BOOL ResizeBuffer(BUF_CONTAINER *buf, size_t newSize);

/*----------------------------------------------------------------------------
 * Performs a block write operation from 'data' of size 'n' to the buffer.
 * The buffer may be resized during this operation to fit all the data, and if
 * it fails to resize, the function returns FALSE; otherwise, returns TRUE.
 *
 *  Arguments:          buf             Pointer to the container
 *                      data            Data to write
 *                      n               Size of data
 *--------------------------------------------------------------------------*/
BOOL WriteToBuffer(BUF_CONTAINER *buf, char *data, size_t n);

/*----------------------------------------------------------------------------
 * Writes data of size 'n' from the beginning of the buffer to 'filename'.
 * The function will create subdirectories if required. Returns TRUE if
 * successful, FALSE otherwise.
 *
 *  Arguments:          buf             Pointer to the container
 *                      n               Number of bytes to write
 *                      filename        Name of file to write to
 *--------------------------------------------------------------------------*/
BOOL WriteToFile(BUF_CONTAINER *buf, size_t n, char *filename);

/*----------------------------------------------------------------------------
 * Releases the memory used by the buffer. After this, the buffer is no
 * longer usable.
 *
 *  Arguments:          buf             Pointer to the container
 *--------------------------------------------------------------------------*/
void ReleaseBuffer(BUF_CONTAINER *buf);

#endif
