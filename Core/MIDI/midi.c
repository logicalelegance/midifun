/*
 * midi.c
 *
 * MIDI Driver for the STM32 platform, based on HAL.
 *
 * cwhite@logicalelegance.com
 */

#include <stdbool.h>
#include <stm32f3xx_hal.h>
#include "midi.h"
#include "circular_buffer.h"

static struct {
	UART_HandleTypeDef *UART_in; // UART associated with the MIDI IN port
	UART_HandleTypeDef *UART_out; // UART associated with the MIDI OUT port
	circular_buffer_t midi_rx_ring;
	circular_buffer_t midi_tx_ring;
} config;

static uint8_t midi_interrupt_rx_buf[16];
static uint8_t midi_rx_data_buf[MIDI_BUFFER_SIZE];
static uint8_t midi_tx_data_buf[MIDI_BUFFER_SIZE];

static struct {
	bool    inited;
	bool    last_tx_complete;
	uint8_t last_status;
} state = { false, 0 };

static struct {
	uint16_t tx_waits;
	uint16_t tx_done;
	uint16_t rx_count;
	uint16_t dequeues;
    uint16_t enqueues;
    uint16_t hal_errors;
    HAL_StatusTypeDef last_hal_error;
} stats = { 0, 0, 0, 0, 0, 0};

MIDI_error_t MIDI_Init(UART_HandleTypeDef *in_uart, UART_HandleTypeDef *out_uart)
{
	config.UART_in = in_uart;
	config.UART_out = out_uart;

	// Create ring buffers
	config.midi_rx_ring.data = midi_rx_data_buf;
	config.midi_rx_ring.read_pos = 0;
	config.midi_rx_ring.write_pos = 0;
	config.midi_rx_ring.size = MIDI_BUFFER_SIZE;

	config.midi_tx_ring.data = midi_tx_data_buf;
	config.midi_tx_ring.read_pos = 0;
	config.midi_tx_ring.write_pos = 0;
	config.midi_tx_ring.size = MIDI_BUFFER_SIZE;

	state.last_status = 0;
	state.inited = true;
	state.last_tx_complete = true;
	return MIDI_OK;
}

/*
 * MIDI Transmission APIs
 */
MIDI_error_t MIDI_Send_RawBytes(uint8_t *data, uint16_t num_data_bytes)
{
    HAL_StatusTypeDef tx_status;

	if (num_data_bytes > 0 && (data != NULL)) {
		tx_status = HAL_UART_Transmit_IT(config.UART_out, data, num_data_bytes);
	    if (tx_status != HAL_OK) {
	    	return MIDI_TIMEOUT;
	   	}
	 }
	return MIDI_OK;
}

MIDI_error_t MIDI_Send_RawChannelMsg(uint8_t command,
                          uint8_t channel,
                          uint8_t num_data_bytes,
                          uint8_t *data)
{
    uint8_t first;
    HAL_StatusTypeDef tx_status;

    first = midi_compose_first_byte(channel, command);

    if (first != state.last_status)
    {
    	tx_status = HAL_UART_Transmit_IT(config.UART_out, &first, 1);
    	if (tx_status != HAL_OK) {
    		return MIDI_TIMEOUT;
    	}
    	state.last_status = first;
    }

    if (num_data_bytes > 0 && (data != NULL)) {
    	tx_status = HAL_UART_Transmit_IT(config.UART_out, data, num_data_bytes);
    	if (tx_status != HAL_OK) {
    		return MIDI_TIMEOUT;
    	}
    }
    return MIDI_OK;
}

MIDI_error_t MIDI_Send_NoteOnMsg(uint8_t channel, uint8_t note, uint8_t vel)
{
	uint8_t msg[2];

	msg[0] = note;
	msg[1] = vel;

	return(MIDI_Send_RawChannelMsg(NoteOn, channel, 2, msg));
}

MIDI_error_t MIDI_Send_NoteOffMsg(uint8_t channel, uint8_t note)
{
	uint8_t msg[1];

	msg[0] = note;
	msg[1] = 127;
	return(MIDI_Send_RawChannelMsg(NoteOff, channel, 2, msg));
}

MIDI_error_t MIDI_Send_CCMsg(uint8_t channel, uint8_t control, uint8_t val)
{
	uint8_t msg[2];

	msg[0] = control;
	msg[1] = val;
	return(MIDI_Send_RawChannelMsg(CC, channel, 2, msg));
}

MIDI_error_t MIDI_Send_AllNotesOffMsg(uint8_t channel)
{
	return(MIDI_Send_CCMsg(channel, AllNotesOff, 0));
}

/*
 * MIDI Reception APIs
 */

MIDI_error_t MIDI_Interrupt_Receive_Begin(void)
{
	HAL_StatusTypeDef halStatus = HAL_UART_Receive_IT(config.UART_in, midi_interrupt_rx_buf, 1);
	if (halStatus != HAL_OK) {
		stats.hal_errors++;
		stats.last_hal_error = halStatus;
	}
	return MIDI_OK;
}

MIDI_error_t MIDI_Interrupt_Receive(void)
{
	eCircularBufferError status;

	stats.rx_count++;

	if (state.inited == false) {
		return MIDI_NOT_READY;
	}
	status = circularBuffer_write_bytes(&config.midi_rx_ring, midi_interrupt_rx_buf, 1);
	if (status != eCircularBufferOk) {
		return MIDI_RX_OVERFLOW;
	} else {
		return MIDI_OK;
	}
}

MIDI_error_t MIDI_Dequeue_Receive(uint8_t *bytes, uint16_t *len) {
	eCircularBufferError status;
	if (state.inited == false) {
			return MIDI_NOT_READY;
	}
	status = circularBuffer_read_bytes(&config.midi_rx_ring, bytes, len);
	if (status != eCircularBufferOk) {
		return MIDI_RX_ERROR; // Possibly just nothing left to read.
	}
	stats.dequeues++;

	return MIDI_OK;
}

MIDI_error_t MIDI_Interrupt_Transmit_Begin(void)
{
	eCircularBufferError status;
	HAL_StatusTypeDef halStatus;
	uint16_t read_len = 1;
	uint8_t byte = 0;

	if (state.inited == false) {
			return MIDI_NOT_READY;
	}
	status = circularBuffer_read_bytes(&config.midi_tx_ring, &byte, &read_len);
	if (status != eCircularBufferOk) {
		state.last_tx_complete = true;
		return MIDI_TX_ERROR;
	}
	state.last_tx_complete = false;
	halStatus = HAL_UART_Transmit_IT(config.UART_in, &byte, 1);
	if (halStatus != HAL_OK) {
		stats.hal_errors++;
	}
	return MIDI_OK;
}

MIDI_error_t MIDI_Interrupt_Transmit_End(void)
{
	if (state.inited == false) {
		return MIDI_NOT_READY;
	}
	stats.tx_done++;
	MIDI_Interrupt_Transmit_Begin(); // Start handling any additional bytes that were added during tx
	return MIDI_OK;
}

MIDI_error_t MIDI_Enqueue_Send(uint8_t *bytes, uint16_t *len) {
	eCircularBufferError status;
	if (state.inited == false) {
		return MIDI_NOT_READY;
	}

	status = circularBuffer_write_bytes(&config.midi_tx_ring, bytes, *len);
	if (status != eCircularBufferOk) {
		return MIDI_RX_ERROR;
	}

	stats.enqueues++;

	if (state.last_tx_complete) {
		state.last_tx_complete = false;
		MIDI_Interrupt_Transmit_Begin();
	} else {
		stats.tx_waits++;
	}
	return MIDI_OK;
}

void MIDI_Log_Error(void)
{
	stats.hal_errors++;
}

void MIDI_Print_Stats(void)
{
	printf("rx_count: %d\r\n", stats.rx_count);
	printf("tx_done: %d\r\n", stats.tx_done);
	printf("tx_waits: %d\r\n", stats.tx_waits);
	printf("dequeues: %d\r\n", stats.dequeues);
	printf("enqueues: %d\r\n", stats.enqueues);
	printf("HAL errors: %d\r\n", stats.hal_errors);
	printf("Last HAL error: %d\r\n", stats.last_hal_error);
}
