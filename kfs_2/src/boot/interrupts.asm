extern isr_handler
extern irq_handler
%macro ISR_NOERRCODE 1
global isr_stub_%1
isr_stub_%1:
    cli                         
    push dword 0                
    push dword %1               
    jmp isr_common_stub
%endmacro
%macro ISR_ERRCODE 1
global isr_stub_%1
isr_stub_%1:
    cli                         
    push dword %1               
    jmp isr_common_stub
%endmacro
%macro IRQ_STUB 1
global irq_stub_%1
irq_stub_%1:
    cli
    push dword 0                
    push dword (%1 + 32)        
    jmp irq_common_stub
%endmacro
ISR_NOERRCODE 0     
ISR_NOERRCODE 1     
ISR_NOERRCODE 2     
ISR_NOERRCODE 3     
ISR_NOERRCODE 4     
ISR_NOERRCODE 5     
ISR_NOERRCODE 6     
ISR_NOERRCODE 7     
ISR_ERRCODE 8       
ISR_NOERRCODE 9     
ISR_ERRCODE 10      
ISR_ERRCODE 11      
ISR_ERRCODE 12      
ISR_ERRCODE 13      
ISR_ERRCODE 14      
ISR_NOERRCODE 15    
ISR_NOERRCODE 16    
ISR_ERRCODE 17      
ISR_NOERRCODE 18    
ISR_NOERRCODE 19    
ISR_NOERRCODE 20    
ISR_NOERRCODE 21    
ISR_NOERRCODE 22    
ISR_NOERRCODE 23    
ISR_NOERRCODE 24    
ISR_NOERRCODE 25    
ISR_NOERRCODE 26    
ISR_NOERRCODE 27    
ISR_NOERRCODE 28    
ISR_NOERRCODE 29    
ISR_ERRCODE 30      
ISR_NOERRCODE 31    
IRQ_STUB 0          
IRQ_STUB 1          
IRQ_STUB 2          
IRQ_STUB 3          
IRQ_STUB 4          
IRQ_STUB 5          
IRQ_STUB 6          
IRQ_STUB 7          
IRQ_STUB 8          
IRQ_STUB 9          
IRQ_STUB 10         
IRQ_STUB 11         
IRQ_STUB 12         
IRQ_STUB 13         
IRQ_STUB 14         
IRQ_STUB 15         
isr_common_stub:
    pusha               
    xor eax, eax        
    mov ax, ds          
    push eax            
    mov ax, 0x10        
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    call isr_handler    
    pop eax             
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    popa                
    add esp, 8          
    iret                
irq_common_stub:
    pusha               
    xor eax, eax        
    mov ax, ds          
    push eax            
    mov ax, 0x10        
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov eax, [esp + 36]
    push eax
    call irq_handler    
    add esp, 4          
    pop eax             
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    popa                
    add esp, 8          
    iret                
global default_int_stub
default_int_stub:
    iret                
