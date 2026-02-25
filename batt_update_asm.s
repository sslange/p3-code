.text                           # IMPORTANT: subsequent stuff is executable
.global set_batt_from_ports
        
## ENTRY POINT FOR REQUIRED FUNCTION
set_batt_from_ports:

        cmpw $0, BATT_VOLTAGE_PORT(%rip) ## compare the battery port to 0, (check if battport<0)
        jl .LESSTHAN ##if less than 0, jump to end

        movw BATT_VOLTAGE_PORT(%rip), %dx ##assign BATT_VOLTAGE_PORT to dx
        sarw $1, %dx ##shift right one
        movw %dx, 0(%rdi) ##move BATT_VOLTAGE_PORT into BattV

        ##check if BATT_STATUS_PORT & (1<<4)
        testw $0b10000, BATT_STATUS_PORT(%rip) ##test if this is true
        jnz .PERCENTMODE ##if this is true (equals one=nz), then it is percent mode
        jmp .VOLTMODE ##else (if false) then jump to volt mode

        .CALCULATE_PERCENT:
        cmpw $3000, %dx ##if battV<3000
        jl .SET_ZERO_PERCENT ##jump to end, which sets eax to 0

        ##create (BattV-3000)>>3
        movw %dx, %cx ##store BattV in cx
        subw $3000 , %cx ## BATTV-3000
        sarw $3, %cx ##shift BATTV>>3
        movb %cl, 2(%rdi) ##batt->percent= shifted BATTV 

        .CHECKGREATER:
        cmpb $100, 2(%rdi) ##checks if percentage is greater than 100
        jle .END ##if not, then return
        movb $100, 2(%rdi) ##if greater, set percent to 100
        
        .END:
        movl $0, %eax
        ret


        ##if batt voltage port<0, set the return to be 1
        .LESSTHAN:
        movl $1, %eax ##set the return to be 1
        ret ##end

         .PERCENTMODE:
        movb $1, 3(%rdi)
        jmp .CALCULATE_PERCENT

        .VOLTMODE:
        movb $2, 3(%rdi)
        jmp .CALCULATE_PERCENT

        .SET_ZERO_PERCENT:
        movb $0, 2(%rdi) ##set percent to 0
        jmp .END ##jump to end because won't ever be greater than 100



        


        ## DON'T FORGET TO RETURN FROM FUNCTIONS




        ## assembly instructions here
        ## rdi is a (batt_t *), a pointer to batt

        ## a useful technique for this problem
        ##movq $0, %rsi
        ##movw    BATT_VOLTAGE_PORT(%rip), %si

        ##movw $57, 0(%rdi)
        # load global variable into register
        # Check the C type of the variable
        #    char / short / int / long
        # and use one of
        #    movb / movw / movl / movq 
        # and appropriately sized destination register                                            

        ## DON'T FORGET TO RETURN FROM FUNCTIONS

### Change to definint semi-global variables used with the next function 
### via the '.data' directive
.data                           # IMPORTANT: use .data directive for data section
	
my_int:                         # declare location an single int
        .int 1234               # value 1234

other_int:                      # declare another accessible via name 'other_int'
        .int 0b0101             # binary value as per C '0b' convention

my_array:                       # declare multiple ints sequentially starting at location
        .int 20                 # 'my_array' for an array. Each are spaced 4 bytes from the
        .int 0x00014            # next and can be given values using the same prefixes as 
        .int 0b11110            # are understood by gcc.
number_bit_mask_array:
        .int 0b0111111 ##zero
        .int 0b0000110 ##one
        .int 0b1011011 ##two
        .int 0b1001111 ##three
        .int 0b1100110 ##four
        .int 0b1101101 ##five
        .int 0b1111101 ##six
        .int 0b0000111 ##seven
        .int 0b1111111 ##eight
        .int 0b1101111 ##nine
        .int 0b0000000; ##blank


## WARNING: Don't forget to switch back to .text as below
## Otherwise you may get weird permission errors when executing 
.text
.global  set_display_from_batt

## ENTRY POINT FOR REQUIRED FUNCTION
set_display_from_batt:
        push %r12
        push %r13
        push %r14

        leaq number_bit_mask_array(%rip), %r13 #sets r13= &number_bit_mask_array[0]
        ##(%rax, %reg, 4), %eax

        movl %edi, %ecx #load the struct into ecx (%rdi is the first argument, but we need 32 bits so edi)
        movl %ecx, %edx # make a copy of struct so can shift to get fields
        shrl $24, %edx # edx= batt.mode
        cmpl $2, %edx
        je .CONT
        cmpl $1, %edx
        je .CONT

        .BAD_VAL:
        movl $1, %eax
        pop %r14
        pop %r13
        pop %r12
        ret

        .CONT:
        movl %ecx, %r8d ##load into new
        shrl $16, %r8d # r8d= batt.percent
        and $0xFF, %r8d

        movl %edi, %eax # eax= batt.mlvolts
        and $0xFFFF, %eax
        cmpw $0, %ax #if batt.volts<=0, do not edit display
        jl .BAD_VAL

        movl $0, (%rsi) ##second argument is display %rsi, deference as did in .c

        cmpl $1, %edx ##if batt.mode=1, percent mode
        jne .VOLTS

        .PERCENT:
        # r8d = batt.percent
        ##divide

       #getting right
       movl %r8d, %eax #numerator is batt.percent
       xorl %edx, %edx
       movl $10, %ecx #denominator is 10
       div %ecx #(batt.percent/10)
       movl %edx, %r9d # right= r9d to get mod, just use remainder (edx)


       ##getting middle
       xorl %edx, %edx #must clear the remainder
       div %ecx #same num and denom as before
       movl %edx, %r10d # middle= r10d


       ##getting left
       movl %r8d, %eax
       xorl %edx, %edx #clear remainder
       movl $100, %ecx
       div %ecx #(batt.percent/100)
       xorl %edx, %edx
       movl $10, %ecx
       div %ecx
       movl %edx, %r11d # left= r11d

        
        
        ## if left =0
        .LEFT:
        cmpl $0, %r11d
        jne .LEFT_ELSE
        movl (%rsi), %eax ##put display into eax
        movl 40(%r13), %edx # get blank (the tenth int) and put it into edx (40 because 10th*4int size)
        shll $17, %edx ##shift blank over 17
        orl %edx, %eax #bitwise or with display and blank
        movl %eax, (%rsi) #put this shift into display
        jmp .MIDDLE

        .LEFT_ELSE:
        movl (%rsi), %eax ##put display into eax
        movl (%r13, %r11, 4), %edx #get arr[left] and put it into edx (no d so will work in 64 bit mode)
        shll $17, %edx ##shift blank over 17
        orl %edx, %eax #bitwise or with display and arr[left]<<17
        movl %eax, (%rsi) #put this shift into display

        .MIDDLE:
        cmpl $0, %r10d #check if middle=0
        jne .MIDDLE_ELSE
        cmpl $0, %r11d #check if left=0
        jne .MIDDLE_ELSE #if one fails, will go into else (workaround && expression)

        movl (%rsi), %eax #put display into eax
        movl 40(%r13), %edx # get blank (the tenth int) and put it into edx (40 because 10th*4int size)
        shll $10, %edx ##shift blank over 10
        orl %edx, %eax #bitwise or with display and blank
        movl %eax, (%rsi) #put this shift into display
        jmp .PERCENT_SYMBOL

        .MIDDLE_ELSE:
        movl (%rsi), %eax ##put display into eax
        movl (%r13, %r10, 4), %edx #get arr[middle] and put it into edx (no d so will work in 64 bit mode)
        shll $10, %edx ##shift over 10
        orl %edx, %eax #bitwise or with display and arr[middle]<<10
        movl %eax, (%rsi) #put this shift into display

        .PERCENT_SYMBOL:
        movl (%rsi), %eax #put display into eax
        orl $1, %eax #bitwise or with display and 1 (do not need to shift by zero, thats just 0)
        movl %eax, (%rsi) ##store back in display

        .PLACE_RIGHT:
        movl (%rsi), %eax #put display into eax
        movl (%r13, %r9, 4), %edx #get arr[right] and put it into edx (no d so will work in 64 bit mode)
        shll $3, %edx #shift arr[right]<<3
        orl %edx, %eax #bitwise or with display and arr[left]<<3
        movl %eax, (%rsi) #put this shift into display
        jmp .PERCENT_DISPLAY



        ##otherwise volt mode

        ##movl 
        ##div 
        .VOLTS:
        #rightV

        movl %eax, %r12d ##save volt to r12d so registers dont get confused (eax is return)

        #check if negative voltage
        cmpl $0, %r12d
        jge .POS_VOLT           # jump to positive volt case if >= 0

        .POS_VOLT:

        movl %r12d, %eax
        addl $5, %eax #volt+5
        xorl %edx, %edx #get rid of old remainder
        movl $10, %ecx
        div %ecx #(volt+5)/10
        xorl %edx, %edx #get rid of old remainder
        movl $10, %ecx
        div %ecx
        movl %edx, %r14d #rightV= r14d

        #middleV
        movl %r12d, %eax #reload volts so right value
        addl $5, %eax #volt+5
        xorl %edx, %edx # get rid of old remainder
        movl $100, %ecx 
        div %ecx #volt/100
        xorl %edx, %edx #get rid of old remainder
        movl $10, %ecx
        div %ecx #(volt/100)%10
        movl %edx, %r9d #middleV= r9d

        #leftV
        movl %r12d, %eax #reload volts
        xorl %edx, %edx
        movl $1000, %ecx 
        div %ecx #volt/1000
        xorl %edx, %edx
        movl $10, %ecx 
        div %ecx #(volt/1000)%10
        movl %edx, %r10d #leftV=r10d


        .VOLT_SHIFTS:
        #shift arr[leftV]
        movl (%rsi), %eax #load display into eax
        movl (%r13, %r10, 4), %edx #put arr[leftV] into edx (r13 is the array base 0)
        shll $17, %edx 
        orl %edx, %eax
        movl %eax, (%rsi) 

        #shift arr[middleV]
        movl (%rsi), %eax
        movl (%r13, %r9, 4), %edx #arr[middleV] into edx
        shll $10, %edx
        orl %edx, %eax
        movl %eax, (%rsi)
        
        #shift arr[right]
        movl (%rsi), %eax
        movl (%r13, %r14, 4), %edx #arr[rightV] into edx
        shll $3, %edx
        orl %edx, %eax
        movl %eax, (%rsi)
        
        #volts symbol
        # *display = *display | (1<< 2); //make the Volts symbol appear
        movl (%rsi), %eax
        movl $1, %edx 
        shll $2, %edx #1<<2
        orl %edx, %eax
        movl %eax, (%rsi)  
       
       #negative symbol?
        movl $1, %edx
        shll $1, %edx #1<<1
        orl %edx, %eax
        movl %eax, (%rsi)


        ## if batt port<0, return 1
        .PERCENT_DISPLAY:
        cmpl $4, %r8d #if batt.percent>4
        jle .ENDING ##if less than 4, jump to end

        ##first bar
        movl (%rsi), %eax #put display into eax
        movl $1, %edx ##place 1 into edx
        shll $24, %edx #shift 24 places
        orl %edx, %eax #bitwise or with display and 1<<24
        movl %eax, (%rsi) #put this shift into display

        ## second bar
        cmpl $29, %r8d ##if percent>29
        jle .ENDING
        movl $3, %edx #0b00011
        shll $24, %edx #shift 24 places
        orl %edx, %eax #bitwise or with display and 3<<24
        movl %eax, (%rsi)

        ##third bar
        cmpl $49, %r8d #if percent>49
        jle .ENDING
        movl $7, %edx #0b00111
        shll $24, %edx #shift 24 places
        orl %edx, %eax #bitwise or with display and 7<<24
        movl %eax, (%rsi)

        ##fourth bar
        cmpl $69, %r8d #if percent>69
        jle .ENDING
        movl $15, %edx #0b01111
        shll $24, %edx #shift 24 places
        orl %edx, %eax #bitwise or with display and 15<<24
        movl %eax, (%rsi)

        ##fifth bar
        cmpl $89, %r8d #if percent>69
        jle .ENDING
        movl $24, %edx #0b11111
        shll $24, %edx #shift 24 places
        orl %edx, %eax #bitwise or with display and 24<<24
        movl %eax, (%rsi)



        .ENDING:
        movl %eax, (%rsi)
        movl $0, %eax
        pop %r14 #must pop in reverse order of push
        pop %r13
        pop %r12
        ret
        ## DON'T FORGET TO RETURN FROM FUNCTIONS




.global batt_update
        
## ENTRY POINT FOR REQUIRED FUNCTION
batt_update:
        subq $8, %rsp #two bytes for mvolts + one for percent + one for mode

        leaq 0(%rsp), %rdi #put batt into arg1
        call set_batt_from_ports
        cmp $0, %eax
        jne .ER

        movl 0(%rsp), %edi #4 bits so in edi
        leaq 4(%rsp), %rsi
        call set_display_from_batt
        cmp $0, %eax
        jne .ER
        
        movl 4(%rsp), %eax #move display into eax
        movl %eax, BATT_DISPLAY_PORT(%rip)
        addq $8, %rsp
        movl $0, %eax
        ret


        .ER:
        addq $8, %rsp
        movl $1, %eax
        ret
	## assembly instructions here
        ## DON'T FORGET TO RETURN FROM FUNCTIONS

