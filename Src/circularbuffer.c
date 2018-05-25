/*
 * circularbuffer.c

 *
 *  Created on: 15 de mai de 2018
 *      Author: tpv
 */
#include <stdbool.h>
#include "circularbuffer.h"

uint8_t circBufPush(circBuf_t *c, uint8_t data) {
    int next = c->head + 1;
    if (next >= c->maxLen)
        next = 0;

    // Cicular buffer is full
    if (next == c->tail)
        return -1;  // quit with an error

    c->buffer[c->head] = data;
    c->head = next;
    c->count++;
    return 0;
}

uint8_t circBufPop(circBuf_t *c) {
    // if the head isn't ahead of the tail, we don't have any characters
    if (c->head == c->tail)
        return -1;  // quit with an error

    uint8_t data = c->buffer[c->tail];
    c->buffer[c->tail] = 0;  // clear the data (optional)

    int next = c->tail + 1;
    if(next >= c->maxLen)
        next = 0;
    c->tail = next;
    c->count--;
    return data;
}

bool circBufReset(circBuf_t *c) {
	bool r = false;
    if(c) {
    	c->head = 0;
	    c->tail = 0;
	    c->count = 0;
	    r = true;
	}
    return r;
}

bool circBufEmpty(circBuf_t c) {
    return (c.head == c.tail);
}

bool circBufFull(circBuf_t c) {
	return ((c.head + 1) % c.maxLen) == c.tail;
}

uint8_t circBufCount(circBuf_t c) {
	return c.count;
}

