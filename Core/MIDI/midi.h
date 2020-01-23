/*
 * midi.h
 *
 * MIDI Driver for the STM32 platform, based on HAL.
 *
 * cwhite@logicalelegance.com
 */

#define MIDI_BUFFER_SIZE 1024

typedef enum {
	MIDI_OK,
	MIDI_NOT_READY,
	MIDI_TIMEOUT,
	MIDI_RX_OVERFLOW,
	MIDI_RX_ERROR,
	MIDI_TX_ERROR,
	MIDI_INVALID_PARAM,
} MIDI_error_t;

typedef enum {
	NoteOff = 0x80,
	NoteOn = 0x90,
	CC = 0xB0,
} midi_command_e;

typedef enum {
	AllNotesOff = 0x7B,
} midi_cc_e;

static inline uint8_t midi_compose_first_byte(uint8_t channel, uint8_t command) {
	return((command & 0xf0) | ((channel - 1) & 0x0f));
}

MIDI_error_t MIDI_Init(UART_HandleTypeDef *in_uart, UART_HandleTypeDef *out_uart);
MIDI_error_t MIDI_Send_RawBytes(uint8_t *data, uint16_t num_data_bytes);
MIDI_error_t MIDI_Send_RawChannelMsg(uint8_t command,
                          uint8_t channel,
                          uint8_t num_data_bytes,
                          uint8_t * data);
MIDI_error_t MIDI_Send_NoteOnMsg(uint8_t channel, uint8_t note, uint8_t vel);
MIDI_error_t MIDI_Send_NoteOffMsg(uint8_t channel, uint8_t note);
MIDI_error_t MIDI_Send_CCMsg(uint8_t channel, uint8_t control, uint8_t val);
MIDI_error_t MIDI_Send_AllNotesOffMsg(uint8_t channel);

MIDI_error_t MIDI_Interrupt_Receive(void);
MIDI_error_t MIDI_Dequeue_Receive(uint8_t *bytes, uint16_t *len);
MIDI_error_t MIDI_Interrupt_Receive_Begin(void);

MIDI_error_t MIDI_Enqueue_Send(uint8_t *bytes, uint16_t *len);
MIDI_error_t MIDI_Interrupt_Transmit_Begin(void);







