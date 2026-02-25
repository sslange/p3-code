#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>              // for debugging

////////////////////////////////////////////////////////////////////////////////
// battery meter structs/functions

// STRUCTS_BEGIN
// Breaks battery stats down into constituent parts
typedef struct{
  short mlvolts;     // voltage read from port, units of 0.001 Volts (milli Volts)
  char  percent;     // percent full converted from voltage
  char  mode;        // 1 for percent, 2 for volts, set based on bit 4 of BATT_STATUS_PORT
} batt_t;

// Accessing fields via a batt_t * pointer
// |----------------+---------+-------------+-------------------|
// |                |         | Destination | Assembly          |
// | C Field Access | Offset  | Size        | Assign 5 to field |
// |----------------+---------+-------------+-------------------|
// | batt->mlvolts  | 0 bytes | 2 bytes     | movw $5, 0(%reg)  |
// | batt->percent  | 2 bytes | 1 bytes     | movb $5, 2(%reg)  |
// | batt->mode     | 3 bytes | 1 bytes     | movb $5, 3(%reg)  |
// |----------------+---------+-------------+-------------------|
//
// Accessing fields via a batt_t packed struct
// |----------------+--------+-------------+---------|
// |                |   Bits | Shift       |         |
// | C Field Access | in reg | Required    | Size    |
// |----------------+--------+-------------+---------|
// | batt.mlvolts   |   0-15 | None        | 2 bytes |
// | batt.percent   |  16-23 | Right by 16 | 1 byte  |
// | batt.mode      |  24-31 | Right by 24 | 1 byte  |
// |----------------+--------+-------------+---------|

// STRUCTS_END

// Functions to implement for battery problem
int set_batt_from_ports(batt_t *batt);
int set_display_from_batt(batt_t batt, int *display);
int batt_update();

////////////////////////////////////////////////////////////////////////////////
// batt_sim.c structs/functions; do not modify

extern short BATT_VOLTAGE_PORT;
// Sensor tied to the battery, provides a measure of voltage in units
// of 0.0005 volts (half milli volts). The sensor can sense a wide
// range of voltages including negatives but the batteries being
// measured are Full when 3.8V (7600 sensor value) or above is read
// and Empty when 3.0V (6000 sensor value) or lower is read.

extern unsigned char BATT_STATUS_PORT;
// The bit at index 4 indicates whether display should be in Volts (0)
// or Percent (1); the bit is tied to a user button which will toggle
// it between modes. Other bits in this char may be set to indicate
// the status of other parts of the meter and should be ignored: ONLY
// BIT 4 BIT MATTERS. C code should only read this port. 

extern int BATT_DISPLAY_PORT;
// Controls battery meter display. Readable and writable. C code
// should mostly write this port with a sequence of bits which will
// light up specific elements of the LCD panel.

////////////////////////////////////////////////////////////////////////////////
// data and printing routines for displaying bits nicely
typedef struct {
  int nbits;
  int nclusters;
  int clusters[32];
} bitspec_t;

extern bitspec_t dispspec;
extern bitspec_t statspec;
extern bitspec_t indicatorspec;

char *bitstr(int x, bitspec_t *spec);
char *bitstr_index(bitspec_t *spec);

void print_display();
// Show the simulated display as ASCII on the screen

