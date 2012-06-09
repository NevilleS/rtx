#ifndef InitializationTable_H
#define InitializationTable_H

#include "RTXConstants.h"
#include "RTXStructures.h"
#include "Processes.h"

/***
 * Defines the static initialization table, used to instantiate PCB objects on startup.
 ***/
static const Record INIT_TABLE[PROCESS_COUNT] = {
    { 0, NULL_PROCESS, &nullProcess },
    { 1, HIGH, &consoleProcess },
    { 2, LOW, &processA },
    { 3, MEDIUM, &processB },
    { 4, MEDIUM, &processC }
};

#endif

