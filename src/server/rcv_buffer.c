/*******************************************************************
* Project:          Implementace zretezeneho RDT
* Subject:          IPK - Pocitacove komunikace a site
* File:             rcv_buffer.c
* Date:             20.4.2011
* Lasta modified:   20.4.2011
* Author:           Radim Loskot xlosko01(at)stud.fit.vutbr.cz
*
* Brief: Source file defining methods of buffer which prints on stdout.
*
*******************************************************************/
/**
* @file rcv_buffer.c
*
* @brief Source file defining methods of buffer which prints on stdout.
* @author Radim Loskot xlosko01(at)stud.fit.vutbr.cz
*/

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "rcv_buffer.h"

/**
 * Initializes buffer.
 * @param buffer Pointer to buffer.
 */
void initBuffer(TBuffer *buffer) {
    for (int i = 0; i < BUFFERSIZE; i++) {
        buffer->data[i] = NULL;
    }
    buffer->first_seq = 0;
    buffer->last_seq = 0;
}

/**
 * Prints data from buffer on STDOUT whether it is possible.
 * @param buffer Pointer to buffer.
 */
void printBuffer(TBuffer *buffer) {
    // For each buffered data
    while (buffer->first_seq <= buffer->last_seq) {
        unsigned int seq = (buffer->first_seq) % BUFFERSIZE;
        // Print only buffered data in correct order
        if (buffer->data[seq] != NULL) {
            fputs(buffer->data[seq], stdout);
            free(buffer->data[seq]);
            buffer->data[seq] = NULL;
            buffer->first_seq++;
        } else { // There was blank space - cannot be buffered
            break;
        }
    }
}

/**
 * Stores data to STDOUT buffer.
 * @param buffer Pointer to buffer.
 * @param seq_num Sequence number of data.
 * @param data Pointer to data to be stored.
 * @return Return the same data on success or NULL on fail.    
 */
char *toBuffer(TBuffer *buffer, unsigned int seq_num, char *data) {

    unsigned int offset = seq_num % BUFFERSIZE;
    
    // Check for ranges and empty place
    if ((buffer->first_seq <= seq_num) && 
        (seq_num < buffer->first_seq + BUFFERSIZE) &&
        (buffer->data[offset] == NULL)) {
        
        // Setting new last buffered sequence
        if (buffer->last_seq < seq_num) {
            buffer->last_seq = seq_num;
        }
        
        buffer->data[offset] = data;
        printBuffer(buffer);          // Try to print buffer
        
        return buffer->data[offset];
    }
    
    return NULL;
} 

/**
 * Destroyes buffer.    
 */
void destroyBuffer(TBuffer *buffer) {
    for (int i = 0; i < BUFFERSIZE; i++) {
        if (buffer->data[i] != NULL) {
            free(buffer->data[i]);
            buffer->data[i] = NULL;
        }
    }
}

/**
 * Checks whether is demanded sequence of data already buffered.
 * @param buffer Pointer to buffer.
 * @param seq_num Sequence number of data.
 * @return Returns 1 whether are data buffered else returns 0s.    
 */
int isBuffered(TBuffer *buffer, unsigned int seq_num) {
    if ((seq_num < buffer->first_seq) || // already printed 
    // Not printed, but buffered
    ((seq_num <= buffer->last_seq) && (buffer->data[seq_num % BUFFERSIZE] != NULL))) {
        return 1;
    }
    return 0;   
}

/**
 * Finds first demanded sequence number of data which has not been buffered.
 * @param buffer Pointer to buffer.
 * @return Returns sequence number of first unbuffered data.    
 */
unsigned int firstBlank(TBuffer *buffer) {
    return buffer->first_seq;   
}

/*** End of file rcv_buffer.c ***/
