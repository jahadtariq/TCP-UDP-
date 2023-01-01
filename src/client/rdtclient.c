/*******************************************************************
* Project:          Implementace zretezeneho RDT
* Subject:          IPK - Pocitacove komunikace a site
* File:             rdtclient.c
* Date:             20.4.2011
* Lasta modified:   20.4.2011
* Author:           Radim Loskot xlosko01(at)stud.fit.vutbr.cz
*
* Brief: RDT client using sliding window for sending packets.
*
*******************************************************************/
/**
* @file rdtclient.c
*
* @brief RDT client using sliding window for sending packets.
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
#include "snd_window.h"
#include <arpa/inet.h>
#include <sys/time.h>
#include <time.h>
#include <sys/types.h>
#include <signal.h>
#include <time.h>

// Max length of accpeting line from stdin - but everything after 80 char will be removed
#define MAXLINE    500

// Packet size
// In real only 90 - just for sure due to adding some info to header in future
#define PACKETSIZE 100
// Length of sending data
#define DATASIZE    80

// Delay in ms, when will be checked whether is any packet lost - timeout
#define RETRY      150
// Delay after which is packet considered as lost
#define LINKDELAY  600

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

char PACKET_BUFFER[PACKETSIZE];        

TWindow window;                      /**< sliding window struture */
in_addr_t dest_addr = 0x7f000001;    /**< destination address - only localhost */
in_port_t src_port = 4030;              /**< local incomming port */
in_port_t dest_port = 4040;             /**< destination port - where to send */
unsigned int cnt_seq = 0;            /**< current sequence to send */
sigset_t sigmask;                    /**< signal mask */
struct itimerval timer;              /**< timer structure */
int udt;                             /**< socket descriptor */

/**
 * Starts timer.
 */
void startTimer() {
    sigprocmask(SIG_UNBLOCK, &sigmask, NULL);
    setitimer(ITIMER_REAL, &timer, NULL);
}

/**
 * Stops timer - block SIGALRM signal.
 */
void stopTimer() {
    sigprocmask(SIG_BLOCK, &sigmask, NULL);
}

/**
 * Prints error.
 * @param error ID of error to be printed. 
 */
void printError(int error) {
    fprintf(stderr, "%s", ERRORS[error]);
    perror("Caused: ");
    stopTimer();
	destroyWindow(&window);
    exit(1);
}

/**
 * Allocates memory for packet and makes new one with specified data.
 * @param data Data which will be appedned to the packet. 
 */
char *makeDataPacket(char * data) {
    // Praparing packet to send
    RDTPacket packet;
    packet.seq = cnt_seq;
    packet.len = strlen(data);
    packet.data = data;
    packet.flags = 0x00;
    
    // Correcting data length - only 80 chars to send
    if (packet.len > DATASIZE) {
        packet.len = DATASIZE; 
    }
    
    // Making final packet from packet structure
    char *_packet = makePacket(packet);
    if(_packet == NULL) {
        printError(E_MALLOC);
    }
    
    return _packet;  
}

/**
 * Sends packet to remote host.
 * @param packet Packet to send. 
 */
void sendPacket(char *packet) {
    if (packet != NULL) {  // Frist check whether there is any packet
        if (!udt_send(udt, dest_addr, dest_port, packet, packetLen(packet))) {
        	printError(E_UDTSEND);   // Sending failed
        }
        // After success - store send time
        struct timeval sended;
        gettimeofday(&sended, NULL);
        window.timestamps[seqNumber(packet) % WINDOWSIZE] = sended.tv_sec * 1000 + sended.tv_usec/1000;
    }
}

/**
 * Timer handler - resends packets from window which are probably lost.
 * @param packet Packet to send. 
 */
void resendPackets(int sig) {
    if (sig == SIGALRM) {
        // Getting current time
        struct timeval request;
        gettimeofday(&request, NULL);
        time_t timestamp = request.tv_sec * 1000 + request.tv_usec/1000;
        register unsigned int offset;

        // Sending packet only from non empty window    
        if (!isEmpty(&window)) {
            // Go through all sequences inside window
            for (int i = 0; i < WINDOWSIZE; i++) {
                offset = (window.first_seq + i) % WINDOWSIZE;
                
                // Resend packet or finish whether is window empty or there is still time
                if (// Abs just due to change of sys time to the past
                    (abs(timestamp - window.timestamps[offset]) > LINKDELAY)
                    // Reached empty window sequece 
                    && (window.packets[offset] != NULL)) {
                    
                        sendPacket(window.packets[offset]);
                        
                } else { // Still time or reached empty sequnce inside window
                    break;
                }
            }
        }
        
    	// Re-install handler.
    	signal(SIGALRM, resendPackets);
	}
}

/**
 * Sets interval delay to timer.
 * @param delay Delay of timer tick in ms. 
 */
void setTimer(int delay) {
	signal(SIGALRM, resendPackets);

	timer.it_interval.tv_sec = delay / 1000; 	// sets an interval of the timer
	timer.it_interval.tv_usec = (delay % 1000) * 1000;	
	timer.it_value.tv_sec = delay / 1000;		// sets an initial value
	timer.it_value.tv_usec = (delay % 1000) * 1000;

	sigemptyset(&sigmask);
	sigaddset(&sigmask, SIGALRM);
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

/**
 * Closes transfer with rdtserver. 
 */
void closeConnection() {
    // Prapare packet with END flag
    RDTPacket packet;
    packet.seq = 0;
    packet.len = 0;
    packet.data = NULL;
    packet.flags = END;
    
    // Creating packet
    char *_packet = makePacket(packet);
    if(_packet == NULL) {
		printError(E_MALLOC);
    }

    // Sending END packet just 5-times for sure with delay
    for(int i = 1; i <= 5; i++) {
        if (!udt_send(udt, dest_addr, dest_port, _packet, packetLen(_packet))) {
		  printError(E_UDTSEND);
        }
        usleep(100); // Sending delay of one packet
    }
    
    free(_packet);
}


int main(int argc, char **argv ) {
    
	char input_line[MAXLINE];     /**< input line buffer - filling from stdin */
	char recv_packet[PACKETSIZE]; /**< recieving packet buffer */
	char *packet;                 /**< packet pointer */
	int res;                      /**< returned value of select */
	int setSTDIN;                 /**< is set to 1 whether is needed reading from stdin */
	
	initWindow(&window);          // Initialize sliding window.
    // readParams(argc, argv);       // Reads params.
    
    setTimer(RETRY);              // Sets retry delay of resending packets.
	udt = udt_init(src_port);     // Returns socket descriptor.

    printf(" In main Fucntion.\n");

	fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK); // Make stdin reading non-clocking.

    // Setting stdin and udt descriptors to the select awaiting SET
    printf(" Reading Data Now .\n");
	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(udt, &readfds);
	FD_SET(STDIN_FILENO, &readfds);
	// Wait until new data are on stdin or new incomming packet
    int A_NO = 0;
    while ((res = select(udt+1, &readfds, NULL, NULL, NULL)) != 0) {
    if (res == -1) continue; // Select was interupted probablz due to timer
   
		if (FD_ISSET(udt, &readfds)) {	// New incomming packet
            stopTimer();                // Stopping timer - working with shared memory
			int n = udt_recv(udt, recv_packet, PACKETSIZE, NULL, NULL);
            // Check whether has at least header and checksum passes
   			if (n >= DATA_OFFSET && testCheckSum(recv_packet, n)) {
                if (hasFlags(recv_packet,ACK)) { 
                    A_NO++;
                    printf(" Acknowledgment # %d \n",A_NO);         // Ack recieved
                    removePacket(&window, seqNumber(recv_packet));
                    // Recieved last ACK packet, exiting
                    if (feof(stdin) && isEmpty(&window)){
                        printf(" End of file reached. \n");
                        break;
                    } 
                } else if (hasFlags(recv_packet,NACK)) {  // Nack recieved 
                    if ((packet = getPacket(&window, seqNumber(recv_packet))) != NULL) {
                        printf(" Retransmitting packet.. \n");
                        sendPacket(packet);
                        removeTo(&window, seqNumber(recv_packet));
                    }
                }
            } else {
                // Bad packet or checksum - try to send first packet from window
    			if (!isEmpty(&window)) {
                    sendPacket(window.packets[window.first_seq % WINDOWSIZE]);
                }
            }
		    startTimer();              // Re-store timer
		}

        // Reading from STDIN only whether window has available sequences
	    if ((setSTDIN = isAvailable(&window))) {
    		if (FD_ISSET(STDIN_FILENO, &readfds)) { // we have read a new line to send
                if (fgets(input_line, MAXLINE, stdin) != NULL ) {
                    stopTimer();
                    printf(" Creating Packet \n");
        			packet = makeDataPacket(input_line);
                    printf(" Sending Packet \n");
        			sendPacket(packet);
                    printf(" Storing Packet to Window\n");
        			storePacket(&window, cnt_seq, packet);
                    printf(" Incrementing Sequence Number\n");
        			cnt_seq++;
        			startTimer();
        		}
    		} else {
                // EOF - exiting on empty window
                if (feof(stdin)) {
                    setSTDIN = 0;
                    if (isEmpty(&window)) break;
                }
            }
    	} 
                     
		// Settings select fd set
		FD_ZERO(&readfds);
		FD_SET(udt, &readfds);
		// Whether window is full, block reading from STDIN - saves CPU
		if (setSTDIN) FD_SET(STDIN_FILENO, &readfds);
	}
	
	stopTimer();
	closeConnection();
	destroyWindow(&window);
	return EXIT_SUCCESS;
}
/*** End of file rdtclient.c ***/
