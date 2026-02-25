### test_batt_update_asm.s: Contains wrapper functions which are
### used to init registers to known "garbage" values before calling a
### student-written function. This helps force out "used a random
### register" problems that will have variable behavior on different
### machines.

.text
.global CALL_set_batt_from_ports
.type	CALL_set_batt_from_ports, @function
CALL_set_batt_from_ports:
        ## %rdi is a pointer argument
        .cfi_startproc

        ## 8 bytes on stack at the start for return address
        ## 6 * 8 bytes pushed
        ## need an addtional 8 bytes to align the stack for a call
        subq $56, %rsp

        ## changes to the rsp require annotation to allow debuggers to
        ## know where the previous stack frame starts. after the above
        ## extension of the stack, the previous stack frame is 64
        ## bytes above rsp
        .cfi_def_cfa_offset 64

        ## save 6 callee save registers except the stack pointer
        movq %rbx,  0(%rsp)
        movq %rbp,  8(%rsp)
        movq %r12, 16(%rsp)
        movq %r13, 24(%rsp)
        movq %r14, 32(%rsp)
        movq %r15, 40(%rsp)

        ## initialize all registers to ensure they have "garbage"
        ## values in them
        movabsq $0xBADBADBADBAD0000, %rax
        movabsq $0xBADBADBADBAD0001, %rbx
        movabsq $0xBADBADBADBAD0002, %rcx
        movabsq $0xBADBADBADBAD0003, %rdx
        movabsq $0xBADBADBADBAD0004, %rsi
        ## movabsq $0xBADBADBADBAD0005, %rdi  # pointer to struct to fill
        ## movabsq $0xBADBADBADBAD0006, %rsp
        movabsq $0xBADBADBADBAD0007, %rbp
        movabsq $0xBADBADBADBAD0008, %r8
        movabsq $0xBADBADBADBAD0009, %r9
        movabsq $0xBADBADBADBAD000A, %r10
        movabsq $0xBADBADBADBAD000B, %r11
        movabsq $0xBADBADBADBAD000C, %r12
        movabsq $0xBADBADBADBAD000D, %r13
        movabsq $0xBADBADBADBAD000E, %r14
        movabsq $0xBADBADBADBAD000F, %r15

        ## call target function
        call set_batt_from_ports

        ## store callee regs to check them later
        call get_callee_regs
        
        ## restore 6 callee registers
        movq  0(%rsp), %rbx
        movq  8(%rsp), %rbp
        movq 16(%rsp), %r12
        movq 24(%rsp), %r13
        movq 32(%rsp), %r14
        movq 40(%rsp), %r15

        addq $56, %rsp
	.cfi_def_cfa_offset 8   # annotate another rsp change for debugger
        
        ## rax has the return value from the function
        ret                     
.cfi_endproc
.size CALL_set_batt_from_ports,.-CALL_set_batt_from_ports

## .global CALL_batt_display_special
## .type	CALL_batt_display_special, @function
## CALL_batt_display_special:
##         ## %rdi is packed struct arg
##         ## %rsi is pointer arg
##         .cfi_startproc
##         subq $56, %rsp          # see first function for details
##         .cfi_def_cfa_offset 64

##         ## save 6 callee save registers except the stack pointer
##         movq %rbx,  0(%rsp)
##         movq %rbp,  8(%rsp)
##         movq %r12, 16(%rsp)
##         movq %r13, 24(%rsp)
##         movq %r14, 32(%rsp)
##         movq %r15, 40(%rsp)

##         ## initialize all registers to ensure they have "garbage"
##         ## values in them
##         movabsq $0xBADBADBADBAD0000, %rax
##         movabsq $0xBADBADBADBAD0001, %rbx
##         movabsq $0xBADBADBADBAD0002, %rcx
##         ## movabsq $0xBADBADBADBAD0003, %rdx  # pointer to display variable
##         ## movabsq $0xBADBADBADBAD0004, %rsi  # later fields of tod_t struct
##         ## movabsq $0xBADBADBADBAD0005, %rdi  # first fields of tod_t struct
##         ## movabsq $0xBADBADBADBAD0006, %rsp
##         movabsq $0xBADBADBADBAD0007, %rbp
##         movabsq $0xBADBADBADBAD0008, %r8
##         movabsq $0xBADBADBADBAD0009, %r9
##         movabsq $0xBADBADBADBAD000A, %r10
##         movabsq $0xBADBADBADBAD000B, %r11
##         movabsq $0xBADBADBADBAD000C, %r12
##         movabsq $0xBADBADBADBAD000D, %r13
##         movabsq $0xBADBADBADBAD000E, %r14
##         movabsq $0xBADBADBADBAD000F, %r15

##         ## call the target function
##         call batt_display_special

##         ## store callee regs to check them later
##         call get_callee_regs

##         ## restore 6 callee registers
##         movq  0(%rsp), %rbx
##         movq  8(%rsp), %rbp
##         movq 16(%rsp), %r12
##         movq 24(%rsp), %r13
##         movq 32(%rsp), %r14
##         movq 40(%rsp), %r15

##         addq $56, %rsp
## 	.cfi_def_cfa_offset 8   # annotate another rsp change for debugger

##         ## rax has the return value from the function
##         ret                     
## .cfi_endproc
## .size CALL_batt_display_special,.-CALL_batt_display_special

.global CALL_set_display_from_batt
.type	CALL_set_display_from_batt, @function
CALL_set_display_from_batt:
        ## %rdi is packed struct arg
        ## %rsi is pointer arg
        .cfi_startproc
        subq $56, %rsp          # see first function for details
        .cfi_def_cfa_offset 64

        ## save 6 callee save registers except the stack pointer
        movq %rbx,  0(%rsp)
        movq %rbp,  8(%rsp)
        movq %r12, 16(%rsp)
        movq %r13, 24(%rsp)
        movq %r14, 32(%rsp)
        movq %r15, 40(%rsp)

        ## initialize all registers to ensure they have "garbage"
        ## values in them
        movabsq $0xBADBADBADBAD0000, %rax
        movabsq $0xBADBADBADBAD0001, %rbx
        movabsq $0xBADBADBADBAD0002, %rcx
        ## movabsq $0xBADBADBADBAD0003, %rdx  # pointer to display variable
        ## movabsq $0xBADBADBADBAD0004, %rsi  # later fields of tod_t struct
        ## movabsq $0xBADBADBADBAD0005, %rdi  # first fields of tod_t struct
        ## movabsq $0xBADBADBADBAD0006, %rsp
        movabsq $0xBADBADBADBAD0007, %rbp
        movabsq $0xBADBADBADBAD0008, %r8
        movabsq $0xBADBADBADBAD0009, %r9
        movabsq $0xBADBADBADBAD000A, %r10
        movabsq $0xBADBADBADBAD000B, %r11
        movabsq $0xBADBADBADBAD000C, %r12
        movabsq $0xBADBADBADBAD000D, %r13
        movabsq $0xBADBADBADBAD000E, %r14
        movabsq $0xBADBADBADBAD000F, %r15

        ## call the target function
        call set_display_from_batt

        ## store callee regs to check them later
        call get_callee_regs

        ## restore 6 callee registers
        movq  0(%rsp), %rbx
        movq  8(%rsp), %rbp
        movq 16(%rsp), %r12
        movq 24(%rsp), %r13
        movq 32(%rsp), %r14
        movq 40(%rsp), %r15

        addq $56, %rsp
	.cfi_def_cfa_offset 8   # annotate another rsp change for debugger

        ## rax has the return value from the function
        ret                     
.cfi_endproc
.size   CALL_set_display_from_batt,.-CALL_set_display_from_batt


.global CALL_batt_update
.type	CALL_batt_update, @function
CALL_batt_update:     
        ## no arguments
        .cfi_startproc
        subq $56, %rsp          # see first function for details
        .cfi_def_cfa_offset 64

        ## save 6 callee save registers except the stack pointer
        movq %rbx,  0(%rsp)
        movq %rbp,  8(%rsp)
        movq %r12, 16(%rsp)
        movq %r13, 24(%rsp)
        movq %r14, 32(%rsp)
        movq %r15, 40(%rsp)

        ## initialize all registers to ensure they have "garbage"
        ## values in them
        movabsq $0xBADBADBADBAD0000, %rax
        movabsq $0xBADBADBADBAD0001, %rbx
        movabsq $0xBADBADBADBAD0002, %rcx
        movabsq $0xBADBADBADBAD0003, %rdx
        movabsq $0xBADBADBADBAD0004, %rsi
        movabsq $0xBADBADBADBAD0005, %rdi
        ## movabsq $0xBADBADBADBAD0006, %rsp
        movabsq $0xBADBADBADBAD0007, %rbp
        movabsq $0xBADBADBADBAD0008, %r8
        movabsq $0xBADBADBADBAD0009, %r9
        movabsq $0xBADBADBADBAD000A, %r10
        movabsq $0xBADBADBADBAD000B, %r11
        movabsq $0xBADBADBADBAD000C, %r12
        movabsq $0xBADBADBADBAD000D, %r13
        movabsq $0xBADBADBADBAD000E, %r14
        movabsq $0xBADBADBADBAD000F, %r15

        ## call the target function
        call batt_update

        ## store callee regs to check them later
        call get_callee_regs

        ## restore 6 callee registers
        movq  0(%rsp), %rbx
        movq  8(%rsp), %rbp
        movq 16(%rsp), %r12
        movq 24(%rsp), %r13
        movq 32(%rsp), %r14
        movq 40(%rsp), %r15

        addq $56, %rsp
	.cfi_def_cfa_offset 8   # annotate another rsp change for debugger

        ## rax has the return value from the function
        ret                     
.cfi_endproc
.size CALL_batt_update, .-CALL_batt_update

### store the callee register values in an array to later check that
### they have not changed
get_callee_regs:
        .cfi_startproc
        leaq callee_reg_vals_actual(%rip), %rdi
        movq %rbx,  0(%rdi) 
        movq %rbp,  8(%rdi) 
        movq %r12, 16(%rdi) 
        movq %r13, 24(%rdi) 
        movq %r14, 32(%rdi) 
        movq %r15, 40(%rdi) 
        ret
	.cfi_endproc
