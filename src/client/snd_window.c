/*******************************************************************
* Project:          Implementace zretezeneho RDT
* Subject:          IPK - Pocitacove komunikace a site
* File:             snd_window.c
* Date:             20.4.2011
* Lasta modified:   20.4.2011
* Author:           Radim Loskot xlosko01(at)stud.fit.vutbr.cz
*
* Brief: Source file defining methods on sliding window - TWindow struture.
*
*******************************************************************/
/**
* @file snd_window.c
*
* @brief Source file defining methods on sliding window - TWindow struture.
* @author Radim Loskot xlosko01(at)stud.fit.vutbr.cz
*/

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "snd_window.h"

/**
 * Initializing window.
 * @param window Pointer to window.
 */
void initWindow(TWindow *window) {
    for (int i = 0; i < WINDOWSIZE; i++) {
        window->packets[i] = NULL;
        window->timestamps[i] = UINT_MAX;
    }
    window->first_seq = 0;
    window->last_seq = 0;
}

/**
 * Checks for non-full window.
 * @param window Pointer to window.
 * @return Return 0 on full window else return 1. 
 */
int isAvailable(TWindow *window) {

    if (window->first_seq + WINDOWSIZE > window->last_seq + 1) {
        return 1;
    }

    return 0;
}

/**
 * Checks for empty window.
 * @param window Pointer to window.
 * @return Return 0 on non-empty window else return 1. 
 */
int isEmpty(TWindow *window) {
    if (((window->first_seq == window->last_seq + 1)              // Last is set behind first awaiting
        || ((window->first_seq == 0) && (window->last_seq == 0))) // Just initialized and waiting for 0 seq
        && (window->packets[window->first_seq % WINDOWSIZE] == NULL)) {
        return 1;
    }

    return 0;
}

/**
 * Gets packet from window.
 * @param window Pointer to window.
 * @param seq_num Sequence number of packet. 
 * @return Return packet of defined sequence number. 
 */
char *getPacket(TWindow *window, unsigned int seq_num) {
    // Check for range
    if ((window->first_seq <= seq_num) &&
        (seq_num < window->first_seq + WINDOWSIZE)) {
        return window->packets[seq_num % WINDOWSIZE];
    }

    return NULL;
}

/**
 * Stores packet to window.
 * @param window Pointer to window.
 * @param seq_num Sequence number of packet. 
 * @param packet Packet to be stored.  
 * @return Return the same packet on success or NULL on fail. 
 */
char *storePacket(TWindow *window, unsigned int seq_num, char *packet) {
    // Check for ranges and empty place
    if ((window->first_seq <= seq_num) &&
        (seq_num < window->first_seq + WINDOWSIZE) &&
        (window->packets[seq_num % WINDOWSIZE] == NULL)) {
        
        // Setting new last sequence
        if (window->last_seq < seq_num) {
            window->last_seq = seq_num;
        }
        
        // Success, returning same packet from window
        return window->packets[seq_num % WINDOWSIZE] = packet;
    }

    return NULL;   // Fail
}

/**
 * Slides window.
 * @param window Pointer to window.
 */
void slideWindow(TWindow *window) {
    // Slide from begin to end
    while (window->first_seq <= window->last_seq) {
        // Slide until first not null packet is reached
        if (window->packets[(window->first_seq) % WINDOWSIZE] == NULL) {
            window->first_seq++;
        } else {
            break;
        }
    }
}

/**
 * Removes packet from window.
 * @param window Pointer to window.
 * @param seq_num Sequence number of packet. 
 * @return Return 1 on success else 0. 
 */
int removePacket(TWindow *window, unsigned int seq_num) {
    unsigned int offset = seq_num % WINDOWSIZE;

    // Check for ranges
    if ((window->first_seq <= seq_num) &&
        (seq_num < window->first_seq + WINDOWSIZE) &&
        (window->packets[offset] != NULL)) {

        // Initializig to default
        free(window->packets[offset]);
        window->packets[offset] = NULL;
        window->timestamps[offset] = UINT_MAX;
        slideWindow(window);                       // Try to slide window
        return 1;
    }

    return 0;
}

/**
 * Removes packets from window to the specified sequence.
 * @param window Pointer to window.
 * @param seq_num Sequence number of packet. 
 */
void removeTo(TWindow *window, unsigned int seq_num) {
    while (window->first_seq < seq_num) {
        removePacket(window, window->first_seq);
    }
}

/**
 * Destroyes window.
 * @param window Pointer to window.
 */
void destroyWindow(TWindow *window) {
    for (int i = 0; i < WINDOWSIZE; i++) {
        if (window->packets[i] != NULL) {
            free(window->packets[i]);
            window->packets[i] = NULL;
            window->timestamps[i] = UINT_MAX;
        }
    }
}
/*** End of file snd_window.c ***/
