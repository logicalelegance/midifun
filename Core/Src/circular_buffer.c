/*
 * circular_buffer.c
 *
 * Generic byte-oriented circular buffer implementation. Adapted from "Making Embedded Systems" by Elecia White.
 * Supports multiple-byte reads and writes.
 * 01/20/2020
 *
 * Chris White
 */

#include <stddef.h>
#include "circular_buffer.h"

static inline eCircularBufferError buffer_is_valid(circular_buffer_t *cb) {
	// Check proper initialization
	if ((cb->data == NULL) || (cb->size == 0) || (cb->size & 0x01)) {
		return eCircularBufferNotValid;
	}
	return eCircularBufferOk;
}

eCircularBufferError circularBuffer_get_length(circular_buffer_t *cb, uint16_t *length) {

	if ((buffer_is_valid(cb) == eCircularBufferOk) && (length != NULL)) {
		*length = (cb->write_pos - cb->read_pos) & (cb->size - 1);
		return eCircularBufferOk;
	}
	return eCircularBufferNotValid;
}

eCircularBufferError circularBuffer_write_bytes(circular_buffer_t *cb, uint8_t *data, uint16_t len) {
	uint16_t curr_length = 0;
	eCircularBufferError status;

	status = circularBuffer_get_length(cb, &curr_length);
	if (status == eCircularBufferOk) {
		if (curr_length == (cb->size - len)) {
			return eCircularBufferFull; // Can't fit!
		}

		memcpy(&cb->data[cb->write_pos], data, len);
		cb->write_pos = (cb->write_pos + 1) & (cb->size - len); // Atomic?
		return eCircularBufferOk;
	} else {
		return status;
	}
}

eCircularBufferError circularBuffer_read_bytes(circular_buffer_t *cb, uint8_t *data, uint16_t *read_len) {
	uint16_t curr_length = 0;
	eCircularBufferError status;

	status = circularBuffer_get_length(cb, &curr_length);
	if (status == eCircularBufferOk) {
		if (curr_length == 0) {
			return eCircularBufferEmpty;
		}

		if (curr_length < *read_len) { // Underflow, read as many bytes as we can
			*read_len = curr_length;
		}
		memcpy(data, &cb->data[cb->read_pos], *read_len);
		cb->read_pos = (cb->read_pos + 1) & (cb->size - *read_len);
		return eCircularBufferOk;
	} else {
		return status;
	}
}



