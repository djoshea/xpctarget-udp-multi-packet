// Author: Dan O'Shea dan@djoshea.com 2013

/* Serialized Data File Logger
 *
 */
 
#include <stdio.h>
#include <string.h> /* For strcmp() */
#include <stdlib.h> /* For EXIT_FAILURE, EXIT_SUCCESS */

#include <math.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <inttypes.h>
#include <signal.h>

// local includes
#include "buffer.h"
#include "parser.h"
#include "network.h"
#include "signalLogger.h"

// NO TRAILING SLASH!
void abortFromMain(int sig) {
    printf("Main thread terminating\n");
    networkReceiveThreadTerminate();
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
    // Register <C-c> handler
    signal(SIGINT, abortFromMain);

    clearBuffers();
    networkReceiveThreadStart();

    while(1) {
        sleep(1);
    }

    abortFromMain(0);
    return(EXIT_SUCCESS);
}

