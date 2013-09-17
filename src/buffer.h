// Author: Dan O'Shea dan@djoshea.com 2013

#ifndef BUFFER_H_INCLUDED
#define BUFFER_H_INCLUDED

#include <inttypes.h>
#include <stdbool.h>

/* Packet maxima */
typedef double timestamp_t;

#define MAX_PACKET_LENGTH 2000
#define MAX_PACKETS_PER_TICK 20 
#define MAX_DATA_SIZE_PER_TICK (MAX_PACKETS_PER_TICK * MAX_PACKET_LENGTH)

#define PACKET_BUFFER_SIZE 2000 
#define PACKETSET_BUFFER_SIZE 50

void diep(const char *s);

/////////// PACKET STRUCTURES //////////////
typedef struct Packet 
{
    uint16_t version; // version of the encoding scheme
    uint8_t transmitId; // transmit Id of the sending block
    timestamp_t timestamp; // timestamp from xpc 

    uint16_t numPackets;
    uint16_t idxPacket; // 1-indexed

    uint8_t rawData[MAX_PACKET_LENGTH];
    uint16_t rawLength;
} Packet;

typedef struct PacketSet
{
    uint16_t version; // version of the encoding scheme
    uint8_t transmitId; // transmit Id of the sending block
    timestamp_t timestamp; // timestamp from xpc 

    uint16_t numPackets;
    bool packetReceived[MAX_PACKETS_PER_TICK];
    Packet* pPackets[MAX_PACKETS_PER_TICK];
} PacketSet;

/////////// RING BUFFERS //////////////

typedef struct PacketRingBuffer
{
    Packet buffer[PACKET_BUFFER_SIZE];
    bool occupied[PACKET_BUFFER_SIZE];
    int head;
} PacketRingBuffer;

typedef struct PacketSetRingBuffer {
    PacketSet buffer[PACKETSET_BUFFER_SIZE];
    bool occupied[PACKETSET_BUFFER_SIZE];
    int head;
} PacketSetRingBuffer;

///////////// PROTOTYPES /////////////

void printPacket(const Packet*);
void printPacketSet(const PacketSet*);

void clearBuffers(void);
void printBufferUsage(void);

void setWarnDropped(void);
void setNoWarnDropped(void);

Packet* pushPacketAtHead(const Packet*);
void removePacketFromBuffer(Packet*);
double calculatePacketBufferRatioOccupied(void);

PacketSet* pushPacketSetAtHead(const PacketSet*);
void removePacketSetFromBuffer(PacketSet*);
double calculatePacketSetBufferRatioOccupied(void);

PacketSet* findPacketSetForPacket(const Packet*);
PacketSet* createPacketSetForPacket(Packet*);

bool checkReceivedAllPackets(PacketSet*);

void logIncompletePacketSet(const PacketSet*);

#endif
