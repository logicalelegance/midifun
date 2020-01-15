/*
 * midi.c
 *
 * MIDI Driver for the STM32 platform, based on HAL.
 *
 * cwhite@logicalelegance.com
 */

#include <stm32f3xx_hal.h>
#include "midi.h"

static struct {
	UART_HandleTypeDef *UART_in; // UART associated with the MIDI IN port
	UART_HandleTypeDef *UART_out; // UART associated with the MIDI OUT port
} config;

static struct {
	uint8_t last_status;
} state;

MIDI_error_t MIDI_Init(UART_HandleTypeDef *in_uart, UART_HandleTypeDef *out_uart)
{
	config.UART_in = in_uart;
	config.UART_out = out_uart;

	state.last_status = 0;
	return MIDI_OK;
}

MIDI_error_t MIDI_Send_RawChannelMsg(uint8_t command,
                          uint8_t channel,
                          uint8_t num_data_bytes,
                          uint8_t * data)
{
    uint8_t first;
    HAL_StatusTypeDef tx_status;

    first = midi_compose_first_byte(channel, command);

    if (first != state.last_status)
    {
    	tx_status = HAL_UART_Transmit(config.UART_out, &first, 1, 10);
    	if (tx_status != HAL_OK) {
    		return MIDI_TIMEOUT;
    	}
    	state.last_status = first;
    }

    if (num_data_bytes > 0 && (data != NULL)) {
    	tx_status = HAL_UART_Transmit(config.UART_out, data, num_data_bytes, 10);
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

