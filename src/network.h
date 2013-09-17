// Author: Dan O'Shea dan@djoshea.com 2013

#ifndef _NETWORK_H_
#define _NETWORK_H_

#include <stdbool.h>
#include "buffer.h"

#define logError printf
#define logInfo printf

// receive, parse, bufferring threads
void networkReceiveThreadStart();
void networkReceiveThreadTerminate();

// send utilities
void networkOpenSendSocket();
void networkCloseSendSocket();
bool networkSend(const char*, unsigned);

#endif
