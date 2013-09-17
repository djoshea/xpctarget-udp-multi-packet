// Author: Dan O'Shea dan@djoshea.com 2013

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mat.h"
#include <pthread.h>

#include "signal.h"
#include "buffer.h"
#include "signalLogger.h"

// TODO remove this mutex
pthread_mutex_t groupBufferMutex = PTHREAD_MUTEX_INITIALIZER;

static PacketRingBuffer pbuf;
static PacketSetRingBuffer psetbuf;

static bool warnDropped = true;

static PacketSet pset; // declared global here to avoid stack overflows

void diep(const char *s)
{
    perror(s);
    exit(1);
}

void setWarnDropped() {
    warnDropped = true;
}

void setNoWarnDropped() {
    warnDropped = false;
}

void clearBuffers() {
    memset(&pbuf, 0, sizeof(PacketRingBuffer));
    memset(&psetbuf, 0, sizeof(PacketSetRingBuffer));
}

void printBufferUsage()
{
    // print a message indicating how full the buffers are

    double rPacket, rPacketSet;

    rPacket = calculatePacketBufferRatioOccupied() * 100;
    rPacketSet = calculatePacketSetBufferRatioOccupied() * 100;

    printf("Buffer Usage: Packet [ %4.1f %% ], PacketSet [ %4.1f %% ]", 
            rPacket, rPacketSet);
}

/////// PACKET BUFFER /////////

// copy a packet onto the buffer at the head
// return a pointer to the packet in its buffer location
Packet * pushPacketAtHead(const Packet* pp)
{
    pbuf.head = (pbuf.head + 1) % PACKET_BUFFER_SIZE;

    if(pbuf.occupied[pbuf.head]) {
        // the existing packet will now be lost
        // find its PacketSet, log an error, and remove all packets
        // in that PacketSet to avoid further confusion
        PacketSet* ppset;
        ppset = findPacketSetForPacket(&pbuf.buffer[pbuf.head]);
        if (ppset != NULL) {
            logIncompletePacketSet(ppset);
            removePacketSetFromBuffer(ppset);
        }
    }

    // copy the Packet into the buffer
    memcpy(pbuf.buffer + pbuf.head, pp, sizeof(Packet));

    // mark that slot as occupied
    pbuf.occupied[pbuf.head] = 1;

    // return a pointer to the newly created packet
    return pbuf.buffer + pbuf.head;
}

void removePacketFromBuffer(Packet* pp)
{
    int index = pp - pbuf.buffer;

    //printf("\tRemoving Packet %d for ts %d at index %d\n", pp->idxPacket, pp->timestamp, index);

    if(index < 0 || index > PACKET_BUFFER_SIZE)
        diep("Attempt to remove Packet not in PacketRingBuffer");

    // clear this packet and mark as unoccupied in buffer
    memset(pp, 0, sizeof(Packet));
    pbuf.occupied[index] = 0;
}

double calculatePacketBufferRatioOccupied()
{
    int i = 0, count; 

    count = 0;
    for (i = 0; i < PACKET_BUFFER_SIZE; i++) {
        count += pbuf.occupied[i]; 
    }

    return (double)count / (double)PACKET_BUFFER_SIZE;
}

/////// PACKETSET BUFFER /////////

PacketSet * pushPacketSetAtHead(const PacketSet* pps)
{
    psetbuf.head = (psetbuf.head + 1) % PACKETSET_BUFFER_SIZE;
    if(psetbuf.occupied[psetbuf.head]) {
        logIncompletePacketSet(psetbuf.buffer + psetbuf.head);
        removePacketSetFromBuffer(psetbuf.buffer + psetbuf.head);
    }

    // copy this packet set into the buffer
    memcpy(psetbuf.buffer + psetbuf.head, pps, sizeof(PacketSet));

    // mark that slot as occupied
    psetbuf.occupied[psetbuf.head] = 1;

    // return a pointer to the newly created packet set
    return psetbuf.buffer + psetbuf.head;
}

void removePacketSetFromBuffer(PacketSet* ppset)
{
    int index = ppset - psetbuf.buffer;

    //printf("Removing PacketSet for ts %d at index %d\n", ppset->timestamp, index);

    if(index < 0 || index > PACKETSET_BUFFER_SIZE)
        diep("Attempt to remove PacketSet not in PacketSetRingBuffer");

    // clear the packets that comprise this packet set
    for(int i = 0; i < ppset->numPackets; i++) 
    {
        if(ppset->packetReceived[i]) 
            removePacketFromBuffer(ppset->pPackets[i]);
    }

    // clear this packet set and mark as unoccupied in buffer
    memset(ppset, 0, sizeof(PacketSet));
    psetbuf.occupied[index] = 0;
}
  
double calculatePacketSetBufferRatioOccupied()
{
    int i = 0, count; 

    count = 0;
    for (i = 0; i < PACKETSET_BUFFER_SIZE; i++) {
        count += psetbuf.occupied[i]; 
    }

    return (double)count / (double)PACKETSET_BUFFER_SIZE;
}

/////////// PACKETSET UTILITIES ///////////

PacketSet* findPacketSetForPacket(const Packet* pPacket)
{
    int i;

    // search backwards through the PacketSetBuffer for a PacketSet with:
    // a matching timestamp, matching transmitId, and matching version
    for(int c = 0; c < PACKETSET_BUFFER_SIZE; c++) {
        i = (psetbuf.head - c) % PACKETSET_BUFFER_SIZE;
        if (psetbuf.occupied[i] && 
            psetbuf.buffer[i].timestamp == pPacket->timestamp && 
            psetbuf.buffer[i].transmitId == pPacket->transmitId &&
            psetbuf.buffer[i].version == pPacket->version)
            
            // found it!
            return psetbuf.buffer + i;
    }

    return NULL;
}

PacketSet* createPacketSetForPacket(Packet* pPacket)
{
    //PacketSet pset; // declared global above

    // not found, create one and copy over relevant fields from packet 
	memset(&pset, 0, sizeof(PacketSet));
	pset.version = pPacket->version;
    pset.timestamp = pPacket->timestamp;
    pset.transmitId = pPacket->transmitId;
    pset.numPackets = pPacket->numPackets;
    pset.packetReceived[pPacket->idxPacket] = 1;
    pset.pPackets[pPacket->idxPacket] = pPacket;

    // add to head of buffer, which copies the data and returns
    // a pointer to the copy
    return pushPacketSetAtHead(&pset); 
}

bool checkReceivedAllPackets(PacketSet* pPacketSet)
{
    // check whether we've received all the packets for this tick
    for(int i = 0; i < pPacketSet->numPackets; i++) {
        if (!pPacketSet->packetReceived[i]) {
            return 0;
        }
    }

    return 1;
}

void printPacketSet(const PacketSet* ppset)
{
    // compute the total bytes across all packets
    int totalBytes = 0;
    unsigned i;
    for(i = 0; i < ppset->numPackets; i++)
    {
        if(ppset->packetReceived[i])
            totalBytes += ppset->pPackets[i]->rawLength;    
    }

    printf("\tPacketSet : [ v %u, txId %u, ts %g, %d bytes received, %d packets (", 
            ppset->version, ppset->transmitId, 
            ppset->timestamp, totalBytes, ppset->numPackets);
    for(i = 0; i < ppset->numPackets; i++)
    {
        if (ppset->packetReceived[i])
            printf("r");
        else
            printf("_");
    }
    printf(") ]\n");
}

void logIncompletePacketSet(const PacketSet* ppset)
{
    // this packet set was overwritten in the buffer before all packets received
    fprintf(stderr, "\nWARNING: Incomplete PacketSet for timestamp %g\n\n", ppset->timestamp);
}

