/*******************************************************************
* Project:          Implementace zretezeneho RDT
* Subject:          IPK - Pocitacove komunikace a site
* File:             rdtserver.c
* Date:             20.4.2011
* Lasta modified:   20.4.2011
* Author:           Radim Loskot xlosko01(at)stud.fit.vutbr.cz
*
* Brief: RDT server using sliding stdout print buffer and GO-BACK-N 
*        as acknowledgement algorithm.
*
*******************************************************************/
/**
* @file rdtserver.c
*
* @brief RDT server using sliding stdout print buffer and GO-BACK-N 
* @brief as acknowledgement algorithm.
* @author Radim Loskot xlosko01(at)stud.fit.vutbr.cz
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define _GNU_SOURCE
#include <getopt.h>
#include <string.h>
#include <fcntl.h>
#include <stdarg.h>
#include "../libs/udt.h"
#include "../libs/rdt.h"
#include "rcv_buffer.h"
#include <sys/time.h>
#include <signal.h>
#include <arpa/inet.h>
#include <limits.h>

// Max recieving packet size 
#define RCV_PACKETSIZE 100

in_addr_t dest_addr = 0x7f000001;    /**< destination address - only localhost */
in_port_t src_port = 4040;              /**< local incomming port */
in_port_t dest_port = 4030;             /**< destination port - where to send */
int udt;                             /**< socket descriptor */
#define PACKETSIZE 22                /**< Out-coming packet size */

/**
 * Enum of all handled errors.
 */
enum errors {
    E_MALLOC,       /**< enum Memory allocation error. */
    E_UDTSEND,      /**< enum Some error caused fail of sending current packet. */
    E_BADPARAMS     /**< enum Bad run parameters from cmd-line. */
};

/**
 * Messages to handled errors.
 */
const char* ERRORS[] = {
    "Error: Memory allocation failed!\n",             // E_MALLOC
    "Error: Unable send packet.\n",                   // E_UDTSEND
    "Error: Missing source or destination port!\n"    // E_BADPARAMS
};

/**
 * Enum of all messages/warnings.
 */
enum msgs {
    MSG_MANYPARAMS,   /**< enum Many run params specified. */
    MSG_USAGE         /**< enum Usage message. */
};

/**
 * Array with app messages.
 */
const char *MSGS[] = {
    "Warning: Too many options/arguments!\n",             // MSG_MANYPARAMS
    "Usage: rdtclient -s source_port -d dest_port\n"      // MSG_USAGE
};


char PACKET_BUFFER[PACKETSIZE];     /**< Packet buffer for preparing packets */
                                    
TBuffer output_buff;                /**< STDOUT print buffer */

/**
 * Prints error.
 * @param error ID of error to be printed. 
 */
void printError(int error) {
    fprintf(stderr, "%s", ERRORS[error]);
    perror("Caused: ");
	destroyBuffer(&output_buff);
    exit(1);
}

/**
 * Sends acknowledgement to sequence.
 * @param seq Sequence number to acknowledge. 
 */
void sendACK(unsigned int seq) {
    // Praparing packet to send
    RDTPacket packet;
    packet.seq = seq;
    packet.len = 0;
    packet.flags = ACK;
    packet.data = NULL;

    // Making final packet from packet structure
    char *_packet = makePacket(packet);
    if(_packet == NULL) {
		printError(E_MALLOC);
    }
    
    // Sending packet
	if (!udt_send(udt, dest_addr, dest_port, _packet, packetLen(_packet))) {
		printError(E_UDTSEND);
	}

    free(_packet);
}

/**
 * Sends non-acknowledgement to sequence.
 * @param seq Sequence number to not acknowledge. 
 */
void sendNACK(unsigned int seq) {
    // Praparing packet to send
    RDTPacket packet;
    packet.seq = seq;
    packet.len = 0;
    packet.flags = NACK;
    packet.data = NULL;

    // Making final packet from packet structure
    char *_packet = makePacket(packet);
    if(_packet == NULL) {
		printError(E_MALLOC);
    }

    // Sending packet
	if (!udt_send(udt, dest_addr, dest_port, _packet, packetLen(_packet))) {
		printError(E_UDTSEND);
	}

    free(_packet);
}

/**
 * Buffering packet to STDOUT buffer.
 * @param packet Packet to store into buffer. 
 */
void buffData(char *packet) {
    unsigned int seq = seqNumber(packet);
    
    if (!isBuffered(&output_buff, seq)) { // Buffer data only whether are not already buffered
        // Allocate mamory for data buffer
        char *data = malloc((dataLen(packet) + 1) * sizeof(char));
        if(data == NULL) {
            printError(E_MALLOC);
        }
        
        getData(packet, data);
        data[dataLen(packet)] = '\0';
        
        // Store to buffer
        if (data != NULL) {
            toBuffer(&output_buff, seq, data);
        }
    }  
}

/**
 * Proccesses run params. Returns 0/1 or finishes app in some cases.
 * @param argc Number of run params. 
 * @param argv Array with run params.
 * @return Return 0 on success or 1 on fail.    
 */
// int readParams(int argc, char **argv) {
// 	char ch;
// 	while ((ch = getopt(argc,argv,"s:d:")) != -1) {
// 		switch(ch) {
// 		case 's':  // Source port
// 			src_port = atol(optarg);
// 			break;
// 		case 'd':  // Destination port
// 			dest_port = atol(optarg);
// 			break;
// 		case '?':  // Unknown flag, print error
// 			fprintf(stderr, "%s", MSGS[MSG_USAGE]);;
//         }
// 	}

// 	// Missing params or bad params.
// 	if (src_port == 0 || dest_port == 0) {
// 		printError(E_BADPARAMS);
// 	}
	
// 	// Many params
//     if (argc > 5) {
// 		fprintf(stderr, "%s", MSGS[MSG_MANYPARAMS]);
// 	}
// 	return 1;
// }

int main(int argc, char **argv ) {
    printf(" Listening to PORT \n");
	char recv_packet[RCV_PACKETSIZE];
    printf(" Reading Packet \n");

    initBuffer(&output_buff);     // Initialize STDOUT buffer.
    // readParams(argc, argv);       // Reads params.
	udt = udt_init(src_port);     // Returns socket descriptor.

	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(udt, &readfds);
	while (select(udt+1, &readfds, NULL, NULL, NULL)) {
		if (FD_ISSET(udt, &readfds)) {	
			int n = udt_recv(udt, recv_packet, RCV_PACKETSIZE, NULL, NULL);
            printf(" Packet Recieved \n");
			
            // Check whether has at least header and checksum passes
            printf(" Checking Packet Header and Checksum\n");
   			if (n >= DATA_OFFSET && testCheckSum(recv_packet, n)) {
                if (!hasFlags(recv_packet, END)) {
                    // Buffering and sending ACK
                    buffData(recv_packet);
                    printf(" \n Sending Acknowledgment\n");
                    sendACK(seqNumber(recv_packet));
                } else { // END flags specified - exiting
                    break;
                }
            } else { // Bad packet - send NACK of first unfinished
                printf(" Bad Packet recieved sending NACK");
                sendNACK(firstBlank(&output_buff));
            } 
		}
		// waiting for new packets
		FD_ZERO(&readfds);
		FD_SET(udt, &readfds);
	}
	
	destroyBuffer(&output_buff);

	return EXIT_SUCCESS;
}

/*** End of file rdtserver.c ***/
