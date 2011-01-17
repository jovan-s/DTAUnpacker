/*  Description:
 *      The purpose of the program is to allow a user to extra .DTA files,
 *      found in Mafia and Hidden & Dangerous 2. The program allows the
 *      user to specify which file they want to load in, along with the two
 *      decryption keys needed.
 *
 *      Afterwards, the program is opens the DTA file using the DLL functions
 *      from "tmp.dll", and begins decrypting and extracting the files to the
 *      hard drive. A resizable buffer is used to keep memory consumption to a minimum.
 *
 *      After the extraction is completed, the program terminates and the user can
 *      browse the archive. A successful termination gives a return code of 0, otherwise
 *      returns -1 and an error message is printed to standard error.
 *
 *  Special thanks:
 *      MassaSnygga, "Accessing Mafias DTA files", http://www.kamalook.de/Mafia/DTA.html
 *      Lars
 *
 *  Author: Jovan Stanojlovic
 */

#include <stdio.h>
#include "main.h"
#include "DTAFunctions.h"

/*----------------------------------------------------------------------------
 * Main entry point. The three arguments that must be specified are:
 *
 *  argv[1] - DTA file
 *  argv[2] - first key (in hex)
 *  argv[3] - second key (in hex)
 *
 *  Returns 0 upon success, -1 otherwise and a message is printed to stderr.
 *--------------------------------------------------------------------------*/
int main(int argc, char *argv[]) {
    APP_DATA    data = { 0 };
    char        error[ERROR_LENGTH];

    if(argc != ARG_LENGTH) {
        PrintUsage(argv[0]);

        return -1;
    }

    if(!InitAppData(&data, error)) {
        printf("Error occured: %s\nExiting...\n", error);

        CleanupAppData(&data);
        return -1;
    }

    /* Obtain command-line arguments */
    strncpy_s(data.dtaFile, 256, argv[1], 256);
    if(!(data.key1 = strtoul(argv[2], NULL, 16)) || !(data.key2 = strtoul(argv[3], NULL, 16))) {
        fprintf(stderr, "Invalid keys provided\n");
        return -1;
    }

    /* Main routine */
    if(!ProcessDTAFile(&data, error)) {
        printf("Error occured: %s\nExiting...\n", error);

        CleanupAppData(&data);
        return -1;
    }

    if(!ProcessDTAHeader(&data, error)) {
        printf("Error occured: %s\nExiting...\n", error);

        CleanupAppData(&data);
        return -1;
    }

    if(!ProcessDTAFiles(&data, error)) {
        printf("Error occured: %s\nExiting...\n", error);

        CleanupAppData(&data);
        return -1;
    }

    CleanupAppData(&data);

    return 0;
}


/*----------------------------------------------------------------------------
 * Prints program usage to stderr.
 *
 *  Arguments:          name            Program name
 *--------------------------------------------------------------------------*/
void PrintUsage(char *name) {
    fprintf(stderr, "\nUsage: %s [.DTA FILE] [KEY1] [KEY2]\n", name);
    fprintf(stderr, "Decrypts and unpacks a DTA \"ISD0\" archive using the keys provided.\n\n");
    fprintf(stderr, "The keys used by Hidden & Dangerous 2 are:\n");
    fprintf(stderr, "Archive\t\tKey1\t\tKey2\n");
    fprintf(stderr, "-------\t\t----\t\t----\n");
    fprintf(stderr, "Maps.dta\t0xF26527FA\t0xB438D0A5\n");
    fprintf(stderr, "Missions.dta\t0x22BCDA98\t0x7654A3F0\n");
    fprintf(stderr, "Models.dta\t0x10ACB252\t0x5D805259\n");
    fprintf(stderr, "Others.dta\t0x65F7AB23\t0xEA85902A\n");
    fprintf(stderr, "Sounds.dta\t0x8D2965CA\t0x4FE85106\n");
    fprintf(stderr, "Scripts.dta\t0xCF761298\t0x0AB4E72D\n");
    fprintf(stderr, "\nThe keys used by Mafia are:\n");
    fprintf(stderr, "Archive\t\tKey1\t\tKey2\n");
    fprintf(stderr, "-------\t\t----\t\t----\n");
    fprintf(stderr, "A0.dta\t\t0xD8D0A975\t0x467ACDE0\n");
    fprintf(stderr, "A1.dta\t\t0x3D98766C\t0xDE7009CD\n");
    fprintf(stderr, "A2.dta\t\t0x82A1C97B\t0x2D5085D4\n");
    fprintf(stderr, "A3.dta\t\t0x43876FEA\t0x900CDBA8\n");
    fprintf(stderr, "A4.dta\t\t0x43876FEA\t0x900CDBA8\n");
    fprintf(stderr, "A5.dta\t\t0xDEAC5342\t0x760CE652\n");
    fprintf(stderr, "A6.dta\t\t0x64CD8D0A\t0x4BC97B2D\n");
    fprintf(stderr, "A7.dta\t\t0xD6FEA900\t0xCDB76CE6\n");
    fprintf(stderr, "A8.dta\t\t0xD8DD8FAC\t0x5324ACE5\n");
    fprintf(stderr, "A9.dta\t\t0x6FEE6324\t0xACDA4783\n");
    fprintf(stderr, "AA.dta\t\t0x5342760C\t0xEDEAC652\n");
    fprintf(stderr, "AB.dta\t\t0xD8D0A975\t0x467ACDE0\n");
    fprintf(stderr, "AC.dta\t\t0x43876FEA\t0x900CDBA8\n");
}

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
BOOL InitAppData(APP_DATA *data, char error[ERROR_LENGTH]) {
    /* Attempt to load the DLL first */
    data->hDTADLL = LoadLibrary("tmp.dll");

    if(data->hDTADLL == NULL) {
        strncpy_s(error, ERROR_LENGTH, "tmp.dll could not be loaded correctly", ERROR_LENGTH);
        return FALSE;
    }

    /* Initialize the buffer with a starting 1024 bytes */
    InitBuffer(&data->buffer, 1024);

    /* Attempt to load functions */
   if(!LoadFunctions(data, error))
       return FALSE;

    return TRUE;
}

/*--------------------------------------------------------------------------
 * Loads the dta* functions found inside of the DLL file. If any error occur,
 * the 'error' string is set, and FALSE is returned.
 *
 *  Arguments:          data            Pointer to the structure
 *                      error           Error message
 *
 *  Returns TRUE if successfully loaded, FALSE otherwise.
 *-------------------------------------------------------------------------*/
BOOL LoadFunctions(APP_DATA *data, char error[ERROR_LENGTH]) {
    data->dtaSetDtaFirstForce   = (FPDtaSetDtaFirstForce)GetProcAddress(data->hDTADLL, FUNC_DTASETDTAFIRSTFORCE);
    data->dtaCreate             = (FPDtaCreate)GetProcAddress(data->hDTADLL, FUNC_DTACREATE);
    data->dtaOpen               = (FPDtaOpen)GetProcAddress(data->hDTADLL, FUNC_DTAOPEN);
    data->dtaRead               = (FPDtaRead)GetProcAddress(data->hDTADLL, FUNC_DTAREAD);
    data->dtaClose              = (FPDtaClose)GetProcAddress(data->hDTADLL, FUNC_DTACLOSE);
    data->dtaSeek               = (FPDtaSeek)GetProcAddress(data->hDTADLL, FUNC_DTASEEK);

    if(data->dtaSetDtaFirstForce == NULL) {
        strncpy_s(error, ERROR_LENGTH, "dtaSetDtaFirstForce() function not found", ERROR_LENGTH);
        return FALSE;
    } else if(data->dtaCreate == NULL) {
        strncpy_s(error, ERROR_LENGTH, "dtaCreate() function not found", ERROR_LENGTH);
        return FALSE;
    } else if(data->dtaOpen == NULL) {
        strncpy_s(error, ERROR_LENGTH, "dtaOpen() function not found", ERROR_LENGTH);
        return FALSE;
    } else if(data->dtaRead == NULL) {
        strncpy_s(error, ERROR_LENGTH, "dtaRead() function not found", ERROR_LENGTH);
        return FALSE;
    } else if(data->dtaClose == NULL) {
        strncpy_s(error, ERROR_LENGTH, "dtaClose() function not found", ERROR_LENGTH);
        return FALSE;
    } else if(data->dtaSeek == NULL) {
        strncpy_s(error, ERROR_LENGTH, "dtaSeek() function not found", ERROR_LENGTH);
        return FALSE;
    }

    return TRUE;
}

/*----------------------------------------------------------------------------
 * Cleans up the APP_DATA structure, closing the DLL handle and deleting the
 * list of keys.
 *
 *  Arguments:          data            Pointer to the structure
 *--------------------------------------------------------------------------*/
void CleanupAppData(APP_DATA *data) {
    ReleaseBuffer(&data->buffer);
    FreeLibrary(data->hDTADLL);
}
