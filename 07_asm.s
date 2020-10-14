

;-----------------------------------------------------------------------------
; Device includes, defines, and assembler directives
;-----------------------------------------------------------------------------

 .def get_PSP
 .def set_PSP
 .def pushR4_11
 .def popR4_11
;-----------------------------------------------------------------------------
; Register values and large immediate values
;-----------------------------------------------------------------------------

.thumb
.const

;------------ -----------------------------------------------------------------
; Subroutines
;-----------------------------------------------------------------------------

.text

set_PSP:
				MOV R4, #2
				MSR CONTROL, R4
				MSR PSP, R0
   				BX LR

get_PSP:
			   MRS    R0,PSP
			   BX     LR                     ; return from subroutine

pushR4_11:
   			  MRS R0, PSP
    		  SUB R0,R0,#4
              STR R11,[R0]
              SUB R0,R0,#4
              STR R10,[R0]
              SUB R0,R0,#4
              STR R9,[R0]
              SUB R0,R0,#4
              STR R8,[R0]
              SUB R0,R0,#4
              STR R7,[R0]
              SUB R0,R0,#4
              STR R6,[R0]
              SUB R0,R0,#4
              STR R5,[R0]
              SUB R0,R0,#4
              STR R4,[R0]
              MSR PSP, R0
              BX LR

popR4_11:
              LDR R4,[R0]
              ADD R0,R0,#4
              LDR R5,[R0]
              ADD R0,R0,#4
              LDR R6,[R0]
              ADD R0,R0,#4
              LDR R7,[R0]
              ADD R0,R0,#4
              LDR R8,[R0]
              ADD R0,R0,#4
              LDR R9,[R0]
              ADD R0,R0,#4
              LDR R10,[R0]
              ADD R0,R0,#4
              LDR R11,[R0]
              ADD R0,R0,#4
   			  MSR PSP, R0
              BX LR

.endm
