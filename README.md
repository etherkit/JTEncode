JT65/JT9/JT4/WSPR/FSQ Encoder Library for Arduino
=============================================
This library very simply generates a set of channel symbols for JT65, JT9, JT4, or WSPR based on the user providing a properly formatted Type 6 message for JT65, JT9, or JT4 (which is 13 valid characters) or a callsign, Maidenhead grid locator, and power output for WSPR. It will also generate an arbitrary FSQ message of up to 200 characters in both directed and non-directed format. When paired with a synthesizer that can output frequencies in fine, phase-continuous tuning steps (such as the Si5351), then a beacon or telemetry transmitter can be created which can change the transmitted characters as needed from the Arduino.

Please feel free to use the issues feature of GitHub if you run into problems or have suggestions for important features to implement.

Hardware Requirements and Setup
-------------------------------
This library has been written for the Arduino platform and has been successfully tested on the Arduino Uno and an Uno clone. Since the library itself does not access the hardware, there is no reason it should not run on any Arduino model of recent vintage.

How To Install
--------------
The best way to install the library is via the Arduino Library Manager, which is available if you are using Arduino IDE version 1.6.2 or greater. To install it this way, simply go to the menu Sketch > Include Library > Manage Libraries..., and then in the search box at the upper-right, type "Etherkit JTEncode". Click on the entry in the list below, then click on the provided "Install" button. By installing the library this way, you will always have notifications of future library updates, and can easily switch between library versions.

If you need to or would like to install the library in the old way, then you can download a copy of the library in a ZIP file. Download a ZIP file of the library from the GitHub repository by going to [this page](https://github.com/etherkit/JTEncode/releases) and clicking the "Source code (zip)" link under the latest release. Finally, open the Arduino IDE, select menu Sketch > Import Library... > Add Library..., and select the ZIP that you just downloaded.

Example
-------
There is a simple example that is placed in your examples menu under JTEncode. Open this to see how to incorporate this library with your code. The example provided with with the library is meant to be used in conjuction with the [Etherkit Si5351A Breakout Board](https://www.etherkit.com/rf-modules/si5351a-breakout-board.html), although it could be modified to use with other synthesizers which meet the technical requirements of the JT65/JT9/JT4/WSPR/FSQ modes.

To run this example, be sure to download the [Si5351Arduino](https://github.com/etherkit/Si5351Arduino) library and follow the instructions there to connect the Si5351A Breakout Board to your Arduino. In order to trigger transmissions, you will also need to connect a momentary pushbutton from pin 12 of the Arduino to ground.

The example sketch itself is fairly straightforward. JT65, JT9, JT4, WSPR, and FSQ modes are modulated in same way: phase-continuous multiple-frequency shift keying (MFSK). The message to be transmitted is passed to the JTEncode method corresponding to the desired mode, along with a pointer to an array which holds the returned channel symbols. When the pushbutton is pushed, the sketch then transmits each channel symbol sequentially as an offset from the base frequency given in the sketch define section.

An instance of the JTEncode object is created:

    JTEncode jtencode;

On sketch startup, the mode parameters are set based on which mode is currently selected (by the DEFAULT_MODE define):

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

Note that the number of channel symbols for each mode is defined in the library, so you can use those defines to initialize your own symbol array sizes.

During transmit, the proper class method is chosen based on the desired mode, then the transmit symbol buffer and the other mode information is set:

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
      call.toUpperCase();
      jtencode.wspr_encode(call, loc, dbm, tx_buffer);
      break;
    case MODE_FSQ_2:
    case MODE_FSQ_3:
    case MODE_FSQ_4_5:
    case MODE_FSQ_6:
      call.toLowerCase();
      jtencode.fsq_dir_encode(call, "n0call", " ", "hello world", tx_buffer);
      break;
    }

Once the channel symbols have been generated, it is a simple matter of transmitting them in sequence, each the correct amount of time:

    // Now transmit the channel symbols
    for(i = 0; i < symbol_count; i++)
    {
        si5351.set_freq((freq * 100) + (tx_buffer[i] * tone_spacing), 0, SI5351_CLK0);
        proceed = false;
        while(!proceed);
    }

Public Methods
------------------
### jt65_encode()
```
/*
 * jt65_encode(char * message, uint8_t * symbols)
 *
 * Takes an arbitrary message of up to 13 allowable characters and returns
 * a channel symbol table.
 *
 * message - Plaintext Type 6 message.
 * symbols - Array of channel symbols to transmit retunred by the method.
 *  Ensure that you pass a uint8_t array of size JT65_SYMBOL_COUNT to the method.
 *
 */
```
### jt9_encode()
```
/*
 * jt9_encode(char * message, uint8_t * symbols)
 *
 * Takes an arbitrary message of up to 13 allowable characters and returns
 * a channel symbol table.
 *
 * message - Plaintext Type 6 message.
 * symbols - Array of channel symbols to transmit retunred by the method.
 *  Ensure that you pass a uint8_t array of size JT9_SYMBOL_COUNT to the method.
 *
 */
```

### jt4_encode()
```
/*
 * jt4_encode(char * message, uint8_t * symbols)
 *
 * Takes an arbitrary message of up to 13 allowable characters and returns
 * a channel symbol table.
 *
 * message - Plaintext Type 6 message.
 * symbols - Array of channel symbols to transmit retunred by the method.
 *  Ensure that you pass a uint8_t array of size JT9_SYMBOL_COUNT to the method.
 *
 */
 ```

### wspr_encode()
```
/*
 * wspr_encode(char * call, char * loc, uint8_t dbm, uint8_t * symbols)
 *
 * Takes an arbitrary message of up to 13 allowable characters and returns
 *
 * call - Callsign (6 characters maximum).
 * loc - Maidenhead grid locator (4 charcters maximum).
 * dbm - Output power in dBm.
 * symbols - Array of channel symbols to transmit retunred by the method.
 *  Ensure that you pass a uint8_t array of size WSPR_SYMBOL_COUNT to the method.
 *
 */
```

### fsq_encode()
```
/*
 * fsq_encode(char * from_call, char * message, uint8_t * symbols)
 *
 * Takes an arbitrary message and returns a FSQ channel symbol table.
 *
 * from_call - Callsign of issuing station (maximum size: 20)
 * message - Null-terminated message string, no greater than 130 chars in length
 * symbols - Array of channel symbols to transmit retunred by the method.
 *  Ensure that you pass a uint8_t array of at least the size of the message
 *  plus 5 characters to the method. Terminated in 0xFF.
 *
 */
```

### fsq_dir_encode()
 ```
/*
* fsq_dir_encode(char * from_call, char * to_call, char cmd, char * message, uint8_t * symbols)
*
* Takes an arbitrary message and returns a FSQ channel symbol table.
*
* from_call - Callsign from which message is directed (maximum size: 20)
* to_call - Callsign to which message is directed (maximum size: 20)
* cmd - Directed command
* message - Null-terminated message string, no greater than 100 chars in length
* symbols - Array of channel symbols to transmit retunred by the method.
*  Ensure that you pass a uint8_t array of at least the size of the message
*  plus 5 characters to the method. Terminated in 0xFF.
*
*/
```

Tokens
------
Here are the defines, structs, and enumerations you will find handy to use with the library.

Defines:

    JT65_SYMBOL_COUNT, JT9_SYMBOL_COUNT, JT4_SYMBOL_COUNT, WSPR_SYMBOL_COUNT

Acknowledgements
----------------
Many thanks to Joe Taylor K1JT for his innovative work in amateur radio. We are lucky to have him. The algorithms in this program were derived from the source code in the [WSJT](http://sourceforge.net/projects/wsjt/) suite of applications. Also, many thanks for Andy Talbot G4JNT for [his paper](http://www.g4jnt.com/JTModesBcns.htm) on the WSPR coding protocol, which helped me to understand the WSPR encoding process, which in turn helped me to understand the related JT protocols.

Also, a big thank you to Murray Greenman, ZL1BPU for working allowing me to pick his brain regarding his neat new mode FSQ.

Changelog
---------
* v1.1.1

    * Update example sketch for Si5351Arduino v2.0.0

* v1.1.0

    * Added FSQ.

* v1.0.1

    * Fixed a bug in jt65_interleave that was causing a buffer overrun.

* v1.0.0

    * Initial Release.

License
-------
JTEncode is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

JTEncode is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with JTEncode.  If not, see <http://www.gnu.org/licenses/>.
