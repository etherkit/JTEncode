//
// Simple JT65/JT9/WSPR/FSQ beacon for Arduino, with the Etherkit
// Si5351A Breakout Board, by Jason Milldrum NT7S.
//
// Transmit an abritrary message of up to 13 valid characters
// (a Type 6 message) in JT65 and JT9, or a standard Type 1
// message in WSPR.
//
// Connect a momentary push button to pin 12 to use as the
// transmit trigger. Get fancy by adding your own code to trigger
// off of the time from a GPS or your PC via virtual serial.
//
// Original code based on Feld Hell beacon for Arduino by Mark
// Vandewettering K6HX, adapted for the Si5351A by Robert
// Liesenfeld AK6L <ak6l@ak6l.org>.
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject
// to the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
// ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
// CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

#include <si5351.h>
#include <JTEncode.h>
#include <rs_common.h>
#include <int.h>
#include <string.h>

#include "Wire.h"

// Mode defines
#define JT9_TONE_SPACING        174           // ~1.74 Hz
#define JT65_TONE_SPACING       269           // ~2.69 Hz
#define JT4_TONE_SPACING        437           // ~4.37 Hz
#define WSPR_TONE_SPACING       146           // ~1.46 Hz
#define FSQ_TONE_SPACING        879           // ~1.74 Hz

#define JT9_CTC                 9000          // CTC value for JT9-1
#define JT65_CTC                5812          // CTC value for JT65A
#define JT4_CTC                 3578          // CTC value for JT4A
#define WSPR_CTC                10672         // CTC value for WSPR
#define FSQ_2_CTC               7812          // CTC value for 2 baud FSQ
#define FSQ_3_CTC               5208          // CTC value for 3 baud FSQ
#define FSQ_4_5_CTC             3472          // CTC value for 4.5 baud FSQ
#define FSQ_6_CTC               2604          // CTC value for 6 baud FSQ

#define JT9_DEFAULT_FREQ        14080800UL
#define JT65_DEFAULT_FREQ       14078500UL
#define JT4_DEFAULT_FREQ        14078500UL
#define WSPR_DEFAULT_FREQ       14097200UL
#define FSQ_DEFAULT_FREQ        7105350UL     // Base freq is 1350 Hz higher than dial freq in USB

#define DEFAULT_MODE            MODE_JT65

// Hardware defines
#define BUTTON                  12
#define LED_PIN                 13

// Enumerations
enum mode {MODE_JT9, MODE_JT65, MODE_JT4, MODE_WSPR, MODE_FSQ_2, MODE_FSQ_3,
  MODE_FSQ_4_5, MODE_FSQ_6};

// Class instantiation
Si5351 si5351;
JTEncode jtencode;

// Global variables
unsigned long freq;
char message[] = "N0CALL AA00";
char call[] = "N0CALL";
char loc[] = "AA00";
uint8_t dbm = 27;
uint8_t tx_buffer[255];
enum mode cur_mode = DEFAULT_MODE;
uint8_t symbol_count;
uint16_t ctc, tone_spacing;

// Global variables used in ISRs
volatile bool proceed = false;

// Timer interrupt vector.  This toggles the variable we use to gate
// each column of output to ensure accurate timing.  Called whenever
// Timer1 hits the count set below in setup().
ISR(TIMER1_COMPA_vect)
{
    proceed = true;
}

// Loop through the string, transmitting one character at a time.
void encode()
{
  uint8_t i;

  // Clear out the old transmit buffer
  memset(tx_buffer, 0, 255);

  // Set the proper frequency and timer CTC depending on mode
  switch(cur_mode)
  {
  case MODE_JT9:
    jtencode.jt9_encode(message, tx_buffer);
    break;
  case MODE_JT65:
    jtencode.jt65_encode(message, tx_buffer);
    break;
  case MODE_JT4:
    jtencode.jt4_encode(message, tx_buffer);
    break;
  case MODE_WSPR:
    jtencode.wspr_encode(call, loc, dbm, tx_buffer);
    break;
  case MODE_FSQ_2:
  case MODE_FSQ_3:
  case MODE_FSQ_4_5:
  case MODE_FSQ_6:
    jtencode.fsq_dir_encode(call, "n0call", ' ', "hello world", tx_buffer);
    break;
  }

  // Reset the tone to the base frequency and turn on the output
  si5351.output_enable(SI5351_CLK0, 1);
  digitalWrite(LED_PIN, HIGH);

  // Now transmit the channel symbols
  if(cur_mode == MODE_FSQ_2 || cur_mode == MODE_FSQ_3 || cur_mode == MODE_FSQ_4_5 || cur_mode == MODE_FSQ_6)
  {
    uint8_t j = 0;

    while(tx_buffer[j++] != 0xff)
    {
    }

    symbol_count = j - 1;
  }

  for(i = 0; i < symbol_count; i++)
  {
      si5351.set_freq((freq * 100) + (tx_buffer[i] * tone_spacing), SI5351_CLK0);
      proceed = false;
      while(!proceed);
  }

  // Turn off the output
  si5351.output_enable(SI5351_CLK0, 0);
  digitalWrite(LED_PIN, LOW);
}


void setup()
{
  // Use the Arduino's on-board LED as a keying indicator.
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Use a button connected to pin 12 as a transmit trigger
  pinMode(BUTTON, INPUT_PULLUP);

  // Set the mode to use
  cur_mode = MODE_WSPR;

  // Set the proper frequency, tone spacing, symbol count, and
  // timer CTC depending on mode
  switch(cur_mode)
  {
  case MODE_JT9:
    freq = JT9_DEFAULT_FREQ;
    ctc = JT9_CTC;
    symbol_count = JT9_SYMBOL_COUNT; // From the library defines
    tone_spacing = JT9_TONE_SPACING;
    break;
  case MODE_JT65:
    freq = JT65_DEFAULT_FREQ;
    ctc = JT65_CTC;
    symbol_count = JT65_SYMBOL_COUNT; // From the library defines
    tone_spacing = JT65_TONE_SPACING;
    break;
  case MODE_JT4:
    freq = JT4_DEFAULT_FREQ;
    ctc = JT4_CTC;
    symbol_count = JT4_SYMBOL_COUNT; // From the library defines
    tone_spacing = JT4_TONE_SPACING;
    break;
  case MODE_WSPR:
    freq = WSPR_DEFAULT_FREQ;
    ctc = WSPR_CTC;
    symbol_count = WSPR_SYMBOL_COUNT; // From the library defines
    tone_spacing = WSPR_TONE_SPACING;
    break;
  case MODE_FSQ_2:
    freq = FSQ_DEFAULT_FREQ;
    ctc = FSQ_2_CTC;
    tone_spacing = FSQ_TONE_SPACING;
    break;
  case MODE_FSQ_3:
    freq = FSQ_DEFAULT_FREQ;
    ctc = FSQ_3_CTC;
    tone_spacing = FSQ_TONE_SPACING;
    break;
  case MODE_FSQ_4_5:
    freq = FSQ_DEFAULT_FREQ;
    ctc = FSQ_4_5_CTC;
    tone_spacing = FSQ_TONE_SPACING;
    break;
  case MODE_FSQ_6:
    freq = FSQ_DEFAULT_FREQ;
    ctc = FSQ_6_CTC;
    tone_spacing = FSQ_TONE_SPACING;
    break;
  }

  // Initialize the Si5351
  // Change the 2nd parameter in init if using a ref osc other
  // than 25 MHz
  si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0);

  // Set CLK0 output
  si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA); // Set for max power if desired
  si5351.output_enable(SI5351_CLK0, 0); // Disable the clock initially

  // Set up Timer1 for interrupts every symbol period.
  noInterrupts();          // Turn off interrupts.
  TCCR1A = 0;              // Set entire TCCR1A register to 0; disconnects
                           //   interrupt output pins, sets normal waveform
                           //   mode.  We're just using Timer1 as a counter.
  TCNT1  = 0;              // Initialize counter value to 0.
  TCCR1B = (1 << CS12) |   // Set CS12 and CS10 bit to set prescale
    (1 << CS10) |          //   to /1024
    (1 << WGM12);          //   turn on CTC
                           //   which gives, 64 us ticks
  TIMSK1 = (1 << OCIE1A);  // Enable timer compare interrupt.
  OCR1A = ctc;             // Set up interrupt trigger count;
  interrupts();            // Re-enable interrupts.
}

void loop()
{
  // Debounce the button and trigger TX on push
  if(digitalRead(BUTTON) == LOW)
  {
    delay(50);   // delay to debounce
    if (digitalRead(BUTTON) == LOW)
    {
      encode();

      delay(50); //delay to avoid extra triggers
    }
  }
}
