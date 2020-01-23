// ConsoleCommands.c
// This is where you add commands:
//		1. Add a protoype
//			static eCommandResult_T ConsoleCommandVer(const char buffer[]);
//		2. Add the command to mConsoleCommandTable
//		    {"ver", &ConsoleCommandVer, HELP("Get the version string")},
//		3. Implement the function, using ConsoleReceiveParam<Type> to get the parameters from the buffer.

#include <string.h>
#include "consoleCommands.h"
#include "console.h"
#include "consoleIo.h"
#include "version.h"
#include "../midi/midi.h"

#define IGNORE_UNUSED_VARIABLE(x)     if ( &x == &x ) {}

static eCommandResult_T ConsoleCommandComment(const char buffer[]);
static eCommandResult_T ConsoleCommandVer(const char buffer[]);
static eCommandResult_T ConsoleCommandHelp(const char buffer[]);
static eCommandResult_T ConsoleCommandMidiNoteOn(const char buffer[]);
static eCommandResult_T ConsoleCommandMidiAllNotesOff(const char buffer[]);
static eCommandResult_T ConsoleCommandMidiTestSequence(const char buffer[]);
static eCommandResult_T ConsoleCommandMidiStats(const char buffer[]);

static const sConsoleCommandTable_T mConsoleCommandTable[] = {
		{ ";", &ConsoleCommandComment, HELP(
				"Comment! You do need a space after the semicolon. ") },
		{ "help", &ConsoleCommandHelp, HELP("Lists the commands available") },
		{ "ver", &ConsoleCommandVer, HELP("Get the version string") },
		{ "MidiNoteOn", &ConsoleCommandMidiNoteOn, HELP("Play note with value") },
		{ "MidiAllNotesOff", &ConsoleCommandMidiAllNotesOff, HELP("Turn off all notes") },
		{ "MidiTestSeq", &ConsoleCommandMidiTestSequence, HELP("Play a test sequence of notes") },
		{ "midistats", &ConsoleCommandMidiStats, HELP("Get MIDI tx/rx stats") },
		CONSOLE_COMMAND_TABLE_END // must be LAST
		};

static eCommandResult_T ConsoleCommandComment(const char buffer[]) {
	// do nothing
	IGNORE_UNUSED_VARIABLE(buffer);
	return COMMAND_SUCCESS;
}

static eCommandResult_T ConsoleCommandHelp(const char buffer[]) {
	uint32_t i;
	uint32_t tableLength;
	eCommandResult_T result = COMMAND_SUCCESS;

	IGNORE_UNUSED_VARIABLE(buffer);

	tableLength = sizeof(mConsoleCommandTable)
			/ sizeof(mConsoleCommandTable[0]);
	for (i = 0u; i < tableLength - 1u; i++) {
		ConsoleIoSendString(mConsoleCommandTable[i].name);
#if CONSOLE_COMMAND_MAX_HELP_LENGTH > 0
		ConsoleIoSendString(" : ");
		ConsoleIoSendString(mConsoleCommandTable[i].help);
#endif // CONSOLE_COMMAND_MAX_HELP_LENGTH > 0
		ConsoleIoSendString(STR_ENDLINE);
	}
	return result;
}

static eCommandResult_T ConsoleCommandMidiStats(const char buffer[]) {
	MIDI_Print_Stats();
}

static eCommandResult_T ConsoleCommandMidiNoteOn(const char buffer[]) {
	uint16_t noteVal;
	eCommandResult_T result;

	result = ConsoleReceiveParamHexUint16(buffer, 1, &noteVal);
	if (COMMAND_SUCCESS == result) {
		MIDI_Send_NoteOnMsg(1, noteVal, 127);
	}
	return result;
}

static eCommandResult_T ConsoleCommandMidiAllNotesOff(const char buffer[]) {
	uint16_t noteVal;
	eCommandResult_T result;

	MIDI_Send_AllNotesOffMsg(1);
	return COMMAND_SUCCESS;
}

static eCommandResult_T ConsoleCommandMidiTestSequence(const char buffer[]) {
	uint16_t noteVals[6];
	uint16_t runCount = 10;
	eCommandResult_T result;

	result = ConsoleReceiveParamInt16(buffer, 1, &noteVals[0]);
	result |= ConsoleReceiveParamInt16(buffer, 2, &noteVals[1]);
	result |= ConsoleReceiveParamInt16(buffer, 3, &noteVals[2]);
	result |= ConsoleReceiveParamInt16(buffer, 4, &noteVals[3]);
	result |= ConsoleReceiveParamInt16(buffer, 5, &noteVals[4]);
	result |= ConsoleReceiveParamInt16(buffer, 6, &noteVals[5]);

	if (COMMAND_SUCCESS == result) {
		uint8_t note_idx = 0;
		while (runCount--) {
			note_idx = 0;
			while (note_idx < 6) {
				MIDI_Send_NoteOnMsg(1, noteVals[note_idx], 127);
				HAL_Delay(300);
				MIDI_Send_NoteOffMsg(1, noteVals[note_idx]);

				note_idx++;
			}
		}
		MIDI_Send_AllNotesOffMsg(1);
	}
	return result;
}


static eCommandResult_T ConsoleCommandVer(const char buffer[]) {
	eCommandResult_T result = COMMAND_SUCCESS;

	IGNORE_UNUSED_VARIABLE(buffer);

	ConsoleIoSendString(VERSION_STRING);
	ConsoleIoSendString(STR_ENDLINE);
	return result;
}

const sConsoleCommandTable_T* ConsoleCommandsGetTable(void) {
	return (mConsoleCommandTable);
}

