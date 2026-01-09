#ifndef IDT_H
#define IDT_H

#include "types.h"

#define IDT_ENTRIES         256
#define IDT_ENTRY_SIZE      8

#define IRQ_BASE            32
#define IRQ_KEYBOARD        (IRQ_BASE + 1)
#define IRQ_TIMER           (IRQ_BASE + 0)

#define IDT_GATE_TASK       0x5
#define IDT_GATE_INT16      0x6
#define IDT_GATE_TRAP16     0x7
#define IDT_GATE_INT32      0xE
#define IDT_GATE_TRAP32     0xF

#define IDT_FLAG_PRESENT    0x80
#define IDT_FLAG_DPL0       0x00
#define IDT_FLAG_DPL3       0x60

typedef struct s_idt_entry
{
    uint16_t    offset_low;
    uint16_t    selector;
    uint8_t     zero;
    uint8_t     type_attr;
    uint16_t    offset_high;
}   PACKED t_idt_entry;

typedef struct s_idt_ptr
{
    uint16_t    limit;
    uint32_t    base;
}   PACKED t_idt_ptr;

typedef struct s_interrupt_frame
{
    uint32_t    eip;
    uint32_t    cs;
    uint32_t    eflags;
    uint32_t    esp;
    uint32_t    ss;
}   t_interrupt_frame;

void    idt_init(void);
void    idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags);
void    idt_load(void);

extern void isr_stub_0(void);
extern void isr_stub_1(void);
extern void isr_stub_2(void);
extern void isr_stub_3(void);
extern void isr_stub_4(void);
extern void isr_stub_5(void);
extern void isr_stub_6(void);
extern void isr_stub_7(void);
extern void isr_stub_8(void);
extern void isr_stub_9(void);
extern void isr_stub_10(void);
extern void isr_stub_11(void);
extern void isr_stub_12(void);
extern void isr_stub_13(void);
extern void isr_stub_14(void);
extern void isr_stub_15(void);
extern void isr_stub_16(void);
extern void isr_stub_17(void);
extern void isr_stub_18(void);
extern void isr_stub_19(void);
extern void isr_stub_20(void);
extern void isr_stub_21(void);
extern void isr_stub_22(void);
extern void isr_stub_23(void);
extern void isr_stub_24(void);
extern void isr_stub_25(void);
extern void isr_stub_26(void);
extern void isr_stub_27(void);
extern void isr_stub_28(void);
extern void isr_stub_29(void);
extern void isr_stub_30(void);
extern void isr_stub_31(void);

extern void irq_stub_0(void);
extern void irq_stub_1(void);
extern void irq_stub_2(void);
extern void irq_stub_3(void);
extern void irq_stub_4(void);
extern void irq_stub_5(void);
extern void irq_stub_6(void);
extern void irq_stub_7(void);
extern void irq_stub_8(void);
extern void irq_stub_9(void);
extern void irq_stub_10(void);
extern void irq_stub_11(void);
extern void irq_stub_12(void);
extern void irq_stub_13(void);
extern void irq_stub_14(void);
extern void irq_stub_15(void);

extern void default_int_stub(void);

void    isr_handler(void);
void    irq_handler(uint32_t irq_num);

#endif
