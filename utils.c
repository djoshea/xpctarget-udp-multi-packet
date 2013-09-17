// MATLAB mat file library

#include <string.h>
#include <stdio.h>
#include <stdlib.h> /* For EXIT_FAILURE, EXIT_SUCCESS */
#include <sys/stat.h> // for directory type flags
#include <sys/time.h> // for directory type flags
#include <time.h>
#include <math.h>

#include "mex.h"

#include "signal.h"
#include "buffer.h"
#include "utils.h"

// PRIVATE PROTOTYPES
//void storeGroupInMxArray(mxArray*, const Group*, int);
//void storeSignalInNamedField(mxArray*, const Signal*);

#ifdef MATLAB_MEX_FILE
void* matlabMallocPersist(size_t size) {
    void* p;
    p = mxMalloc((mwSize)size);
    mexMakeMemoryPersistent(p);
    return p;
}

void* matlabCallocPersist(size_t n, size_t size) {
    void* p;
    p = mxCalloc((mwSize)n, (mwSize)size);
    mexMakeMemoryPersistent(p);
    return p;
}

void* matlabReallocPersist(void *p, size_t size) {
    p = mxRealloc(p, (mwSize)size);
    mexMakeMemoryPersistent(p);
    return p;
}

void matlabFree(void* p) {
    if(p != NULL)
        mxFree(p);
}

void* matlabMallocTemp(size_t size) {
    return mxMalloc((mwSize)size);
}

void* matlabCallocTemp(size_t n, size_t size) {
   return mxCalloc((mwSize)n, (mwSize)size);
}

void* matlabReallocTemp(void *p, size_t size) {
    return mxRealloc(p, (mwSize)size);
}
#endif

void diep(const char *s)
{
    perror(s);
    exit(1);
}

#ifndef MACOS
static struct timespec tstart, tstop;
static double totalElapsed;

void tic() {
    totalElapsed = 0;
    clock_gettime(CLOCK_MONOTONIC, &tstart);
}

void ticpause() {
    double elapsed;
    clock_gettime(CLOCK_MONOTONIC, &tstop);
    elapsed = (tstop.tv_sec - tstart.tv_sec);
    elapsed += (tstop.tv_nsec - tstart.tv_nsec) / 1000000000.0;
    totalElapsed += elapsed;
}

void ticresume() {
    clock_gettime(CLOCK_MONOTONIC, &tstart);
}

void toc() {
    ticpause();
    //printf("Elapsed time : %.6f seconds\n", totalElapsed);
}
#endif

mxClassID convertDataTypeIdToMxClassId(uint8_t dataTypeId)
{
    switch (dataTypeId) {
        case DTID_DOUBLE: // double
            return mxDOUBLE_CLASS;
            
        case DTID_SINGLE: // single
            return mxSINGLE_CLASS;

        case DTID_INT8: // int8
            return mxINT8_CLASS;
                
        case DTID_UINT8: // uint8
            return mxUINT8_CLASS;

        case DTID_INT16: // int16
            return mxINT16_CLASS;

        case DTID_UINT16: // uint16
            return mxUINT16_CLASS;

        case DTID_INT32: // int32
            return mxINT32_CLASS;

        case DTID_UINT32: // uint32
            return mxUINT32_CLASS;

        case DTID_CHAR: // char -> uint8 although use mxCreateString instead 
            return mxUINT8_CLASS; 

        default:
            diep("Unknown data type Id");
    }

    // never reach here
    return mxDOUBLE_CLASS;
}

int mkdirRecursive(const char *dir) {
    char tmp[MAX_FILENAME_LENGTH];
    char *p = NULL;
    size_t len;
    int failed;

    //printf("Recursive mkdir %s\n", dir);

    snprintf(tmp, sizeof(tmp),"%s",dir);
    len = strlen(tmp);
    if(tmp[len - 1] == '/')
        tmp[len - 1] = 0;
    for(p = tmp + 1; *p; p++)
    {
        if(*p == '/') {
            *p = 0;
            mkdir(tmp, S_IRWXU);
            *p = '/';
        }
    }
    //printf("mkdir %s\n", tmp);
    failed = mkdir(tmp, S_IRWXU);
    return failed;
}

timestamp_t getCurrentWallclock() {
    struct timeval tv;
    gettimeofday(&tv, NULL);

    return (timestamp_t)(tv.tv_sec) + (timestamp_t)(tv.tv_usec) / 100000.0;
}

void convertWallclockToLocalTime(timestamp_t wallclock, struct tm* timeinfo, unsigned* msec) {
    // timestamps are seconds since the unix epoch, i.e. unix timestamps
    time_t t = (time_t)wallclock;
    localtime_r(&t, timeinfo);

	// and msec within the current second (to ensure the names never collide)
    *msec = (unsigned int)floor((wallclock-floor(wallclock)) * 1000.0);
}

double_t convertTimestampToMatlabDateNum(timestamp_t timestamp)
{
    // timestamps are seconds since the unix epoch
    // matlab datenums are days since jan 1, 0000

    // need to get the UTC offset in order to put into local time
    struct tm timeInfo;
    time_t t = (time_t)timestamp;
    localtime_r(&t, &timeInfo);
    double_t secOffsetToLocalTime = timeInfo.tm_gmtoff;

    return (timestamp + (double_t)UNIX_EPOCH_OFFSET_SECONDS + secOffsetToLocalTime) / (double_t)(SECONDS_IN_DAY);
}

