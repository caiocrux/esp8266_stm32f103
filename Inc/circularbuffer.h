/*
 * circularbuffer.h
 *
 *  Created on: 15 de mai de 2018
 *      Author: tpv
 */

#ifndef CIRCULARBUFFER_H_
#define CIRCULARBUFFER_H_
#include <inttypes.h>
#include <stdbool.h>
typedef struct
{
    uint8_t * const buffer;
    int head;
    int tail;
    int count;
    const int maxLen;
}circBuf_t;

#define CIRCBUF_DEF(x,y) uint8_t x##_space[y];circBuf_t x = { x##_space,0,0,0,y}

uint8_t circBufPush(circBuf_t *c, uint8_t data);
uint8_t circBufPop(circBuf_t *c);
bool circBufReset(circBuf_t *c);
bool circBufEmpty(circBuf_t c);
bool circBufFull(circBuf_t c);
uint8_t circBufCount(circBuf_t c);
#endif /* CIRCULARBUFFER_H_ */
