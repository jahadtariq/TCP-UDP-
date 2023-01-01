/*******************************************************************
* Project:          Implementace zretezeneho RDT
* Subject:          IPK - Pocitacove komunikace a site
* File:             rcv_buffer.h
* Date:             20.4.2011
* Lasta modified:   20.4.2011
* Author:           Radim Loskot xlosko01(at)stud.fit.vutbr.cz
*
* Brief: Header file of buffer with random filling which prints on stdout.
*
*******************************************************************/
/**
* @file rcv_buffer.h
*
* @brief Header file of buffer with random filling which prints on stdout.
* @author Radim Loskot xlosko01(at)stud.fit.vutbr.cz
*/

#define BUFFERSIZE 16          // Size of STDOUT buffer

/**
 * STDOUT print buffer structure.
 */
typedef struct {
    char *data[BUFFERSIZE];     /**< data buffers */
    unsigned int first_seq;     /**< first unbufered sequence */
    unsigned int last_seq;      /**< last buffered sequence */
} TBuffer;

/**
 * Initializes buffer.
 * @param buffer Pointer to buffer.
 */
void initBuffer(TBuffer *buffer);

/**
 * Stores data to STDOUT buffer.
 * @param buffer Pointer to buffer.
 * @param seq_num Sequence number of data.
 * @param data Pointer to data to be stored.
 * @return Return the same data on success or NULL on fail.    
 */
char *toBuffer(TBuffer *buffer, unsigned int seq_num, char *data);

/**
 * Destroyes buffer.    
 */
void destroyBuffer(TBuffer *buffer);

/**
 * Finds first demanded sequence number of data which has not been buffered.
 * @param buffer Pointer to buffer.
 * @return Returns sequence number of first unbuffered data.    
 */
unsigned int firstBlank(TBuffer *buffer);

/**
 * Checks whether is demanded sequence of data already buffered.
 * @param buffer Pointer to buffer.
 * @param seq_num Sequence number of data.
 * @return Returns 1 whether are data buffered else returns 0s.    
 */
int isBuffered(TBuffer *buffer, unsigned int seq_num);

/*** End of file rcv_buffer.h ***/
