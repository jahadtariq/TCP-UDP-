/*******************************************************************
* Project:          Implementace zretezeneho RDT
* Subject:          IPK - Pocitacove komunikace a site
* File:             rdt.h
* Date:             20.4.2011
* Lasta modified:   20.4.2011
* Author:           Radim Loskot xlosko01(at)stud.fit.vutbr.cz
*
* Brief: Header file with inline methods which represents access layer to data 
*        and header information inside RTP packet of implemented RDP protocol.
*
*******************************************************************/
/**
* @file rdt.h
*
* @brief Header file with inline methods which represents access layer to data 
* @brief and header information inside RTP packet of implemented RDP protocol.
* @author Radim Loskot xlosko01(at)stud.fit.vutbr.cz
*/

#ifndef RDT_H_
#define RDT_H_

#include <limits.h>
#include <errno.h>
#include <ctype.h>

#define SUM_OFFSET    0           // Offset of checksum
#define SEQ_OFFSET    2           // Offset of sequence number
#define LEN_OFFSET    6           // Offset of data length number
#define FLAGS_OFFSET  8           // Packet flags offset

#define HEADER_OFFSET 2           // Header offset - without checksum
#define DATA_OFFSET  10           // Data offset

/**
 * Packet structure.
 */
typedef struct packet {
    unsigned int   seq;      /**< sequence number of packet */
    unsigned short len;      /**< data length */
    unsigned short flags;    /**< flags */
    char *data;              /**< transfering data */
} RDTPacket;

/**
 * Enum of packet flags.
 */
enum flags {
    ACK          = 0x01,     /**< enum packet with ACK */
    NACK         = 0x02,     /**< enum packet with NACK */
    END          = 0x04      /**< enum packet finishing transfer */
    // 0x08, 0x0F etc...
};

/**
 * Converts unsigned short type into 2-item char array.
 * @param number Number to be converted.
 * @param bytes Pointer to array where converted number will be stored. 
 */
static inline void ushort2bytes(unsigned short number, char *bytes) {
    bytes[0] = (number >> 8) & 0xFF;
    bytes[1] = number & 0xFF; 
}   

/**
 * Converts unsigned short stored in 2-item char array into unsigned short datatype.
 * @param bytes Pointer to array where is stored coded number. 
 * @return Converted unsigned short.
 */
static inline unsigned short bytes2ushort (char *bytes) {
    return ((unsigned char)bytes[0] << 8) + (unsigned char)bytes[1];
}

/** Converts unsigned int type into 4-item char array.
 * @param number Number to be converted.
 * @param bytes Pointer to array where converted number will be stored. 
 */
static inline void uint2bytes(unsigned int number, char *bytes) {
    bytes[0] = (number >> 24) & 0xFF;
    bytes[1] = (number >> 16) & 0xFF;
    bytes[2] = (number >> 8) & 0xFF;
    bytes[3] = number & 0xFF; 
}

/**
 * Converts unsigned int stored in 4-item char array into unsigned int datatype.
 * @param bytes Pointer to array where is stored coded number. 
 * @return Converted unsigned int.
 */
static inline unsigned int bytes2uint (char *bytes) {
    return ((unsigned char)bytes[0] << 24) + ((unsigned char)bytes[1] << 16) +
           ((unsigned char)bytes[2] << 8) + (unsigned char)bytes[3];
}

/*
 * Source code took from RFC 1071 and edited.
 * See: http://www.faqs.org/rfcs/rfc1071.html
 * @param packet Packet from which will be checksum calculated.
 * @param size Size of packet.
 * @return Checksum of packet.  
*/
static inline unsigned short checksum(unsigned char *packet, size_t size) {
    register long sum = 0; // Using fully architecture as possible
    unsigned short *_packet = (unsigned short *) packet;
    
    /* Do it quickly - UNFOLDED - but depends on architecture */
    while(size > 1)  {
        sum += *_packet++;
        size -= 2;
    }

    /* Do sum of the rest last byte whether exists */
    if( size > 0 )
        sum += *(unsigned char *)_packet;

    /* Fold 32-bit sum to 16 bits */
    while (sum>>16)
        sum = (sum & 0xffff) + (sum >> 16);
               
    return ~sum;
}

/**
 * Calculates checksum and comapares with checksum sotred inside packet.
 * @param packet Packet which will be tested.
 * @param len Size of packet.
 * @return Return 1 whether checksum passed else returns 0.
 */
static inline int testCheckSum(char *packet, unsigned short len) {
    return checksum((unsigned char *)&packet[HEADER_OFFSET], len - HEADER_OFFSET) == bytes2ushort(packet); 
}

/**
 * Retrieves data from packet.
 * @param packet Packet containing data.
 * @param data Pointer where data will be stored.
 * @return Return size of packet data.
 */
static inline size_t getData(char *packet, char *data) {
    unsigned short int len = bytes2ushort(&packet[LEN_OFFSET]);
    memcpy(data, &packet[DATA_OFFSET], len); 
    return len;
}

/**
 * Test whether has packet defined flags.
 * @param packet Tested packet.
 * @param flags Demeanded flags to be tested.
 * @return Returns 1 whether packet has defined flags else returns 0.
 */
static inline int hasFlags(char *packet, unsigned short flags) {
    return bytes2ushort(&packet[FLAGS_OFFSET]) & flags;
}

/**
 * Returns sequence number from packet.
 * @param packet Pointer to packett.
 * @return Returns sequence number from packet.
 */
static inline unsigned int seqNumber(char *packet) {
    return bytes2uint(&packet[SEQ_OFFSET]);
}

/**
 * Retrieves packet length from already existing packet.
 * @param packet Pointer to packett.
 * @return Returns packet size.
 */
static inline unsigned short packetLen(char *packet) {
    return DATA_OFFSET + bytes2ushort(&packet[LEN_OFFSET]);
}

/**
 * Returns data length from packet.
 * @param packet Pointer to packett.
 * @return Returns data length from packet.
 */
static inline unsigned short dataLen(char *packet) {
    return bytes2ushort(&packet[LEN_OFFSET]);
}

static inline char *makePacket(RDTPacket packet) {
    char *_packet = malloc(DATA_OFFSET + packet.len);
    
    if (_packet == NULL) return 0;
    
    memset(_packet, 0,DATA_OFFSET + packet.len); 

    uint2bytes(packet.seq, &_packet[SEQ_OFFSET]);
    ushort2bytes(packet.len, &_packet[LEN_OFFSET]);
    ushort2bytes(packet.flags, &_packet[FLAGS_OFFSET]);
    if (packet.len) {
        memcpy(&_packet[DATA_OFFSET], packet.data, packet.len);
    }
    ushort2bytes(checksum((unsigned char *)&_packet[HEADER_OFFSET], DATA_OFFSET + packet.len - HEADER_OFFSET), _packet);
    return _packet;
}

#endif /* UDT_H_ */

/*** End of file udt.h ***/
