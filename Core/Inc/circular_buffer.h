/*
 * circular_buffer.h
 *
 * Generic circular buffer implementation. Adapted from "Making Embedded Systems" by Elecia White
 *
 * 01/20/2020
 *
 * Chris White
 */

#include <stdint.h>

// Circular buffer data structure
typedef struct {
	uint8_t *data;   // Memory buffer to hold the entire ring
	uint16_t size;  // Size in bytes of buffer, power of 2
	uint16_t read_pos; // Current read position, 0 to size - 1
	uint16_t write_pos; // Current write position, 0 to size - 1
} circular_buffer_t;

typedef enum {
	eCircularBufferOk = 0,
	eCircularBufferFull = 1,
	eCircularBufferEmpty = 2,
	eCircularBufferNotValid = 3
} eCircularBufferError;


eCircularBufferError circularBuffer_get_length(circular_buffer_t *cb, uint16_t *length);
eCircularBufferError circularBuffer_write_bytes(circular_buffer_t *cb, uint8_t *data, uint16_t len);
eCircularBufferError circularBuffer_read_bytes(circular_buffer_t *cb, uint8_t *data, uint16_t *read_len);


