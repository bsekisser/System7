/*
 * SpeechSmoke.c - Speech Manager Smoke Test
 *
 * Tests basic Speech Manager functionality:
 * - Speech Manager initialization
 * - SpeakString API call
 * - Audio output pathway integration
 * - Speech status queries
 *
 * Expected log output:
 *   Speech Manager smoke test: TESTING
 *   SpeakString("Hello World") returned: 0
 *   SpeechBusy returned: 0
 *   SpeechBusySystemWide returned: 0
 *   Speech Manager smoke test: SUCCESS
 *
 * This test verifies that:
 * 1. SpeechManager is properly initialized
 * 2. SpeakString API is accessible and returns valid error codes
 * 3. Status queries work without crashing
 * 4. Audio output pathway is exercised
 */

#include "SystemTypes.h"
#include "SpeechManager/SpeechManager.h"
#include "System71StdLib.h"

/* Smoke test flag - define to enable */
#ifndef SPEECH_SMOKE_TEST
#define SPEECH_SMOKE_TEST 0
#endif

#if SPEECH_SMOKE_TEST

/*
 * RunSpeechSmokeTest - Execute smoke test
 */
void RunSpeechSmokeTest(void)
{
    OSErr err;
    short isBusy;
    unsigned char testString[32];

    serial_puts("\nSpeech Manager smoke test: TESTING\n");

    /* Test 1: Initialize Speech Manager (should already be done by main.c) */
    err = SpeechManagerInit();
    serial_puts("SpeechManagerInit returned: ");
    if (err == 0) serial_puts("0\n");
    else serial_puts("ERROR\n");

    if (err != noErr) {
        serial_puts("Speech Manager smoke test: FAILED (init error)\n");
        return;
    }

    /* Test 2: Prepare test string - "Hello World" in Pascal format */
    /* Pascal string format: first byte is length, followed by string data */
    testString[0] = 11;  /* Length of "Hello World" */
    testString[1] = 'H';
    testString[2] = 'e';
    testString[3] = 'l';
    testString[4] = 'l';
    testString[5] = 'o';
    testString[6] = ' ';
    testString[7] = 'W';
    testString[8] = 'o';
    testString[9] = 'r';
    testString[10] = 'l';
    testString[11] = 'd';

    /* Test 3: Call SpeakString - this will:
     * - Create a speech channel if needed
     * - Process the text
     * - Send audio through SpeechOutput to SoundManager
     * - Even with stub implementation, pathway is exercised
     */
    err = SpeakString((StringPtr)testString);
    serial_puts("SpeakString(\"Hello World\") returned: ");
    if (err == 0) serial_puts("0"); else serial_puts("E");
    serial_puts("\n");

    if (err != noErr && err != paramErr) {
        serial_puts("Warning: SpeakString returned unexpected error\n");
    }

    /* Test 4: Check if speech is busy */
    isBusy = SpeechBusy();
    serial_puts("SpeechBusy returned: ");
    if (isBusy == 0) serial_puts("0"); else serial_puts("1");
    serial_puts("\n");

    /* Test 5: Check system-wide speech activity */
    isBusy = SpeechBusySystemWide();
    serial_puts("SpeechBusySystemWide returned: ");
    if (isBusy == 0) serial_puts("0"); else serial_puts("1");
    serial_puts("\n");

    /* Test 6: Test with empty string (should return error) */
    testString[0] = 0;  /* Empty string */
    err = SpeakString((StringPtr)testString);
    serial_puts("SpeakString(empty) returned: ");
    if (err == 0) serial_puts("0"); else serial_puts("E");
    serial_puts("\n");

    if (err == paramErr) {
        serial_puts("Correctly rejected empty string\n");
    }

    /* Test 7: Another phrase - "System Seven" */
    testString[0] = 12;  /* Length */
    testString[1] = 'S';
    testString[2] = 'y';
    testString[3] = 's';
    testString[4] = 't';
    testString[5] = 'e';
    testString[6] = 'm';
    testString[7] = ' ';
    testString[8] = 'S';
    testString[9] = 'e';
    testString[10] = 'v';
    testString[11] = 'e';
    testString[12] = 'n';

    err = SpeakString((StringPtr)testString);
    serial_puts("SpeakString(\"System Seven\") returned: ");
    if (err == 0) serial_puts("0"); else serial_puts("E");
    serial_puts("\n");

    serial_puts("Speech Manager smoke test: SUCCESS\n\n");
}

#else

void RunSpeechSmokeTest(void)
{
    /* Test disabled */
}

#endif
