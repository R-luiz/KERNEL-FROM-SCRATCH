; **************************************************************************** ;
;                                                                              ;
;   KFS_2 - Kernel From Scratch                                                ;
;                                                                              ;
;   boot.asm - Multiboot Header and Entry Point                                ;
;                                                                              ;
;   This file contains:                                                        ;
;   - Multiboot header (for GRUB compatibility)                                ;
;   - Initial GDT for boot (temporary, will be replaced at 0x800)              ;
;   - Stack initialization                                                     ;
;   - gdt_flush function for loading new GDT from C                            ;
;   - Jump to kernel_main (C code)                                             ;
;                                                                              ;
; **************************************************************************** ;

; =============================================================================
; Multiboot Header Constants
; =============================================================================
; See: https://www.gnu.org/software/grub/manual/multiboot/multiboot.html

MBOOT_PAGE_ALIGN    equ 1 << 0          ; Align loaded modules on page boundaries
MBOOT_MEM_INFO      equ 1 << 1          ; Provide memory map
MBOOT_HEADER_MAGIC  equ 0x1BADB002      ; Magic number for bootloader
MBOOT_HEADER_FLAGS  equ MBOOT_PAGE_ALIGN | MBOOT_MEM_INFO
MBOOT_CHECKSUM      equ -(MBOOT_HEADER_MAGIC + MBOOT_HEADER_FLAGS)

; =============================================================================
; Stack Configuration
; =============================================================================

STACK_SIZE          equ 16384           ; 16 KB stack size

; =============================================================================
; GDT Selectors (must match gdt.h)
; =============================================================================

GDT_KERNEL_CODE     equ 0x08            ; Kernel code segment selector
GDT_KERNEL_DATA     equ 0x10            ; Kernel data segment selector

; =============================================================================
; Multiboot Header Section
; =============================================================================
; This MUST be in the first 8KB of the kernel binary
; GRUB scans for this signature to identify valid kernels

section .multiboot
align 4
    dd MBOOT_HEADER_MAGIC               ; Magic number
    dd MBOOT_HEADER_FLAGS               ; Flags
    dd MBOOT_CHECKSUM                   ; Checksum (magic + flags + checksum = 0)

; =============================================================================
; BSS Section - Uninitialized Data
; =============================================================================
; Stack grows downward, so stack_top is at the end of reserved space

section .bss
align 16
stack_bottom:
    resb STACK_SIZE                     ; Reserve 16KB for stack
stack_top:

; =============================================================================
; Initial GDT - Temporary for Boot
; =============================================================================
; This GDT is used only during initial boot. The kernel will set up
; the proper GDT at address 0x800 and reload it via gdt_flush().
;
; Layout:
;   0x00: Null descriptor (required)
;   0x08: Code segment (ring 0, execute/read, 4GB flat)
;   0x10: Data segment (ring 0, read/write, 4GB flat)

section .rodata
align 16
boot_gdt_start:
    ; Null descriptor (entry 0)
    dq 0x0000000000000000

    ; Code segment descriptor (entry 1, selector 0x08)
    ; Base=0, Limit=0xFFFFF, Access=0x9A, Flags=0xC
    dw 0xFFFF       ; Limit bits 0-15
    dw 0x0000       ; Base bits 0-15
    db 0x00         ; Base bits 16-23
    db 0x9A         ; Access: present, ring0, code, exec/read
    db 0xCF         ; Flags (4KB gran, 32-bit) + Limit bits 16-19
    db 0x00         ; Base bits 24-31

    ; Data segment descriptor (entry 2, selector 0x10)
    ; Base=0, Limit=0xFFFFF, Access=0x92, Flags=0xC
    dw 0xFFFF       ; Limit bits 0-15
    dw 0x0000       ; Base bits 0-15
    db 0x00         ; Base bits 16-23
    db 0x92         ; Access: present, ring0, data, read/write
    db 0xCF         ; Flags (4KB gran, 32-bit) + Limit bits 16-19
    db 0x00         ; Base bits 24-31
boot_gdt_end:

boot_gdt_descriptor:
    dw boot_gdt_end - boot_gdt_start - 1  ; GDT size (limit)
    dd boot_gdt_start                      ; GDT base address

; =============================================================================
; Text Section - Executable Code
; =============================================================================

section .text
global _start                           ; Export entry point for linker
global gdt_flush                        ; Export for C code to reload GDT
extern kernel_main                      ; Import C function

; -----------------------------------------------------------------------------
; _start - Kernel Entry Point
; -----------------------------------------------------------------------------
; Called by GRUB after loading the kernel
; At this point:
;   - CPU is in 32-bit protected mode
;   - Interrupts are disabled
;   - Paging is disabled
;   - EAX contains multiboot magic number (0x2BADB002)
;   - EBX contains pointer to multiboot information structure
; -----------------------------------------------------------------------------

_start:
    ; Disable interrupts (should already be disabled, but be safe)
    cli

    ; Save multiboot parameters before we clobber registers
    mov edi, eax                        ; Save magic number
    mov esi, ebx                        ; Save multiboot info pointer

    ; Load initial boot GDT
    lgdt [boot_gdt_descriptor]

    ; Far jump to reload CS with our code segment selector
    jmp GDT_KERNEL_CODE:.reload_segments

.reload_segments:
    ; Reload data segment registers with our data segment selector
    mov ax, GDT_KERNEL_DATA
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Set up the stack
    mov esp, stack_top

    ; Clear EBP for stack traces
    xor ebp, ebp

    ; Push multiboot info for potential future use
    push esi                            ; Multiboot info pointer
    push edi                            ; Multiboot magic number

    ; Call the kernel main function
    ; kernel_main will call gdt_init() to set up GDT at 0x800
    call kernel_main

    ; If kernel_main returns (it shouldn't), halt the CPU
.hang:
    cli
    hlt
    jmp .hang

; -----------------------------------------------------------------------------
; gdt_flush - Load new GDT and reload segment registers
; -----------------------------------------------------------------------------
; Called from C: void gdt_flush(uint32_t gdt_ptr);
;
; Parameter:
;   [esp+4] = pointer to gdt_ptr structure (6 bytes: limit + base)
;
; This function:
;   1. Loads the new GDT using LGDT instruction
;   2. Performs a far jump to reload CS
;   3. Reloads all data segment registers
; -----------------------------------------------------------------------------

gdt_flush:
    ; Get the GDT pointer from the stack
    mov eax, [esp + 4]

    ; Load the new GDT
    lgdt [eax]

    ; Far jump to reload CS with kernel code segment
    ; This flushes the instruction pipeline and loads CS
    jmp GDT_KERNEL_CODE:.gdt_flush_reload

.gdt_flush_reload:
    ; Reload all data segment registers with kernel data segment
    mov ax, GDT_KERNEL_DATA
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ret

; =============================================================================
; End of boot.asm
; =============================================================================
