/*
 * midi_application.c
 *
 * Main MIDI application.
 *
 * 01/21/2020
 *
 * Chris White
 */

//#define DEBUG_MIDI_TX
#include "midi_application.h"

void MIDI_Application_Process(void)
{
	MIDI_error_t status = MIDI_OK;
	uint8_t next_byte = 0;
	uint16_t bytes_to_read = 1;

	// Just emulate MIDI through for now
	do {
		if (!MIDI_Interrupt_Is_Armed()) {
			MIDI_Interrupt_Receive_Begin();
		}

		status = MIDI_Dequeue_Receive(&next_byte, &bytes_to_read);
		if (status == MIDI_OK) {
			status = MIDI_Enqueue_Send(&next_byte, &bytes_to_read);
#ifdef DEBUG_MIDI_TX
			printf("Sent: %x\r\n", next_byte);
#endif
		}
	} while (status == MIDI_OK);
}
