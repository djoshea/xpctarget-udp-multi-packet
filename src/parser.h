#ifndef PARSER_H_INCLUDED 
#define PARSER_H_INCLUDED

#include "buffer.h"

#define logParsingError printf

#define PACKET_HEADER_STRING "#ds"

// used for holding raw bytestream from multiple packets in a packetset
typedef struct PacketSetData
{
    uint8_t buffer[MAX_DATA_SIZE_PER_TICK];
    uint32_t length;
    uint16_t version; // version of the encoding scheme
    timestamp_t timestamp; // timestamp from xpc 
} PacketSetData;

bool parsePacket(const uint8_t*, int, Packet*);
void processRawPacket(uint8_t*, int);
void processPacketSet(PacketSet*);

///////////// DATA TYPES DECLARATIONS /////////////

typedef float single_t;
typedef double double_t;

///////////// UINT8_T BUFFER UTILS ///////

// these macros help with the process of pulling bytes off of a uint8_t buffer
// [bufPtr], typecasting it as [type], and storing it in [assignTo]. The bufPtr
// is automatically advanced by the correct number of bytes. assignTo must be of
// type [type], not a pointer.
#define STORE_TYPE(type, bufPtr, assignTo) \
    memcpy(&assignTo, bufPtr, sizeof(type)); bufPtr += sizeof(type) 

#define STORE_CHAR(bufPtr,   assignTo) STORE_TYPE(char,     bufPtr, assignTo) 
#define STORE_INT8(bufPtr,   assignTo) STORE_TYPE(int8_t,   bufPtr, assignTo) 
#define STORE_UINT8(bufPtr,  assignTo) STORE_TYPE(uint8_t,  bufPtr, assignTo) 
#define STORE_INT16(bufPtr,  assignTo) STORE_TYPE(int16_t,  bufPtr, assignTo) 
#define STORE_UINT16(bufPtr, assignTo) STORE_TYPE(uint16_t, bufPtr, assignTo) 
#define STORE_INT32(bufPtr,  assignTo) STORE_TYPE(int32_t,  bufPtr, assignTo) 
#define STORE_UINT32(bufPtr, assignTo) STORE_TYPE(uint32_t, bufPtr, assignTo) 
#define STORE_SINGLE(bufPtr, assignTo) STORE_TYPE(single_t, bufPtr, assignTo) 
#define STORE_DOUBLE(bufPtr, assignTo) STORE_TYPE(double_t, bufPtr, assignTo) 

// these macros help with the process of pulling nElements*sizeof(type) bytes off 
// of a uint8_t buffer [bufPtr], typecasting them as [type], and storing them 
// into a buffer [pAssign]. The bufPtr is automatically advanced by the correct 
// number of bytes. assignTo must be of type [type*], i.e. a pointer.
#define STORE_TYPE_ARRAY(type, bufPtr, pAssign, nElements) \
    memcpy(pAssign, bufPtr, nElements*sizeof(type)); bufPtr += sizeof(type) * nElements

#define STORE_CHAR_ARRAY(   bufPtr, pAssign, nElements) STORE_TYPE_ARRAY(char,     bufPtr, pAssign, nElements) 
#define STORE_INT8_ARRAY(   bufPtr, pAssign, nElements) STORE_TYPE_ARRAY(int8_t,   bufPtr, pAssign, nElements) 
#define STORE_UINT8_ARRAY(  bufPtr, pAssign, nElements) STORE_TYPE_ARRAY(uint8_t,  bufPtr, pAssign, nElements) 
#define STORE_INT16_ARRAY(  bufPtr, pAssign, nElements) STORE_TYPE_ARRAY(int16_t,  bufPtr, pAssign, nElements) 
#define STORE_UINT16_ARRAY( bufPtr, pAssign, nElements) STORE_TYPE_ARRAY(uint16_t, bufPtr, pAssign, nElements) 
#define STORE_INT32_ARRAY(  bufPtr, pAssign, nElements) STORE_TYPE_ARRAY(int32_t,  bufPtr, pAssign, nElements) 
#define STORE_UINT32_ARRAY( bufPtr, pAssign, nElements) STORE_TYPE_ARRAY(uint32_t, bufPtr, pAssign, nElements) 
#define STORE_SINGLE_ARRAY( bufPtr, pAssign, nElements) STORE_TYPE_ARRAY(single_t, bufPtr, pAssign, nElements) 
#define STORE_DOUBLE_ARRAY( bufPtr, pAssign, nElements) STORE_TYPE_ARRAY(double_t, bufPtr, pAssign, nElements) 

#endif // ifndef PARSER_H_INCLUDED 

