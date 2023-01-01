/*******************************************************************
* Project:          Implementace zretezeneho RDT
* Subject:          IPK - Pocitacove komunikace a site
* File:             snd_window.h
* Date:             20.4.2011
* Lasta modified:   20.4.2011
* Author:           Radim Loskot xlosko01(at)stud.fit.vutbr.cz
*
* Brief: Header file of sliding window - TWindow struture and its methods.
*
*******************************************************************/
/**
* @file snd_window.h
*
* @brief Header file of sliding window - TWindow struture and its methods.
* @author Radim Loskot xlosko01(at)stud.fit.vutbr.cz
*/

#include <time.h>

// Window size
#define WINDOWSIZE 5

/**
 * Window structure.
 */
typedef struct {
    char *packets[WINDOWSIZE];           /**< array with packets */
    time_t timestamps[WINDOWSIZE];       /**< sending timestamps for each packet */
    unsigned int first_seq;              /**< first set sequence */
    unsigned int last_seq;               /**< last set sequence */
} TWindow;

/**
 * Initializing window.
 * @param window Pointer to window.
 */
void initWindow(TWindow *window);

/**
 * Gets packet from window.
 * @param window Pointer to window.
 * @param seq_num Sequence number of packet. 
 * @return Return packet of defined sequence number. 
 */
char *getPacket(TWindow *window, unsigned int seq_num);

/**
 * Stores packet to window.
 * @param window Pointer to window.
 * @param seq_num Sequence number of packet. 
 * @param packet Packet to be stored.  
 * @return Return the same packet on success or NULL on fail. 
 */
char *storePacket(TWindow *window, unsigned int seq_num, char *packet);

/**
 * Removes packet from window.
 * @param window Pointer to window.
 * @param seq_num Sequence number of packet. 
 * @return Return 1 on success else 0. 
 */
int removePacket(TWindow *window, unsigned int seq_num);

/**
 * Destroyes window.
 * @param window Pointer to window.
 */
void destroyWindow(TWindow *window);

/**
 * Checks for non-full window.
 * @param window Pointer to window.
 * @return Return 0 on full window else return 1. 
 */
int isAvailable(TWindow *window);

/**
 * Checks for empty window.
 * @param window Pointer to window.
 * @return Return 0 on non-empty window else return 1. 
 */
int isEmpty(TWindow *window);

/**
 * Removes packets from window to the specified sequence.
 * @param window Pointer to window.
 * @param seq_num Sequence number of packet. 
 */
void removeTo(TWindow *window, unsigned int seq_num);

/*** End of file snd_window.h ***/
