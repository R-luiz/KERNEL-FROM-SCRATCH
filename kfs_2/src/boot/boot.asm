MBOOT_PAGE_ALIGN    equ 1 << 0          
MBOOT_MEM_INFO      equ 1 << 1          
MBOOT_HEADER_MAGIC  equ 0x1BADB002      
MBOOT_HEADER_FLAGS  equ MBOOT_PAGE_ALIGN | MBOOT_MEM_INFO
MBOOT_CHECKSUM      equ -(MBOOT_HEADER_MAGIC + MBOOT_HEADER_FLAGS)
STACK_SIZE          equ 16384           
GDT_KERNEL_CODE     equ 0x08            
GDT_KERNEL_DATA     equ 0x10            
section .multiboot
align 4
    dd MBOOT_HEADER_MAGIC               
    dd MBOOT_HEADER_FLAGS               
    dd MBOOT_CHECKSUM                   
section .bss
align 16
stack_bottom:
    resb STACK_SIZE                     
stack_top:
section .rodata
align 16
boot_gdt_start:
    dq 0x0000000000000000
    dw 0xFFFF       
    dw 0x0000       
    db 0x00         
    db 0x9A         
    db 0xCF         
    db 0x00         
    dw 0xFFFF       
    dw 0x0000       
    db 0x00         
    db 0x92         
    db 0xCF         
    db 0x00         
boot_gdt_end:
boot_gdt_descriptor:
    dw boot_gdt_end - boot_gdt_start - 1  
    dd boot_gdt_start                      
section .text
global _start                           
global gdt_flush                        
extern kernel_main                      
_start:
    cli
    mov edi, eax                        
    mov esi, ebx                        
    lgdt [boot_gdt_descriptor]
    jmp GDT_KERNEL_CODE:.reload_segments
.reload_segments:
    mov ax, GDT_KERNEL_DATA
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, stack_top
    xor ebp, ebp
    push esi                            
    push edi                            
    call kernel_main
.hang:
    cli
    hlt
    jmp .hang
gdt_flush:
    mov eax, [esp + 4]
    lgdt [eax]
    jmp GDT_KERNEL_CODE:.gdt_flush_reload
.gdt_flush_reload:
    mov ax, GDT_KERNEL_DATA
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    ret
