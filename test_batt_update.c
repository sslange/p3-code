// test_batt_update.c: testing program for functions in batt_update.c

#include <string.h>
#include <math.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include "batt.h"

// macro to set up a test with given name, print the source of the
// test; very hacky, fragile, but useful
#define IF_TEST(TNAME) \
  if( RUNALL || strcmp( TNAME, test_name)==0 ) { \
    sprintf(sysbuf,"awk 'NR==(%d){P=1;gsub(\"^ *\",\"\");} P==1 && /ENDTEST/{P=0; print \"}\\n---OUTPUT---\"} P==1{print}' %s", __LINE__, __FILE__); \
    system(sysbuf); nrun++;  \
  } \
  if( RUNALL || strcmp( TNAME, test_name)==0 )

char sysbuf[1024];
int RUNALL = 0;
int nrun = 0;


void print_batt(batt_t *batt){
  printf("{\n"); 
  printf("  mlvolts = %d,\n",batt->mlvolts);
  printf("  percent = %d,\n",batt->percent);
  printf("  mode    = %d,\n",batt->mode);
  printf("}\n");
}

void print_ports(){
  printf("%-18s : %d\n","BATT_VOLTAGE_PORT", BATT_VOLTAGE_PORT);
  printf("%-18s : %s\n","BATT_STATUS_PORT", bitstr(BATT_STATUS_PORT, &statspec));
  printf("%-18s : %s\n","index",bitstr_index(&statspec));
  printf("%-18s : %s\n","BATT_DISPLAY_PORT",bitstr(BATT_DISPLAY_PORT,&dispspec));
  printf("%-18s : %s\n","index",bitstr_index(&dispspec));
}

// prints an integer as though it were the display
void print_dispint_display(int dispint){
  printf("DISPINT BITS ARE:\n");
  printf("bits:  %s\n",bitstr(dispint, &dispspec));
  printf("index: %s\n",bitstr_index(&dispspec));
  int old_port = BATT_DISPLAY_PORT;
  printf("DISPINT AS DISPLAY:\n");
  BATT_DISPLAY_PORT = dispint;
  print_display();
  BATT_DISPLAY_PORT = old_port;
}

// defined in assembly to set up registers before calling required functions
int CALL_set_batt_from_ports(batt_t *);
int CALL_set_display_from_batt(batt_t, int *);
int CALL_batt_update();

// Used in assembly wrapper to retrieve callee save registers
char *callee_reg_name[6] = {"%rbx", "%rbp", "%r12", "%r13", "%r14", "%r15"};
long callee_reg_vals_actual[6] = {};
long callee_reg_vals_expect[6] = {
  0xBADBADBADBAD0001,  // %rbx
  0xBADBADBADBAD0007,  // %rbp
  0xBADBADBADBAD000C,  // %r12
  0xBADBADBADBAD000D,  // %r13
  0xBADBADBADBAD000E,  // %r14
  0xBADBADBADBAD000F,  // %r15
};

void check_callee_registers(){
  for(int i=0; i<6; i++){
    if(callee_reg_vals_actual[i] != callee_reg_vals_expect[i]){
      char *regname = callee_reg_name[i];
      printf("%4s: Callee Register changed during function\n",regname);
      printf("  Expect: 0x%08lX\n",callee_reg_vals_expect[i]);
      printf("  Actual: 0x%08lX\n",callee_reg_vals_actual[i]);
      printf("Did you use it and neglect to restore it? Try a push/pop combination\n");
    }
  }
}

int main(int argc, char *argv[]){
  if(argc < 2){
    printf("usage: %s <test_name>\n", argv[0]);
    return 1;
  }
  char *test_name = argv[1];
  char sysbuf[1024];

  int *dispint = malloc(sizeof(int)); // used for tests that set the display
  batt_t *actual_batt = malloc(sizeof(batt_t));

  if(0){}

  ////////////////////////////////////////////////////////////////////////////////
  // set_batt_from_ports() tests
  IF_TEST("set_batt_from_ports() 0 V") {
    // Check sensor value of 0 and status set for voltage
    BATT_VOLTAGE_PORT = 0;
    BATT_STATUS_PORT  = 0b000000;
    BATT_DISPLAY_PORT = -1;
    int ret = CALL_set_batt_from_ports(actual_batt);
    printf("ret: %d\n",ret);
    printf("actual = "); print_batt(actual_batt);
    print_ports();
    check_callee_registers();
  } // ENDTEST

  IF_TEST("set_batt_from_ports() 0 P") {
    // Check sensor value of 0 and status set for percent
    BATT_VOLTAGE_PORT = 0;
    BATT_STATUS_PORT  = 0b010000;
    BATT_DISPLAY_PORT = -1;
    int ret = CALL_set_batt_from_ports(actual_batt);
    printf("ret: %d\n",ret);
    printf("actual = "); print_batt(actual_batt);
    print_ports();
    check_callee_registers();
  } // ENDTEST

  IF_TEST("set_batt_from_ports() 7400 V") {
    // Check sensor value of 7400 (3.70 V) and status set for voltage
    BATT_VOLTAGE_PORT = 7400;
    BATT_STATUS_PORT  = 0b000000;
    BATT_DISPLAY_PORT = -1;
    int ret = CALL_set_batt_from_ports(actual_batt);
    printf("ret: %d\n",ret);
    printf("actual = "); print_batt(actual_batt);
    print_ports();
    check_callee_registers();
  } // ENDTEST

  IF_TEST("set_batt_from_ports() 7400 P") {
    // Check sensor value of 7400 (3.70 V) and status set for percent
    BATT_VOLTAGE_PORT = 7400;
    BATT_STATUS_PORT  = 0b010000;
    BATT_DISPLAY_PORT = -1;
    int ret = CALL_set_batt_from_ports(actual_batt);
    printf("ret: %d\n",ret);
    printf("actual = "); print_batt(actual_batt);
    print_ports();
    check_callee_registers();
  } // ENDTEST

  IF_TEST("set_batt_from_ports() mixed STATUS V") {
    // Checks that only bit 4 of BATT_STATUS_PORT is used for
    // determining the Voltage/Percent mode. Bit 4 is 0 but other bits
    // are set in this test
    BATT_VOLTAGE_PORT = 7500;
    BATT_STATUS_PORT  = 0b10101001;
    BATT_DISPLAY_PORT = -1;
    int ret = CALL_set_batt_from_ports(actual_batt);
    printf("ret: %d\n",ret);
    printf("actual = "); print_batt(actual_batt);
    print_ports();
    check_callee_registers();
  } // ENDTEST

  IF_TEST("set_batt_from_ports() mixed STATUS P") {
    // Checks that only bit 4 of BATT_STATUS_PORT is used for
    // determining the Voltage/Percent mode. Bit 4 is 0 but other bits
    // are set in this test
    BATT_VOLTAGE_PORT = 6277;
    BATT_STATUS_PORT  = 0b01110110;
    BATT_DISPLAY_PORT = -1;
    int ret = CALL_set_batt_from_ports(actual_batt);
    printf("ret: %d\n",ret);
    printf("actual = "); print_batt(actual_batt);
    print_ports();
    check_callee_registers();
  } // ENDTEST

  IF_TEST("set_batt_from_ports() 7845 P is 100%") {
    // Checks that the percentage is set to 100% if voltage port is
    // sufficiently high.
    BATT_VOLTAGE_PORT = 7845;
    BATT_STATUS_PORT  = 0b010000;
    BATT_DISPLAY_PORT = -1;
    int ret = CALL_set_batt_from_ports(actual_batt);
    printf("ret: %d\n",ret);
    printf("actual = "); print_batt(actual_batt);
    print_ports();
    check_callee_registers();
  } // ENDTEST

  IF_TEST("set_batt_from_ports() 2500 V") {
    // Checks a low voltage rating should be 0 percent
    BATT_VOLTAGE_PORT = 2500;
    BATT_STATUS_PORT  = 0b000000;
    BATT_DISPLAY_PORT = -1;
    int ret = CALL_set_batt_from_ports(actual_batt);
    printf("ret: %d\n",ret);
    printf("actual = "); print_batt(actual_batt);
    print_ports();
    check_callee_registers();
  } // ENDTEST

  IF_TEST("set_batt_from_ports() 6579 P") {
    // Checks proper voltage / percent for a mid-range sensor value.
    BATT_VOLTAGE_PORT = 6579;
    BATT_STATUS_PORT  = 0b010000;
    BATT_DISPLAY_PORT = -1;
    int ret = CALL_set_batt_from_ports(actual_batt);
    printf("ret: %d\n",ret);
    printf("actual = "); print_batt(actual_batt);
    print_ports();
    check_callee_registers();
  } // ENDTEST

  IF_TEST("set_batt_from_ports() 6016 V") {
    // Low voltage sensor value should yield 1%
    BATT_VOLTAGE_PORT = 6016;
    BATT_STATUS_PORT  = 0b101100;
    BATT_DISPLAY_PORT = -1;
    int ret = CALL_set_batt_from_ports(actual_batt);
    printf("ret: %d\n",ret);
    printf("actual = "); print_batt(actual_batt);
    print_ports();
    check_callee_registers();
  } // ENDTEST

  IF_TEST("set_batt_from_ports() 6050 P") {
    // Low voltage sensor value, should be nonzero percent
    BATT_VOLTAGE_PORT = 6050;
    BATT_STATUS_PORT  = 0b110000;
    BATT_DISPLAY_PORT = -1;
    int ret = CALL_set_batt_from_ports(actual_batt);
    printf("ret: %d\n",ret);
    printf("actual = "); print_batt(actual_batt);
    print_ports();
    check_callee_registers();
  } // ENDTEST

  IF_TEST("set_batt_from_ports() FAILS") {
    // Negative voltage sensor value should cause a failure and leave
    // the batt_t struct 'actual' unchanged.
    BATT_VOLTAGE_PORT = -7200;
    BATT_STATUS_PORT  = 0b110011;
    BATT_DISPLAY_PORT = -1;
    memset(actual_batt, 0, sizeof(batt_t));
    int ret = CALL_set_batt_from_ports(actual_batt);
    printf("ret: %d\n",ret);
    printf("actual = "); print_batt(actual_batt);
    print_ports();
    check_callee_registers();
  } // ENDTEST

  ////////////////////////////////////////////////////////////////////////////////
  // set_display_from_batt() tests
  ////////////////////////////////////////////////////////////////////////////////

  IF_TEST("set_display_from_batt() 0 V") {
    // Show 0.0 V
    BATT_VOLTAGE_PORT = -1;
    BATT_STATUS_PORT  = -1;
    BATT_DISPLAY_PORT = -1;
    batt_t batt = {
      .mlvolts = 0,
      .percent = 0,
      .mode = 2,
    };
    int ret = CALL_set_display_from_batt(batt, dispint);
    printf("ret: %d\n",ret);
    printf("%-18s : %s\n%-18s : %s\n",
           "dispint bits", bitstr(*dispint, &dispspec),
           "index", bitstr_index(&dispspec));
    printf("\n");  print_ports();  printf("\n");
    print_dispint_display(*dispint);
    check_callee_registers();
  } // ENDTEST

  IF_TEST("set_display_from_batt() 0 %") {
    // Given 0 volts which should also show 0% battery remaining
    BATT_VOLTAGE_PORT = -1;
    BATT_STATUS_PORT  = -1;
    BATT_DISPLAY_PORT = -1;
    batt_t batt = {
      .mlvolts = 0,
      .percent = 0,
      .mode = 1,
    };
    int ret = CALL_set_display_from_batt(batt, dispint);
    printf("ret: %d\n",ret);
    printf("%-18s : %s\n%-18s : %s\n",
           "dispint bits", bitstr(*dispint, &dispspec),
           "index", bitstr_index(&dispspec));
    printf("\n");  print_ports();  printf("\n");
    print_dispint_display(*dispint);
    check_callee_registers();
  } // ENDTEST

  IF_TEST("set_display_from_batt() 3.50 V") {
    // Set dispint from the given batt which is in Voltage mode
    // and should result in a bit arrangement showing 3.50 V. 
    BATT_VOLTAGE_PORT = -1;
    BATT_STATUS_PORT  = -1;
    BATT_DISPLAY_PORT = -1;
    batt_t batt = {
      .mlvolts = 3500,
      .percent = 62,
      .mode = 2,
    };
    int ret = CALL_set_display_from_batt(batt, dispint);
    printf("ret: %d\n",ret);
    printf("%-18s : %s\n%-18s : %s\n",
           "dispint bits", bitstr(*dispint, &dispspec),
           "index", bitstr_index(&dispspec));
    printf("\n");  print_ports();  printf("\n");
    print_dispint_display(*dispint);
    check_callee_registers();
  } // ENDTEST

  IF_TEST("set_display_from_batt() 3.507 V") {
    // Should show 3.51 V: rounds Up associated with the last digit
    // of the voltage reading.
    BATT_VOLTAGE_PORT = -1;
    BATT_STATUS_PORT  = -1;
    BATT_DISPLAY_PORT = -1;
    batt_t batt = {
      .mlvolts = 3507,
      .percent = 62,
      .mode = 2,
    };
    int ret = CALL_set_display_from_batt(batt, dispint);
    printf("ret: %d\n",ret);
    printf("%-18s : %s\n%-18s : %s\n",
           "dispint bits", bitstr(*dispint, &dispspec),
           "index", bitstr_index(&dispspec));
    printf("\n");  print_ports();  printf("\n");
    print_dispint_display(*dispint);
    check_callee_registers();
  } // ENDTEST
  
  IF_TEST("set_display_from_batt() 62 %") {
    // Percent mode, show 62 %
    BATT_VOLTAGE_PORT = -1;
    BATT_STATUS_PORT  = -1;
    BATT_DISPLAY_PORT = -1;
    batt_t batt = {
      .mlvolts = 3500,
      .percent = 62,
      .mode = 1,
    };
    int ret = CALL_set_display_from_batt(batt, dispint);
    printf("ret: %d\n",ret);
    printf("%-18s : %s\n%-18s : %s\n",
           "dispint bits", bitstr(*dispint, &dispspec),
           "index", bitstr_index(&dispspec));
    printf("\n");  print_ports();  printf("\n");
    print_dispint_display(*dispint);
    check_callee_registers();
  } // ENDTEST

  IF_TEST("set_display_from_batt() 87 %") {
    // Show 87 %
    BATT_VOLTAGE_PORT = -1;
    BATT_STATUS_PORT  = -1;
    BATT_DISPLAY_PORT = -1;
    batt_t batt = {
      .mlvolts = 3700,
      .percent = 87,
      .mode = 1,
    };
    int ret = CALL_set_display_from_batt(batt, dispint);
    printf("ret: %d\n",ret);
    printf("%-18s : %s\n%-18s : %s\n",
           "dispint bits", bitstr(*dispint, &dispspec),
           "index", bitstr_index(&dispspec));
    printf("\n");  print_ports();  printf("\n");
    print_dispint_display(*dispint);
    check_callee_registers();
  } // ENDTEST
  
  IF_TEST("set_display_from_batt() 4.21 V") {
    // Show 4.21 Volts
    BATT_VOLTAGE_PORT = -1;
    BATT_STATUS_PORT  = -1;
    BATT_DISPLAY_PORT = -1;
    batt_t batt = {
      .mlvolts = 4217,
      .percent = 100,
      .mode = 2,
    };
    int ret = CALL_set_display_from_batt(batt, dispint);
    printf("ret: %d\n",ret);
    printf("%-18s : %s\n%-18s : %s\n",
           "dispint bits", bitstr(*dispint, &dispspec),
           "index", bitstr_index(&dispspec));
    printf("\n");  print_ports();  printf("\n");
    print_dispint_display(*dispint);
    check_callee_registers();
  } // ENDTEST

  IF_TEST("set_display_from_batt() 100 %") {
    // Show 100%, the only case in which the hundreds digit of percent
    // is used.
    BATT_VOLTAGE_PORT = -1;
    BATT_STATUS_PORT  = -1;
    BATT_DISPLAY_PORT = -1;
    batt_t batt = {
      .mlvolts = 4217,
      .percent = 100,
      .mode = 1,
    };
    int ret = CALL_set_display_from_batt(batt, dispint);
    printf("ret: %d\n",ret);
    printf("%-18s : %s\n%-18s : %s\n",
           "dispint bits", bitstr(*dispint, &dispspec),
           "index", bitstr_index(&dispspec));
    printf("\n");  print_ports();  printf("\n");
    print_dispint_display(*dispint);
    check_callee_registers();
  } // ENDTEST

  IF_TEST("set_display_from_batt() level 1") {
    // Non-zero level that should show 1 bar in the level meter
    BATT_VOLTAGE_PORT = -1;
    BATT_STATUS_PORT  = -1;
    BATT_DISPLAY_PORT = -1;
    batt_t batt = {
      .mlvolts = 6151,
      .percent = 9,
      .mode = 1,
    };
    int ret = CALL_set_display_from_batt(batt, dispint);
    printf("ret: %d\n",ret);
    printf("%-18s : %s\n%-18s : %s\n",
           "dispint bits", bitstr(*dispint, &dispspec),
           "index", bitstr_index(&dispspec));
    printf("\n");  print_ports();  printf("\n");
    print_dispint_display(*dispint);
    check_callee_registers();
  } // ENDTEST

  IF_TEST("set_display_from_batt() level 2") {
    // 30% level is first point at which 2 bars show
    BATT_VOLTAGE_PORT = -1;
    BATT_STATUS_PORT  = -1;
    BATT_DISPLAY_PORT = -1;
    batt_t batt = {
      .mlvolts = 6480,
      .percent = 30,
      .mode = 1,
    };
    int ret = CALL_set_display_from_batt(batt, dispint);
    printf("ret: %d\n",ret);
    printf("%-18s : %s\n%-18s : %s\n",
           "dispint bits", bitstr(*dispint, &dispspec),
           "index", bitstr_index(&dispspec));
    printf("\n");  print_ports();  printf("\n");
    print_dispint_display(*dispint);
    check_callee_registers();
  } // ENDTEST

  IF_TEST("set_display_from_batt() level 3") {
    // 56% level is should show 3 bars
    BATT_VOLTAGE_PORT = -1;
    BATT_STATUS_PORT  = -1;
    BATT_DISPLAY_PORT = -1;
    batt_t batt = {
      .mlvolts = 6900,
      .percent = 56,
      .mode = 2,
    };
    int ret = CALL_set_display_from_batt(batt, dispint);
    printf("ret: %d\n",ret);
    printf("%-18s : %s\n%-18s : %s\n",
           "dispint bits", bitstr(*dispint, &dispspec),
           "index", bitstr_index(&dispspec));
    printf("\n");  print_ports();  printf("\n");
    print_dispint_display(*dispint);
    check_callee_registers();
  } // ENDTEST


  IF_TEST("set_display_from_batt() level 4") {
    // 89% is highest percentage at which 4 bars will be shown
    BATT_VOLTAGE_PORT = -1;
    BATT_STATUS_PORT  = -1;
    BATT_DISPLAY_PORT = -1;
    batt_t batt = {
      .mlvolts = 3713,
      .percent = 89,
      .mode = 2,
    };
    int ret = CALL_set_display_from_batt(batt, dispint);
    printf("ret: %d\n",ret);
    printf("%-18s : %s\n%-18s : %s\n",
           "dispint bits", bitstr(*dispint, &dispspec),
           "index", bitstr_index(&dispspec));
    printf("\n");  print_ports();  printf("\n");
    print_dispint_display(*dispint);
    check_callee_registers();
  } // ENDTEST

  IF_TEST("set_display_from_batt() level 5") {
    // 90% is lowest percentage at which 5 bars will be shown
    BATT_VOLTAGE_PORT = -1;
    BATT_STATUS_PORT  = -1;
    BATT_DISPLAY_PORT = -1;
    batt_t batt = {
      .mlvolts = 3720,
      .percent = 90,
      .mode = 1,
    };
    int ret = CALL_set_display_from_batt(batt, dispint);
    printf("ret: %d\n",ret);
    printf("%-18s : %s\n%-18s : %s\n",
           "dispint bits", bitstr(*dispint, &dispspec),
           "index", bitstr_index(&dispspec));
    printf("\n");  print_ports();  printf("\n");
    print_dispint_display(*dispint);
    check_callee_registers();
  } // ENDTEST

  IF_TEST("set_display_from_batt() level 5 V") {
    // 90% is lowest percentage at which 5 bars will be shown
    BATT_VOLTAGE_PORT = -1;
    BATT_STATUS_PORT  = -1;
    BATT_DISPLAY_PORT = -1;
    batt_t batt = {
      .mlvolts = 3721,
      .percent = 90,
      .mode = 2,
    };
    int ret = CALL_set_display_from_batt(batt, dispint);
    printf("ret: %d\n",ret);
    printf("%-18s : %s\n%-18s : %s\n",
           "dispint bits", bitstr(*dispint, &dispspec),
           "index", bitstr_index(&dispspec));
    printf("\n");  print_ports();  printf("\n");
    print_dispint_display(*dispint);
    check_callee_registers();
  } // ENDTEST

  IF_TEST("set_display_from_batt() error mlvolts") {
    // Should detect negative voltage and error out
    BATT_VOLTAGE_PORT = -1;
    BATT_STATUS_PORT  = -1;
    BATT_DISPLAY_PORT = -1;
    batt_t batt = {
      .mlvolts = -3421,
      .percent = 0,
      .mode = 1,
    };
    *dispint = -1;        // should not change from -1
    int ret = CALL_set_display_from_batt(batt, dispint);
    printf("ret: %d\n",ret);
    printf("%-18s : %s\n%-18s : %s\n",
           "dispint bits", bitstr(*dispint, &dispspec),
           "index", bitstr_index(&dispspec));
    printf("\n");  print_ports();  printf("\n");
    print_dispint_display(*dispint);
    check_callee_registers();
  } // ENDTEST

  IF_TEST("set_display_from_batt() error mode") {
    // Should detect mode is not 1 or 2 and error out
    BATT_VOLTAGE_PORT = -1;
    BATT_STATUS_PORT  = -1;
    BATT_DISPLAY_PORT = -1;
    batt_t batt = {
      .mlvolts = 3200,
      .percent = 40,
      .mode = 4,
    };
    *dispint = -1;        // should not change from -1
    int ret = CALL_set_display_from_batt(batt, dispint);
    printf("ret: %d\n",ret);
    printf("%-18s : %s\n%-18s : %s\n",
           "dispint bits", bitstr(*dispint, &dispspec),
           "index", bitstr_index(&dispspec));
    printf("\n");  print_ports();  printf("\n");
    print_dispint_display(*dispint);
    check_callee_registers();
  } // ENDTEST

  IF_TEST("Repeated set_display_from_batt()") {
    // Repeatedly call set_display_from_batt() to
    // ensure it functions properly with use
    // several times in a row
    BATT_VOLTAGE_PORT = -1;
    BATT_STATUS_PORT  = -1;
    BATT_DISPLAY_PORT = -1;
    batt_t batt = {}; int ret;
    printf("=== CALL 1 ====\n");
    batt.mlvolts = 3609;
    batt.percent = 76;
    batt.mode = 2;
    ret = CALL_set_display_from_batt(batt, dispint);
    printf("ret: %d\n",ret);
    printf("%-18s : %s\n%-18s : %s\n",
           "dispint bits", bitstr(*dispint, &dispspec),
           "index", bitstr_index(&dispspec));
    printf("\n");  print_ports();  printf("\n");
    print_dispint_display(*dispint);
    check_callee_registers();
    printf("=== CALL 2 ====\n");
    batt.mlvolts = 3193;
    batt.percent = 24;
    batt.mode = 1;
    ret = CALL_set_display_from_batt(batt, dispint);
    printf("ret: %d\n",ret);
    printf("%-18s : %s\n%-18s : %s\n",
           "dispint bits", bitstr(*dispint, &dispspec),
           "index", bitstr_index(&dispspec));
    printf("\n");  print_ports();  printf("\n");
    print_dispint_display(*dispint);
    check_callee_registers();
    printf("=== CALL 3 ====\n");
    batt.mlvolts = 3796;
    batt.percent = 99;
    batt.mode = 1;
    ret = CALL_set_display_from_batt(batt, dispint);
    printf("ret: %d\n",ret);
    printf("%-18s : %s\n%-18s : %s\n",
           "dispint bits", bitstr(*dispint, &dispspec),
           "index", bitstr_index(&dispspec));
    printf("\n");  print_ports();  printf("\n");
    print_dispint_display(*dispint);
    check_callee_registers();
    printf("=== CALL 4 ====\n");
    batt.mlvolts = 3399;
    batt.percent = 49;
    batt.mode = 2;
    ret = CALL_set_display_from_batt(batt, dispint);
    printf("ret: %d\n",ret);
    printf("%-18s : %s\n%-18s : %s\n",
           "dispint bits", bitstr(*dispint, &dispspec),
           "index", bitstr_index(&dispspec));
    printf("\n");  print_ports();  printf("\n");
    print_dispint_display(*dispint);
    check_callee_registers();
  } // ENDTEST



  IF_TEST("First Two Functions in Sequence") {
    // Call first two functions in sequence to
    // detect if there are any problems with such
    // with dat transfer
    BATT_VOLTAGE_PORT = 6573;
    BATT_STATUS_PORT  = 0b01010100; // Percent mode
    BATT_DISPLAY_PORT = -1;
    print_ports();
    printf("==== CALLING 1st FUNCTION ====\n");
    int ret = CALL_set_batt_from_ports(actual_batt);
    printf("ret: %d\n",ret);
    printf("actual = "); print_batt(actual_batt);
    printf("==== CALLING 2nd FUNCTION ====\n");
    ret = CALL_set_display_from_batt(*actual_batt, dispint);
    printf("ret: %d\n",ret);
    printf("%-18s : %s\n%-18s : %s\n",
           "dispint bits", bitstr(*dispint, &dispspec),
           "index", bitstr_index(&dispspec));
    print_ports();
    print_dispint_display(*dispint);
    check_callee_registers();
  } // ENDTEST

  ////////////////////////////////////////////////////////////////////////////////
  // batt_update() tests

  IF_TEST("batt_update() 7400 V") {
    // call batt_update() with given sensor and status
    BATT_VOLTAGE_PORT = 7400;
    BATT_STATUS_PORT  = 0b01100110; // Volts mode
    BATT_DISPLAY_PORT = -1;
    int ret = CALL_batt_update();
    printf("ret: %d\n",ret);
    printf("\n");  print_ports();  printf("\n");
    printf("Display based on BATT_DISPLAY_PORT:\n");
    print_display();
    check_callee_registers();
  } // ENDTEST

  IF_TEST("batt_update() 7400 P") {
    // call batt_update() with given sensor and status
    BATT_VOLTAGE_PORT = 7400;
    BATT_STATUS_PORT  = 0b01010100; // Percent mode
    BATT_DISPLAY_PORT = -1;
    int ret = CALL_batt_update();
    printf("ret: %d\n",ret);
    printf("\n");  print_ports();  printf("\n");
    printf("Display based on BATT_DISPLAY_PORT:\n");
    print_display();
    check_callee_registers();
  } // ENDTEST

  IF_TEST("batt_update() 7291 P") {
    // call batt_update() with given sensor and status
    BATT_VOLTAGE_PORT = 7291;
    BATT_STATUS_PORT  = 0b011011; // Percent mode
    BATT_DISPLAY_PORT = -1;
    int ret = CALL_batt_update();
    printf("ret: %d\n",ret);
    printf("\n");  print_ports();  printf("\n");
    printf("Display based on BATT_DISPLAY_PORT:\n");
    print_display();
    check_callee_registers();
  } // ENDTEST

  IF_TEST("batt_update() 7645 V") {
    // call batt_update() with given sensor and status
    BATT_VOLTAGE_PORT = 7645;
    BATT_STATUS_PORT  = 0b01100101; // Volts mode
    BATT_DISPLAY_PORT = -1;
    int ret = CALL_batt_update();
    printf("ret: %d\n",ret);
    printf("\n");  print_ports();  printf("\n");
    printf("Display based on BATT_DISPLAY_PORT:\n");
    print_display();
    check_callee_registers();
  } // ENDTEST

  IF_TEST("batt_update() repeated calls") {
    // Call batt_update() several times in a row
    // to ensure no problems result from lingering
    // data changes / use of globals / etc.
    printf("====FIRST CALL TO batt_update()====\n");
    BATT_VOLTAGE_PORT = 6297;
    BATT_STATUS_PORT  = 0b01001101; // Volts mode
    BATT_DISPLAY_PORT = -1;
    int ret = CALL_batt_update();
    printf("ret: %d\n",ret);
    print_ports();  printf("\n");
    printf("Display based on BATT_DISPLAY_PORT:\n");
    print_display();
    check_callee_registers();
    printf("\n");
    printf("====SECOND CALL TO batt_update()====\n");
    BATT_VOLTAGE_PORT = 6980;
    BATT_STATUS_PORT  = 0b11010111; // Percent mode
    BATT_DISPLAY_PORT = -1;
    ret = CALL_batt_update();
    printf("ret: %d\n",ret);
    print_ports();  printf("\n");
    printf("Display based on BATT_DISPLAY_PORT:\n");
    print_display();
    check_callee_registers();

  } // ENDTEST


  IF_TEST("batt_update() error") {
    // call batt_update() with negative voltage reading which should
    // bail out without changing anything.
    BATT_VOLTAGE_PORT = -6421;  // Negative value causes error out
    BATT_STATUS_PORT  = 0b111011; // Percent mode
    BATT_DISPLAY_PORT = -1;
    int ret = CALL_batt_update();
    printf("ret: %d\n",ret);
    printf("\n");  print_ports();  printf("\n");
    printf("Display based on BATT_DISPLAY_PORT:\n");
    print_display();
    check_callee_registers();
  } // ENDTEST

  ////////////////////////////////////////////////////////////////////////////////
  // END MATTER
  ////////////////////////////////////////////////////////////////////////////////
  if(nrun == 0){
    printf("No test named '%s' found\n",test_name);
    return 1;
  }
  else if(nrun > 1){
    printf("%d tests run\n",nrun);
  }

  free(actual_batt);
  free(dispint);

  return 0;
}
