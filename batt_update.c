#include "batt.h"
int set_batt_from_ports(batt_t *batt){
    if (BATT_VOLTAGE_PORT <0){
        return 1;
    }
    batt->mlvolts= (BATT_VOLTAGE_PORT >>1); //set voltage using formula
    int BattV= batt->mlvolts;

    if (BATT_STATUS_PORT & (1 << 4)){ //if at 4, it is 1, mode is percent
        batt->mode= 1;
    } else {
        batt->mode=2;
    }   
    if (BattV <3000){ //otherwise it will be zero
        batt->percent =0;
    } else{
         batt->percent= ((BattV-3000)>>3);
    }


    if (batt->percent >100){ //cant have a percentage greater than 100
        batt->percent= 100;
    }
    return 0;
}

// Uses the two global variables (ports) BATT_VOLTAGE_PORT and
// BATT_STATUS_PORT to set the fields of the parameter 'batt'.  If
// BATT_VOLTAGE_PORT is negative, then battery has been wired wrong;
// no fields of 'batt' are changed and 1 is returned to indicate an
// error.  Otherwise, sets fields of batt based on reading the voltage
// value and converting to precent using the provided formula. Returns
// 0 on a successful execution with no errors. This function DOES NOT
// modify any global variables but may access global variables.
//
// CONSTRAINT: Avoids the use of the division operation as much as
// possible. Makes use of shift operations in place of division where
// possible.
//
// CONSTRAINT: Uses only integer operations. No floating point
// operations are used as the target machine does not have a FPU.
// 
// CONSTRAINT: Limit the complexity of code as much as possible. Do
// not use deeply nested conditional structures. Seek to make the code
// as short, and simple as possible. Code longer than 40 lines may be
// penalized for complexity.

int set_display_from_batt(batt_t batt, int *display){
    int arr[10]= { 0b0111111, 0b0000110, 0b1011011, 0b1001111,0b1100110,0b1101101, 0b1111101, 0b0000111, 0b1111111, 0b1101111};
    short blank= 0b0000000;
    *display=0;
    if (batt.mode==1){ //percent
        int right=batt.percent%10;//ones place
        int middle= (batt.percent/10)%10; //tens place
        int left= (batt.percent/100) %10; //hundreds place

        if (left==0){
            *display= *display | (blank << 17); //if left place is 0, display blank
        } else{
        *display = *display | (arr[left] << 17); //same as x= x| (shift)
        }

        if (middle==0 && left==0){ //if middle is a 0, display a blank
            *display= *display | (blank <<10);
        } else{
            *display = *display | (arr[middle] << 10);
        }
        *display = *display | (1<<0); //make the percent symbol appear
        *display= *display | (arr[right] << 3); //place right 


    } else{ //volt
        
        int volt= batt.mlvolts;
        if (volt<0){
            return 1;
        }
        int rightV= (volt+5)/10 %10; //add 5 to round
        int middleV= (volt/100)%10;
        int leftV = (volt/1000)%10;

        *display = *display | (arr[leftV] << 17);
        *display = *display | (arr[middleV] << 10);
        *display = *display | (arr[rightV] << 3);
        *display = *display | (1<< 2); //make the Volts symbol appear
        *display = *display | (1<<1);
    }
    int percent= batt.percent;
    //display the percentage of the battery
    if (percent > 4){
            *display = *display | (0b00001 << 24);
        } if (percent > 29){
            *display = *display | (0b00011 << 24);
        } if (percent > 49){
            *display = *display | (0b00111 << 24);
        } if (percent > 69){
            *display = *display | (0b01111 << 24);
        } if (percent > 89){
            *display = *display | (0b11111 << 24);
        }
    return 0;
}
// Alters the bits of integer pointed to by 'display' to reflect the
// data in struct param 'batt'.  Does not assume any specific bit
// pattern stored at 'display' and completely resets all bits in it on
// successfully completing.  Selects either to show Percent (mode=1) or
// Volts (mode=2). If Volts are displayed, only displays 3 digits
// rounding the lowest digit up or down appropriate to the last digit.
// Calculates each digit to display changes bits at 'display' to show
// the volts/percent according to the pattern for each digit. Modifies
// additional bits to show a decimal place for volts and a 'V' or '%'
// indicator appropriate to the mode. In both modes, places bars in
// the level display as indicated by percentage cutoffs in provided
// diagrams. This function DOES NOT modify any global variables but
// may access global variables. Always returns 0.
// 
// CONSTRAINT: Limit the complexity of code as much as possible. Do
// not use deeply nested conditional structures. Seek to make the code
// as short, and simple as possible. Code longer than 65 lines may be
// penalized for complexity.

int batt_update(){
    batt_t batt;
    int display;
    if (set_batt_from_ports(&batt) !=0 || set_display_from_batt(batt, &display) !=0){  //if theres an error, return 1
        return 1;
    }
    BATT_DISPLAY_PORT= display; //sets to display from function
    return 0;
}
// Called to update the battery meter display.  Makes use of
// set_batt_from_ports() and set_display_from_batt() to access battery
// voltage sensor then set the display. Checks these functions and if
// they indicate an error, does NOT change the display and returns 1.
// If functions succeed, modifies BATT_DISPLAY_PORT to show current
// battery level and returns 0.
// 
// CONSTRAINT: Does not allocate any heap memory as malloc() is NOT
// available on the target microcontroller.  Uses stack and global
// memory only.