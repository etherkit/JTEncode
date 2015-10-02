JT65/JT9/WSPR Encoder Library for Arduino
=========================================
This library very simply generates a set of channel symbols for JT65, JT9, or WSPR based on the user providing a properly formatted Type 6 message for JT65 or JT9 (which is 13 valid characters) or a callsign, Maidenhead grid locator, and power output for WSPR. When paired with a synthesizer that can output frequencies in fine, phase-continuous tuning steps (such as the Si5351), then a beacon or telemetry transmitter can be created which can change the transmitted characters as needed from the Arduino.

Please feel free to use the issues feature of GitHub if you run into problems or have suggestions for important features to implement.

Hardware Requirements and Setup
-------------------------------
This library has been written for the Arduino platform and has been successfully tested on the Arduino Uno and an Uno clone. Since the library itself does not access the hardware, there is no reason it should not run on any Arduino model of recent vintage.

How To Install
--------------
Include the JTEncode library into your instance of the Arduino IDE. Download a ZIP file of the library from the GitHub repository by using the "Download ZIP" button at the right of the main repository page. Extract the ZIP file, then rename the unzipped folder as "JTEncode". Finally, open the Arduino IDE, select menu Sketch > Import Library... > Add Library..., and select the renamed folder that you just downloaded. Restart the IDE and you should have access to the new library.

(Yes, the Arduino IDE can import a ZIP file, but it doesn't like filenames with characters such as dashes, as GitHub does when it appends the branch name with a dash. Perhaps there's an elegant way around this, we'll see.)

Example
-------
There is a simple example that is placed in your examples menu under JTEncode. Open this to see how to incorporate this library with your code. The example provided with with the library is meant to be used in conjuction with the [Etherkit Si5351A Breakout Board](https://www.etherkit.com/rf-modules/si5351a-breakout-board.html), although it could be modified to use with other synthesizers which meet the technical requirements of the JT65/JT9/WSPR modes.

To run this example, be sure to download the [Si5351Arduino](https://github.com/etherkit/Si5351Arduino) library and follow the instructions there to connect the Si5351A Breakout Board to your Arduino. In order to trigger transmissions, you will also need to connect a momentary pushbutton from pin 12 of the Arduino to ground.

The example sketch itself is fairly straightforward. JT65, JT9, and WSPR modes are modulated in same way: phase-continuous multiple-frequency shift keying (MFSK). The message to be transmitted is passed to the JTEncode method corresponding to the desired mode, along with a pointer to an array which holds the returned channel symbols. When the pushbutton is pushed, the sketch then transmits each channel symbol sequentially as an offset from the base frequency given in the sketch define section.

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
    case MODE_WSPR:
      freq = WSPR_DEFAULT_FREQ;
      ctc = WSPR_CTC;
      symbol_count = WSPR_SYMBOL_COUNT; // From the library defines
      tone_spacing = WSPR_TONE_SPACING;
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
    case MODE_WSPR:
      jtencode.wspr_encode(call, loc, dbm, tx_buffer);
      break;
    }

Once the channel symbols have been generate, it is a simple matter of transmitting them in sequence, each of the correct amount of time:

    // Now transmit the channel symbols
    for(i = 0; i < symbol_count; i++)
    {
        si5351.set_freq((freq * 100) + (tx_buffer[i] * tone_spacing), 0, SI5351_CLK0);
        proceed = false;
        while(!proceed);
    }

Public Methods
------------------
###jt65_encode()
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
###jt9_encode()
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

###wspr_encode()
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
Tokens
------
Here are the defines, structs, and enumerations you will find handy to use with the library.

Defines:

    JT65_SYMBOL_COUNT, JT9_SYMBOL_COUNT, WSPR_SYMBOL_COUNT

Acknowledgements
----------------
Many thanks to Joe Taylor K1JT for his innovative work in amateur radio. We are lucky to have him. The algorithms in this program were derived from the source code in the [WSJT](http://sourceforge.net/projects/wsjt/) suite of applications. Also, many thanks for Andy Talbot G4JNT for [his paper](http://www.g4jnt.com/JTModesBcns.htm) on the WSPR coding protocol, which helped me to understand the WSPR encoding process, which in turn helped me to understand the related JT protocols.
