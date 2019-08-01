; Credit: https://www.embeddedrelated.com/showarticle/528.php
; Adapted to 16mhz by RouNdeL
#define __SFR_OFFSET 0
#include <avr/io.h>

; extern void output_grb_pc2(uint8_t * ptr, uint16_t count)
;
; r18 = data byte
; r19 = 7-bit count
; r20 = 1 output
; r21 = 0 output
; r22 = SREG save
; r24:25 = 16-bit count
; r26:27 (X) = data pointer

.equ      OUTBIT,   2


.global output_grb_pc2
output_grb_pc2:
         movw   r26, r24      ;r26:27 = X = p_buf
         movw   r24, r22      ;r24:25 = count
         in     r22, SREG     ;save SREG (global int state)
         cli                  ;no interrupts from here on, we're cycle-counting
         in     r20, PORTC
         ori    r20, (1<<OUTBIT)         ;our '1' output
         in     r21, PORTC
         andi   r21, ~(1<<OUTBIT)        ;our '0' output
         ldi    r19, 7        ;7 bit counter (8th bit is different)
         ld     r18,X+        ;get first data byte
loop1:
         out    PORTC, r20    ; 1   +0 start of a bit pulse
         nop                  ; 1   +1
         lsl    r18           ; 1   +2 next bit into C, MSB first
         nop                  ; 1   +3
         brcs   L1            ; 1/2 +4 branch if 1
         nop                  ; 1   +5
         out    PORTC, r21    ; 1   +6 end hi for '0' bit (6 clocks hi)
         nop                  ; 1   +7
         nop                  ; 1   +8
         bst    r18, 7        ; 1   +9 save last bit of data for fast branching
         nop                  ; 1   +10
         subi   r19, 1        ; 1   +11 how many more bits for this byte?
         nop                  ; 1   +12
         breq   bit8          ; 1/2 +13 last bit, do differently
         nop                  ; 1   +14
         nop                  ; 1   +15
         nop                  ; 1   +16
         nop                  ; 1   +17
         rjmp   loop1         ; 2   +18, 10 total for 0 bit
L1:
         nop                  ; 1   +6
         nop                  ; 1   +7
         nop                  ; 1   +8
         bst    r18, 7        ; 1   +9 save last bit of data for fast branching
         nop                  ; 1   +10
         subi   r19, 1        ; 1   +11 how many more bits for this byte
         nop                  ; 1   +12
         out    PORTC, r21    ; 1   +13 end hi for '1' bit (14 clocks hi)
         nop                  ; 1   +14
         nop                  ; 1   +15
         nop                  ; 1   +16
         nop                  ; 1   +17
         brne   loop1         ; 2/1 +18 10 total for 1 bit (fall thru if last bit)
bit8:
         ldi    r19, 7        ; 1   +19 bit count for next byte
         out    PORTC, r20    ; 1   +0 start of a bit pulse
         nop                  ; 1   +1
         brts   L2            ; 1/2 +2 branch if last bit is a 1
         nop                  ; 1   +3
         nop                  ; 1   +4
         nop                  ; 1   +5
         out    PORTC, r21    ; 1   +6 end hi for '0' bit (6 clocks hi)
         nop                  ; 1   +7
         ld     r18, X+       ; 2   +8 fetch next byte
         nop                  ; 1   +10
         sbiw   r24, 1        ; 2   +11 dec byte counter
         nop                  ; 1   +13
         nop                  ; 1   +14
         nop                  ; 1   +15
         nop                  ; 1   +16
         nop                  ; 1   +17
         brne   loop1         ; 2   +18 loop back or return
         out    SREG, r22     ; restore global int flag
         ret
L2:
         ld     r18, X+       ; 2   +4 fetch next byte
         nop                  ; 1   +6
         sbiw   r24, 1        ; 2   +7 dec byte counter
         nop                  ; 1   +9
         nop                  ; 1   +10
         nop                  ; 1   +11
         nop                  ; 1   +12
         nop                  ; 1   +13
         out    PORTC, r21    ; 1   +14 end hi for '1' bit (14 clocks hi)
         nop                  ; 1   +15
         nop                  ; 1   +16
         brne   loop1         ; 2   +18 loop back or return
         out    SREG, r22     ; restore global int flag
         ret