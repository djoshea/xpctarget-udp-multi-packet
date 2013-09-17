// Author: Dan O'Shea dan@djoshea.com 2013

#include <stdio.h>
#include <string.h>
#include "buffer.h"
#include "parser.h"

// used by functions below, declared here so they are put in .bss or .data
// rather than cause stack overflows
static PacketSetData rawData;
static Packet p;

bool parsePacket(const uint8_t* rawPacket, int bytesRead, Packet* pp)
{
    const uint8_t* pBuf = rawPacket;

    memset(pp, 0, sizeof(Packet));

    if (bytesRead < 8) {
        // ERROR
        printf("Packet too short!\n");
        return false;
    } 

    // check packet header string matches (otherwise could be garbage packet)
    if(strncmp((char*)pBuf, PACKET_HEADER_STRING, strlen(PACKET_HEADER_STRING)) != 0)
    {
        // packet header does not match, discard
        return false;
    }
    pBuf = pBuf + strlen(PACKET_HEADER_STRING);

    // store the packet version
    STORE_UINT16(pBuf, pp->version);

    if (pp->version != 3) {
        printf("Packet is wrong version!\n");
        return false;
    }

    // store the transmit id of the sending block (allow different packet sets 
    // to be sorted and assembled correctly)
    STORE_UINT8(pBuf, pp->transmitId);

    // store the timestamp
    STORE_DOUBLE(pBuf, pp->timestamp);

    // store the number of packets
    STORE_UINT16(pBuf, pp->numPackets);

    // store the 1-indexed packet number
    STORE_UINT16(pBuf, pp->idxPacket);

    // convert this to 0-indexed
    pp->idxPacket--;

    // compute the data length in this packet
    pp->rawLength = bytesRead - (pBuf - rawPacket);
    
    // copy the raw data into the data buffer
    STORE_UINT8_ARRAY(pBuf, pp->rawData, pp->rawLength);

    return true;
}

void printPacket(const Packet* pp)
{
    printf("\tPacket [ v %u, txId %u, ts %g, packet %u of %u ]\n", 
            pp->version, pp->transmitId,  
            pp->timestamp, pp->idxPacket+1, pp->numPackets);
}

// packetData is the byte stream received directly off of the socket
// parse it into a Packet, add it to its PacketSet (or create one),
// and process the PacketSet if it is the last remaining Packet for it.
void processRawPacket(uint8_t* rawPacket, int bytesRead)
{
    bool validPacket, receivedAll;

    // parse rawPacket into a Packet struct
    memset(&p, 0, sizeof(Packet));
    validPacket = parsePacket(rawPacket, bytesRead, &p);

    if(!validPacket) {
        printf("Invalid packet received!\n");
        return;
    }

    // add Packet to the head of the packet buffer
    Packet* pPacket;
    pPacket = pushPacketAtHead(&p);
    //printPacket(pPacket);

    PacketSet* pPacketSet;
    pPacketSet = findPacketSetForPacket(pPacket);
    //printPacketSet(pPacketSet);

    if(pPacketSet == NULL)
    {
        // no existing packet set, should be first packet received 
        // this timestamp. create a new packet set
        pPacketSet = createPacketSetForPacket(pPacket);
    } else {
        // store a pointer to this packet inside the packet set
        pPacketSet->pPackets[pPacket->idxPacket] = pPacket;
        pPacketSet->packetReceived[pPacket->idxPacket] = 1;
        //printf("added packet %d at %x to ps\n", pPacket->idxPacket, pPacket);
    }

    // have we received the full group of packets yet?
    receivedAll = checkReceivedAllPackets(pPacketSet);
    if (receivedAll) {

        // look at the multi-packet data in this set
        // turn it into signals on the SignalBuffer, and remove the 
        // packet set and packets from their buffers
        processPacketSet(pPacketSet);

        removePacketSetFromBuffer(pPacketSet);
    }
}

// pPacketSet points to packetSet for which we have received all the packets
// parse it into signals. then parse these signals into signal groups
// signalLogger.cc will take care of removing the packet set and it's packets
// from their buffers.
void processPacketSet(PacketSet* pPacketSet)
{
    //PacketSetData rawData; // declared global
    Packet* pPacket;

    //printf("\nProcessing Packet Set for timestamp %d:\n", pPacketSet->timestamp);
    //printPacketSet(pPacketSet);

    // clear buffer to store all packet data
    memset(rawData.buffer, 0, MAX_DATA_SIZE_PER_TICK * sizeof(uint8_t));

    // loop over the packets, copying each into the data buffer
    int bufOffset = 0;
    for(int i = 0; i < pPacketSet->numPackets; i++) {
        // copy over this packet's data into the data buffer
        pPacket = pPacketSet->pPackets[i];
        memcpy(rawData.buffer + bufOffset, pPacket->rawData, 
                pPacket->rawLength * sizeof(uint8_t));
        
        // advance the offset into the data buffer
        bufOffset += pPacket->rawLength * sizeof(uint8_t);
    }

    // store the number of bytes stored
    rawData.length = bufOffset;
     
    //printf("Raw data length: %d\n", rawData.length);

    // store the current timestamp
    rawData.timestamp = pPacketSet->timestamp;

    // parse the raw data stream into signals
    // push these signals onto the buffer
    //
    //
    // INSERT HANDLING CODE HERE!
    printf("Received new data!\n");
}

