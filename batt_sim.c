// batt_sim.c: DO NOT MODIFY
//
// Battery simulator support functions.

#include "batt.h"

#define DISP_MAX_BITS 32

// These global variables are documented in the header file
short BATT_VOLTAGE_PORT;
unsigned char BATT_STATUS_PORT = 0;

int32_t BATT_DISPLAY_PORT = 0;
// Global variable used to control the scale display. Making changes to
// this variable will change the thermostat display. Type ensures 32 bits.

////////////////////////////////////////////////////////////////////////////////
// Data and functions to set the state of the thermo clock display 

#define NROWS 7
#define NCOLS 32
// Convenience struct for representing the thermometer display as
// characters
typedef struct {
  char chars[NROWS][NCOLS];
} display_t;

// 0 +-^^^-+  ####   ####  ####    0
// 1 |     |     #      #  #  #    1
// 2 |#####|     #      #  #  #    2
// 3 |#####|  ####      #  #  #  V 3
// 4 |#####|     #      #  #  #    4
// 5 |#####|     #      #  #  #    5
// 6 +-----+  #### o    #  ####    6
//   012345678901234567890123456789                       
//   0         1         2                                          

// Initial display config with decimal place
display_t init_display = {
  .chars = {
    "+-^^^-+                        ", 
    "|     |                        ",                 
    "|     |                        ",                 
    "|     |                        ",                 
    "|     |                        ",                 
    "|     |                        ",                    
    "+-----+                        ",
  }
};

// Reset display to be empty 
void reset_display(display_t *disp){
  *disp = init_display;
}

// Print an display 
void internal_print_display(display_t *display){
  for(int i=0; i<NROWS; i++){
    printf("%s\n",display->chars[i]);
  }
}  

// Position and char in display 
typedef struct {
  int r,c; int ch;
} charpos;
    
// Collection of characters corresponding to one bit in the state being set 
typedef struct {
  int len;                      // number of chars corresponding to this bit
  charpos pos[6];               // position of chars for this bit
} charpos_coll;


#define RGTH 22
#define MIDH 16
#define LFTH 9
#define TOPV 0
#define BARH 1

#define LFT 0
#define LMD 5
#define RMD 10
#define RGT 15


// Correspondence of bit positions to which characters should be set 
charpos_coll bits2chars[DISP_MAX_BITS] = {
  { .len=1, .pos={{ 4,28,'%'}}                                      }, // 0
  { .len=1, .pos={{ 3,28,'V'}}                                      }, // 1
  { .len=1, .pos={{ 6,14,'o'}}                                      }, // 2

  { .len=4, .pos={{TOPV+0,RGTH+0,'#'},{TOPV+0,RGTH+1,'#'},{TOPV+0,RGTH+2,'#'},{TOPV+0,RGTH+3,'#'}} }, // 3 
  { .len=4, .pos={{TOPV+0,RGTH+3,'#'},{TOPV+1,RGTH+3,'#'},{TOPV+2,RGTH+3,'#'},{TOPV+3,RGTH+3,'#'}} }, // 4
  { .len=4, .pos={{TOPV+3,RGTH+3,'#'},{TOPV+4,RGTH+3,'#'},{TOPV+5,RGTH+3,'#'},{TOPV+6,RGTH+3,'#'}} }, // 5
  { .len=4, .pos={{TOPV+6,RGTH+0,'#'},{TOPV+6,RGTH+1,'#'},{TOPV+6,RGTH+2,'#'},{TOPV+6,RGTH+3,'#'}} }, // 6
  { .len=4, .pos={{TOPV+3,RGTH+0,'#'},{TOPV+4,RGTH+0,'#'},{TOPV+5,RGTH+0,'#'},{TOPV+6,RGTH+0,'#'}} }, // 7
  { .len=4, .pos={{TOPV+0,RGTH+0,'#'},{TOPV+1,RGTH+0,'#'},{TOPV+2,RGTH+0,'#'},{TOPV+3,RGTH+0,'#'}} }, // 8
  { .len=4, .pos={{TOPV+3,RGTH+0,'#'},{TOPV+3,RGTH+1,'#'},{TOPV+3,RGTH+2,'#'},{TOPV+3,RGTH+3,'#'}} }, // 9 

  { .len=4, .pos={{TOPV+0,MIDH+0,'#'},{TOPV+0,MIDH+1,'#'},{TOPV+0,MIDH+2,'#'},{TOPV+0,MIDH+3,'#'}} }, // 10
  { .len=4, .pos={{TOPV+0,MIDH+3,'#'},{TOPV+1,MIDH+3,'#'},{TOPV+2,MIDH+3,'#'},{TOPV+3,MIDH+3,'#'}} }, // 11
  { .len=4, .pos={{TOPV+3,MIDH+3,'#'},{TOPV+4,MIDH+3,'#'},{TOPV+5,MIDH+3,'#'},{TOPV+6,MIDH+3,'#'}} }, // 12
  { .len=4, .pos={{TOPV+6,MIDH+0,'#'},{TOPV+6,MIDH+1,'#'},{TOPV+6,MIDH+2,'#'},{TOPV+6,MIDH+3,'#'}} }, // 13
  { .len=4, .pos={{TOPV+3,MIDH+0,'#'},{TOPV+4,MIDH+0,'#'},{TOPV+5,MIDH+0,'#'},{TOPV+6,MIDH+0,'#'}} }, // 14
  { .len=4, .pos={{TOPV+0,MIDH+0,'#'},{TOPV+1,MIDH+0,'#'},{TOPV+2,MIDH+0,'#'},{TOPV+3,MIDH+0,'#'}} }, // 15
  { .len=4, .pos={{TOPV+3,MIDH+0,'#'},{TOPV+3,MIDH+1,'#'},{TOPV+3,MIDH+2,'#'},{TOPV+3,MIDH+3,'#'}} }, // 16

  { .len=4, .pos={{TOPV+0,LFTH+0,'#'},{TOPV+0,LFTH+1,'#'},{TOPV+0,LFTH+2,'#'},{TOPV+0,LFTH+3,'#'}} }, // 17
  { .len=4, .pos={{TOPV+0,LFTH+3,'#'},{TOPV+1,LFTH+3,'#'},{TOPV+2,LFTH+3,'#'},{TOPV+3,LFTH+3,'#'}} }, // 18
  { .len=4, .pos={{TOPV+3,LFTH+3,'#'},{TOPV+4,LFTH+3,'#'},{TOPV+5,LFTH+3,'#'},{TOPV+6,LFTH+3,'#'}} }, // 19
  { .len=4, .pos={{TOPV+6,LFTH+0,'#'},{TOPV+6,LFTH+1,'#'},{TOPV+6,LFTH+2,'#'},{TOPV+6,LFTH+3,'#'}} }, // 20
  { .len=4, .pos={{TOPV+3,LFTH+0,'#'},{TOPV+4,LFTH+0,'#'},{TOPV+5,LFTH+0,'#'},{TOPV+6,LFTH+0,'#'}} }, // 21
  { .len=4, .pos={{TOPV+0,LFTH+0,'#'},{TOPV+1,LFTH+0,'#'},{TOPV+2,LFTH+0,'#'},{TOPV+3,LFTH+0,'#'}} }, // 22
  { .len=4, .pos={{TOPV+3,LFTH+0,'#'},{TOPV+3,LFTH+1,'#'},{TOPV+3,LFTH+2,'#'},{TOPV+3,LFTH+3,'#'}} }, // 23

  { .len=5,.pos={{5,1,'#'},{5,2,'#'},{5,3,'#'},{5,4,'#'},{5,5,'#'}} }, // 24
  { .len=5,.pos={{4,1,'#'},{4,2,'#'},{4,3,'#'},{4,4,'#'},{4,5,'#'}} }, // 25
  { .len=5,.pos={{3,1,'#'},{3,2,'#'},{3,3,'#'},{3,4,'#'},{3,5,'#'}} }, // 26
  { .len=5,.pos={{2,1,'#'},{2,2,'#'},{2,3,'#'},{2,4,'#'},{2,5,'#'}} }, // 27
  { .len=5,.pos={{1,1,'#'},{1,2,'#'},{1,3,'#'},{1,4,'#'},{1,5,'#'}} }, // 28
};

void set_display(display_t *disp, int bits){
  int i,j;
  int mask = 0x1;
  reset_display(disp);
  for(i=0; i<DISP_MAX_BITS; i++){
    //    printf("Checking %d\n",i);
    if( bits & (mask << i) ){ // ith bit set, fill in characters 
      //      printf("%d IS SET\n",i);
      charpos_coll coll = bits2chars[i];
      //      printf("Coll len: %d\n",coll.len);
      for(j=0; j<coll.len; j++){
        //        printf("Inner iter %d\n",j);
        charpos pos = coll.pos[j];
        disp->chars[pos.r][pos.c] = pos.ch;
        // print_thermo_display(thermo);
      }
    }
  }
}


// Use the global BATT_DISPLAY_PORT to show the display
void print_display(){
  display_t display;
  set_display(&display, BATT_DISPLAY_PORT);
  internal_print_display(&display);
  return;
}

bitspec_t dispspec = {
  .nbits = 32,
  .nclusters = 6,
  .clusters = {3, 5, 7, 7, 7, 3},
};

bitspec_t statspec = {
  .nbits = 8,
  .nclusters = 2,
  .clusters = {4,4},
};

// Generate a string version of the bits which clusters the bits based
// on the logical groupings in the problem
char *bitstr(int x, bitspec_t *spec){
  static char buffer[512];
  int idx = 0;
  int clust_idx = 0;
  int clust_break = spec->clusters[0];
  int max = spec->nbits-1;
  for(int i=0; i<spec->nbits; i++){
    if(i==clust_break){
      buffer[idx] = ' ';
      idx++;
      clust_idx++;
      clust_break += spec->clusters[clust_idx];
    }
    buffer[idx] = x & (1 << (max-i)) ? '1' : '0';
    idx++;
  }
  buffer[idx] = '\0';
  return buffer;
}

// Creates a string of bit indices matching the clustering pattern
// above
char *bitstr_index(bitspec_t *spec){
  static char buffer[512];
  char format[256];
  int pos = 0;
  int idx = spec->nbits;
  for(int i=0; i<spec->nclusters; i++){
    idx -= spec->clusters[i];
    if(spec->clusters[i] > 1){
      sprintf(format, "%s%dd ","%",spec->clusters[i]);
      pos += sprintf(buffer+pos, format, idx);
    }
    else{                       // cluster of 1 bit gets no index
      pos += sprintf(buffer+pos, "  ");
    }
  }
  buffer[pos-1] = '\0';         // eliminate trailing space
  return buffer;
}
