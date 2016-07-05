/*
 * JTEncode.h - JT65/JT9/WSPR encoder library for Arduino
 *
 * Copyright (C) 2015 Jason Milldrum <milldrum@gmail.com>
 *
 * Based on the algorithms presented in the WSJT software suite.
 * Thanks to Andy Talbot G4JNT for the whitepaper on the WSPR encoding
 * process that helped me to understand all of this.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "int.h"
#include "rs_common.h"

#include <stdint.h>
#include <avr/pgmspace.h>

#define JT65_SYMBOL_COUNT                   126
#define JT9_SYMBOL_COUNT                    85
#define JT4_SYMBOL_COUNT                    207
#define WSPR_SYMBOL_COUNT                   162

#define JT65_ENCODE_COUNT                   63
#define JT9_ENCODE_COUNT                    69

#define JT9_BIT_COUNT                       206
#define JT4_BIT_COUNT                       206
#define WSPR_BIT_COUNT                      162

// Define the structure of a varicode table
typedef struct fsq_varicode
{
    uint8_t ch;
    uint8_t var[2];
} Varicode;

// The FSQ varicode table, based on the FSQ Varicode V3.0
// document provided by Murray Greenman, ZL1BPU

const Varicode fsq_code_table[] PROGMEM =
{
  {' ', {00, 00}}, // space
  {'!', {11, 30}},
  {'"', {12, 30}},
  {'#', {13, 30}},
  {'$', {14, 30}},
  {'%', {15, 30}},
  {'&', {16, 30}},
  {'\'', {17, 30}},
  {'(', {18, 30}},
  {')', {19, 30}},
  {'*', {20, 30}},
  {'+', {21, 30}},
  {',', {27, 29}},
  {'-', {22, 30}},
  {'.', {27, 00}},
  {'/', {23, 30}},
  {'0', {10, 30}},
  {'1', {01, 30}},
  {'2', {02, 30}},
  {'3', {03, 30}},
  {'4', {04, 30}},
  {'5', {05, 30}},
  {'6', {06, 30}},
  {'7', {07, 30}},
  {'8', {8, 30}},
  {'9', {9, 30}},
  {':', {24, 30}},
  {';', {25, 30}},
  {'<', {26, 30}},
  {'=', {00, 31}},
  {'>', {27, 30}},
  {'?', {28, 29}},
  {'@', {00, 29}},
  {'A', {01, 29}},
  {'B', {02, 29}},
  {'C', {03, 29}},
  {'D', {04, 29}},
  {'E', {05, 29}},
  {'F', {06, 29}},
  {'G', {07, 29}},
  {'H', {8, 29}},
  {'I', {9, 29}},
  {'J', {10, 29}},
  {'K', {11, 29}},
  {'L', {12, 29}},
  {'M', {13, 29}},
  {'N', {14, 29}},
  {'O', {15, 29}},
  {'P', {16, 29}},
  {'Q', {17, 29}},
  {'R', {18, 29}},
  {'S', {19, 29}},
  {'T', {20, 29}},
  {'U', {21, 29}},
  {'V', {22, 29}},
  {'W', {23, 29}},
  {'X', {24, 29}},
  {'Y', {25, 29}},
  {'Z', {26, 29}},
  {'[', {01, 31}},
  {'\\', {02, 31}},
  {']', {03, 31}},
  {'^', {04, 31}},
  {'_', {05, 31}},
  {'`', {9, 31}},
  {'a', {01, 00}},
  {'b', {02, 00}},
  {'c', {03, 00}},
  {'d', {04, 00}},
  {'e', {05, 00}},
  {'f', {06, 00}},
  {'g', {07, 00}},
  {'h', {8, 00}},
  {'i', {9, 00}},
  {'j', {10, 00}},
  {'k', {11, 00}},
  {'l', {12, 00}},
  {'m', {13, 00}},
  {'n', {14, 00}},
  {'o', {15, 00}},
  {'p', {16, 00}},
  {'q', {17, 00}},
  {'r', {18, 00}},
  {'s', {19, 00}},
  {'t', {20, 00}},
  {'u', {21, 00}},
  {'v', {22, 00}},
  {'w', {23, 00}},
  {'x', {24, 00}},
  {'y', {25, 00}},
  {'z', {26, 00}},
  {'{', {06, 31}},
  {'|', {07, 31}},
  {'}', {8, 31}},
  {'~', {00, 30}},
  {127, {28, 31}}, // DEL
  {13,  {28, 00}}, // CR
  {10,  {28, 00}}, // LF
  {0,   {28, 30}}, // IDLE
  {241, {10, 31}}, // plus/minus
  {246, {11, 31}}, // division sign
  {248, {12, 31}}, // degrees sign
  {158, {13, 31}}, // multiply sign
  {156, {14, 31}}, // pound sterling sign
  {8,   {27, 31}}  // BS
};

class JTEncode
{
public:
  JTEncode(void);
  void jt65_encode(char *, uint8_t *);
  void jt9_encode(char *, uint8_t *);
  void jt4_encode(char *, uint8_t *);
  void wspr_encode(char *, char *, uint8_t, uint8_t *);
  void fsq_encode(char *, char *, uint8_t *);
  void fsq_dir_encode(char *, char *, char *, char *, uint8_t *);
private:
  uint8_t jt_code(char);
  uint8_t wspr_code(char);
  uint8_t gray_code(uint8_t);
  void jt_message_prep(char *);
  void wspr_message_prep(char *, char *, uint8_t);
  void jt65_bit_packing(char *, uint8_t *);
  void jt9_bit_packing(char *, uint8_t *);
  void wspr_bit_packing(uint8_t *);
  void jt65_interleave(uint8_t *);
  void jt9_interleave(uint8_t *);
  void wspr_interleave(uint8_t *);
  void jt9_packbits(uint8_t *, uint8_t *);
  void jt_gray_code(uint8_t *, uint8_t);
  void jt65_merge_sync_vector(uint8_t *, uint8_t *);
  void jt9_merge_sync_vector(uint8_t *, uint8_t *);
  void jt4_merge_sync_vector(uint8_t *, uint8_t *);
  void wspr_merge_sync_vector(uint8_t *, uint8_t *);
  void convolve(uint8_t *, uint8_t *, uint8_t, uint8_t);
  void rs_encode(uint8_t *, uint8_t *);
  void encode_rs_int(void *,data_t *, data_t *);
  void free_rs_int(void *);
  void * init_rs_int(int, int, int, int, int, int);
  void init_crc8(void);
  uint8_t crc8(char *);
  void * rs_inst;
  char callsign[7];
  char locator[5];
  uint8_t power;
  static uint8_t crc8_table[256];
};
