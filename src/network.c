// Author: Dan O'Shea dan@djoshea.com 2013

#include <pthread.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h> /* For strcmp() */
#include <time.h>

//Windows specific stuff
#ifdef WIN32
//#include <windows.h>
#include <winsock2.h>
#include <WS2tcpip.h>
#define close(s) closesocket(s)
#define s_errno WSAGetLastError()
#define EWOULDBLOCK WSAEWOULDBLOCK
#define usleep(a) Sleep((a)/1000)
#define MSG_NOSIGNAL 0
#define nonblockingsocket(s) {unsigned long ctl = 1;ioctlsocket( s, FIONBIO, &ctl );}
typedef int socklen_t;
//End windows stuff
#else
#include <arpa/inet.h>
#include <inttypes.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#define nonblockingsocket(s)  fcntl(s,F_SETFL,O_NONBLOCK)
#define Sleep(a) usleep(a*1000)
#endif

// Local includes
#include "buffer.h"
#include "parser.h"
#include "network.h"

// bind port for broadcast UDP receive
#define PORT 28001
#define SEND_IP "192.168.1.255"
#define SEND_PORT 10000 

void *networkReceiveThread(void * dummy);
void networkReceiveThreadCleanup(void * dummy);

int sock;
bool sockOpen;
pthread_t netThread;

int sockOut;
bool sockOutOpen;

void networkReceiveThreadStart() {
    // Start Network Receive Thread
    int rcNetwork = pthread_create(&netThread, NULL, networkReceiveThread, NULL); 
    if (rcNetwork) {
        logError("ERROR! Return code from pthread_create() is %d\n", rcNetwork);
        exit(-1);
    }
}

void networkReceiveThreadTerminate() {
    pthread_cancel(netThread);
    pthread_join(netThread, NULL);
}

void * networkReceiveThread(void * dummy)
{
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    //pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
    pthread_cleanup_push(networkReceiveThreadCleanup, NULL);

	uint8_t rawPacket[MAX_PACKET_LENGTH];
    int port = PORT;

    logInfo("Network: Receiving UDP broadcast at port %d\n", port);

    // Setup Socket Variables
    struct sockaddr_in si_me, si_other;
    int slen=sizeof(si_other);

    if ((sock=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
        diep("socket");

    sockOpen = true; 

    // Set socket buffer size to avoid dropped packets 
    int length = MAX_PACKET_LENGTH*50; 
    if (setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char*)&length, sizeof(int)) == -1) {
        logError("Error setting socket receive buffer size \n");
    }

    // setup to receive broadcast
    int broadcast = 1;
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));

    // Setup Local Socket on specified port to accept from any address
    memset((char *) &si_me, 0, sizeof(si_me)); 
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(port);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(sock,(struct sockaddr*) &si_me, sizeof(si_me))==-1)
        diep("bind");

    // clear the packet and packet set buffers
    clearBuffers();

    struct timespec req;
    while(1)
    {
        // Read from the socket 
        int bytesRead = recvfrom(sock, rawPacket, MAX_PACKET_LENGTH - 10, 0,
                (struct sockaddr*)&si_other, (socklen_t *)&slen);

        if(bytesRead == -1)
            diep("recvfrom()");

        processRawPacket(rawPacket, bytesRead);

        pthread_testcancel();
        req.tv_sec = 0;
        req.tv_nsec = 100000;
        //nanosleep(&req, &req);
    }

    close(sock);
    sockOpen = false;
    pthread_cleanup_pop(0);

    return NULL;
}

void networkReceiveThreadCleanup(void* dummy) {
    logInfo("Network: Terminating thread\n");
    if(sockOpen)
        close(sock);
    sockOpen = false;
}

struct sockaddr_in si_send; // address for writing responses 

void networkOpenSendSocket() {
    memset((char *) &si_send, 0, sizeof(si_send));
    si_send.sin_family = AF_INET;
    si_send.sin_port = htons(SEND_PORT);

    sockOut = sock;
    sockOutOpen = false;
    logInfo("Network: Ready to send to %s:%d\n", SEND_IP, SEND_PORT);
}

void networkCloseSendSocket() {
    sockOutOpen = false;
}

bool networkSend(const char* sendBuffer, unsigned bytesSend) {
    if(sendto(sockOut, (char *)sendBuffer, bytesSend, 
        0, (struct sockaddr *)&si_send, sizeof(si_send)) == -1) {
        logError("Sendto error");
        return false;
    }

    return true;
}
