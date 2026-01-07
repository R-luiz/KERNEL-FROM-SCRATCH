; **************************************************************************** ;
;                                                                              ;
;   KFS_1 - Kernel From Scratch                                                ;
;                                                                              ;
;   interrupts.asm - Interrupt Handler Stubs                                   ;
;                                                                              ;
; **************************************************************************** ;

; =============================================================================
; External C Functions
; =============================================================================

extern isr_handler
extern irq_handler

; =============================================================================
; ISR Stub Macro (for CPU exceptions)
; =============================================================================

%macro ISR_NOERRCODE 1
global isr_stub_%1
isr_stub_%1:
    cli                         ; Disable interrupts
    push byte 0                 ; Dummy error code
    push byte %1                ; Interrupt number
    jmp isr_common_stub
%endmacro

%macro ISR_ERRCODE 1
global isr_stub_%1
isr_stub_%1:
    cli                         ; Disable interrupts
    push byte %1                ; Interrupt number (error already pushed by CPU)
    jmp isr_common_stub
%endmacro

; =============================================================================
; IRQ Stub Macro (for hardware interrupts)
; =============================================================================

%macro IRQ_STUB 1
global irq_stub_%1
irq_stub_%1:
    cli
    push byte 0                 ; Dummy error code
    push byte (%1 + 32)         ; IRQ number + 32
    jmp irq_common_stub
%endmacro

; =============================================================================
; CPU Exception Handlers (0-31)
; =============================================================================

ISR_NOERRCODE 0     ; Divide by zero
ISR_NOERRCODE 1     ; Debug
ISR_NOERRCODE 2     ; Non-maskable interrupt
ISR_NOERRCODE 3     ; Breakpoint
ISR_NOERRCODE 4     ; Overflow
ISR_NOERRCODE 5     ; Bound range exceeded
ISR_NOERRCODE 6     ; Invalid opcode
ISR_NOERRCODE 7     ; Device not available
ISR_ERRCODE 8       ; Double fault (has error code)
ISR_NOERRCODE 9     ; Coprocessor segment overrun
ISR_ERRCODE 10      ; Invalid TSS (has error code)
ISR_ERRCODE 11      ; Segment not present (has error code)
ISR_ERRCODE 12      ; Stack-segment fault (has error code)
ISR_ERRCODE 13      ; General protection fault (has error code)
ISR_ERRCODE 14      ; Page fault (has error code)
ISR_NOERRCODE 15    ; Reserved
ISR_NOERRCODE 16    ; x87 floating-point exception
ISR_ERRCODE 17      ; Alignment check (has error code)
ISR_NOERRCODE 18    ; Machine check
ISR_NOERRCODE 19    ; SIMD floating-point exception
ISR_NOERRCODE 20    ; Virtualization exception
ISR_NOERRCODE 21    ; Reserved
ISR_NOERRCODE 22    ; Reserved
ISR_NOERRCODE 23    ; Reserved
ISR_NOERRCODE 24    ; Reserved
ISR_NOERRCODE 25    ; Reserved
ISR_NOERRCODE 26    ; Reserved
ISR_NOERRCODE 27    ; Reserved
ISR_NOERRCODE 28    ; Reserved
ISR_NOERRCODE 29    ; Reserved
ISR_ERRCODE 30      ; Security exception (has error code)
ISR_NOERRCODE 31    ; Reserved

; =============================================================================
; Hardware IRQ Handlers (32-47)
; =============================================================================

IRQ_STUB 0          ; Timer
IRQ_STUB 1          ; Keyboard
IRQ_STUB 2          ; Cascade
IRQ_STUB 3          ; COM2
IRQ_STUB 4          ; COM1
IRQ_STUB 5          ; LPT2
IRQ_STUB 6          ; Floppy
IRQ_STUB 7          ; LPT1
IRQ_STUB 8          ; RTC
IRQ_STUB 9          ; Free
IRQ_STUB 10         ; Free
IRQ_STUB 11         ; Free
IRQ_STUB 12         ; PS/2 Mouse
IRQ_STUB 13         ; FPU
IRQ_STUB 14         ; Primary ATA
IRQ_STUB 15         ; Secondary ATA

; =============================================================================
; Common ISR Handler
; =============================================================================

isr_common_stub:
    pusha               ; Push all general purpose registers

    mov ax, ds          ; Save data segment
    push eax

    mov ax, 0x10        ; Load kernel data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    call isr_handler    ; Call C handler

    pop eax             ; Restore data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popa                ; Restore registers
    add esp, 8          ; Clean up error code and ISR number
    sti                 ; Re-enable interrupts
    iret                ; Return from interrupt

; =============================================================================
; Common IRQ Handler
; =============================================================================

irq_common_stub:
    pusha               ; Push all general purpose registers

    mov ax, ds          ; Save data segment
    push eax

    mov ax, 0x10        ; Load kernel data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    call irq_handler    ; Call C handler

    pop eax             ; Restore data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popa                ; Restore registers
    add esp, 8          ; Clean up error code and IRQ number
    sti                 ; Re-enable interrupts
    iret                ; Return from interrupt
