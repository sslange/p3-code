// batt_sim.c: DO NOT MODIFY
//
// Battery meter simulator main program and supporting functions. Read
// voltage sensor value and mode (volts or percent) from the command
// line and show the results of running functions from batt_update.c
// on the screen.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "batt.h"
//                        43210
#define VOLTAGE_FLAG 0b01101111     // bit 4 is 0 but others may be set
#define PERCENT_FLAG 0b10010001     // bit 4 is 1 but others may be set

int main(int argc, char **argv){

  if(argc < 3){                 
    printf("usage: %s {voltage_val} {V | P}\n",argv[0]);
    printf("  arg1 voltage_val: integer, units of 0.001 volts \n");
    printf("  arg2 V or P: Voltage or Percent display\n");
    return 0;
  }
  BATT_VOLTAGE_PORT = atoi(argv[1]);
  printf("BATT_VOLTAGE_PORT set to: %u\n", BATT_VOLTAGE_PORT);

  if(argv[2][0]=='V' || argv[2][0]=='v'){
    BATT_STATUS_PORT |= VOLTAGE_FLAG;
  }
  else if(argv[2][0]=='P' || argv[2][0]=='p'){
    BATT_STATUS_PORT |= PERCENT_FLAG;
  }
  else{
    printf("Unknown display mode: '%s'\n",argv[2]);
    printf("Should be 'V' or 'P'\n");
    return 1;
  }

  printf("BATT_VOLTAGE_PORT: %hd\n",BATT_VOLTAGE_PORT);
  printf("BATT_STAUS_PORT:   %s\n",bitstr(BATT_STATUS_PORT, &statspec));
  printf("index:             %s\n",bitstr_index(&statspec));

  batt_t *batt = malloc(sizeof(batt_t));
  int result;

  printf("result = set_batt_from_ports(batt);\n");
  result = set_batt_from_ports(batt);
  printf("result: %d\n",result);
  // printf("%s", (result == 0) ? "" : "WARNING: Non-zero value returned\n");

  printf("batt = {\n"); 
  printf("  mlvolts = %d\n", batt->mlvolts);
  printf("  percent = %d\n", batt->percent);
  printf("  mode    = %d\n", batt->mode);
  printf("}\n");

  printf("result = batt_update();\n");
  result = batt_update();
  printf("result: %d\n", result);
  // printf("%s", (result == 0) ? "" : "WARNING: Non-zero value returned\n");

  printf("BATT_VOLTAGE_PORT: %hd\n", BATT_VOLTAGE_PORT);
  printf("BATT_DISPLAY_PORT:\n");
  printf("bits:  %s\n",bitstr(BATT_DISPLAY_PORT, &dispspec));
  printf("index: %s\n",bitstr_index(&dispspec));
  printf("Batt Display:\n");
  print_display();

  free(batt);
  return 0;

}
