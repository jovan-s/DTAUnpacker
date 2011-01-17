/*  Description:
 *      Declarations of functions that are used in conjugation with the APP_DATA
 *      struct. The struct is used to avoid too many global variables, and
 *      contains function pointers to the DLL functions, amongs other things.
 *
 *  Author: Jovan Stanojlovic
 */
#ifndef MAIN_H_
#define MAIN_H_

#include "DTAFunctions.h"

/* Number of required arguments */
#define ARG_LENGTH      4

/*----------------------------------------------------------------------------
 * Prints program usage to stderr.
 *
 *  Arguments:          name            Program name
 *--------------------------------------------------------------------------*/
void PrintUsage(char *name);

/*----------------------------------------------------------------------------
 * Initializes the APP_DATA structure that the program uses to manage keys
 * and function pointers. If an error occurs, the function returns FALSE and
 * 'error' is set to indicate the problem.
 *
 *  Arguments:          data            Pointer to the structure
 *                      error           Error message
 *
 *  Returns TRUE if successfully initialized, FALSE otherwise.
 *--------------------------------------------------------------------------*/
BOOL InitAppData(APP_DATA *data, char error[ERROR_LENGTH]);

/*----------------------------------------------------------------------------
 * Loads the dta* functions found inside of the DLL file. If any error occur,
 * the 'error' string is set, and FALSE is returned.
 *
 *  Arguments:          data            Pointer to the structure
 *                      error           Error message
 *
 *  Returns TRUE if successfully loaded, FALSE otherwise.
 *--------------------------------------------------------------------------*/
BOOL LoadFunctions(APP_DATA *data, char error[ERROR_LENGTH]);

/*----------------------------------------------------------------------------
 * Cleans up the APP_DATA structure, closing the DLL handle and deleting the
 * list of keys.
 *
 *  Arguments:          data            Pointer to the structure
 *--------------------------------------------------------------------------*/
void CleanupAppData(APP_DATA *data);

#endif
