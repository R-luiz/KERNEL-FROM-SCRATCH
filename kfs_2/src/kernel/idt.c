#include "../include/idt.h"
#include "../lib/string.h"

static t_idt_entry  g_idt[IDT_ENTRIES] ALIGNED(16);
static t_idt_ptr    g_idtp;

void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags)
{

    g_idt[num].offset_low = (uint16_t)(base & 0xFFFF);
    g_idt[num].offset_high = (uint16_t)((base >> 16) & 0xFFFF);
    g_idt[num].selector = sel;
    g_idt[num].zero = 0;
    g_idt[num].type_attr = flags;
}

void idt_init(void)
{
    uint16_t    i;
    uint8_t     flags;


    k_memset(g_idt, 0, sizeof(g_idt));


    g_idtp.limit = (uint16_t)(sizeof(g_idt) - 1);
    g_idtp.base = (uint32_t)&g_idt;


    flags = (uint8_t)(IDT_FLAG_PRESENT | IDT_FLAG_DPL0 | IDT_GATE_INT32);



    idt_set_gate(0, (uint32_t)isr_stub_0, 0x08, flags);
    idt_set_gate(1, (uint32_t)isr_stub_1, 0x08, flags);
    idt_set_gate(2, (uint32_t)isr_stub_2, 0x08, flags);
    idt_set_gate(3, (uint32_t)isr_stub_3, 0x08, flags);
    idt_set_gate(4, (uint32_t)isr_stub_4, 0x08, flags);
    idt_set_gate(5, (uint32_t)isr_stub_5, 0x08, flags);
    idt_set_gate(6, (uint32_t)isr_stub_6, 0x08, flags);
    idt_set_gate(7, (uint32_t)isr_stub_7, 0x08, flags);
    idt_set_gate(8, (uint32_t)isr_stub_8, 0x08, flags);
    idt_set_gate(9, (uint32_t)isr_stub_9, 0x08, flags);
    idt_set_gate(10, (uint32_t)isr_stub_10, 0x08, flags);
    idt_set_gate(11, (uint32_t)isr_stub_11, 0x08, flags);
    idt_set_gate(12, (uint32_t)isr_stub_12, 0x08, flags);
    idt_set_gate(13, (uint32_t)isr_stub_13, 0x08, flags);
    idt_set_gate(14, (uint32_t)isr_stub_14, 0x08, flags);
    idt_set_gate(15, (uint32_t)isr_stub_15, 0x08, flags);
    idt_set_gate(16, (uint32_t)isr_stub_16, 0x08, flags);
    idt_set_gate(17, (uint32_t)isr_stub_17, 0x08, flags);
    idt_set_gate(18, (uint32_t)isr_stub_18, 0x08, flags);
    idt_set_gate(19, (uint32_t)isr_stub_19, 0x08, flags);
    idt_set_gate(20, (uint32_t)isr_stub_20, 0x08, flags);
    idt_set_gate(21, (uint32_t)isr_stub_21, 0x08, flags);
    idt_set_gate(22, (uint32_t)isr_stub_22, 0x08, flags);
    idt_set_gate(23, (uint32_t)isr_stub_23, 0x08, flags);
    idt_set_gate(24, (uint32_t)isr_stub_24, 0x08, flags);
    idt_set_gate(25, (uint32_t)isr_stub_25, 0x08, flags);
    idt_set_gate(26, (uint32_t)isr_stub_26, 0x08, flags);
    idt_set_gate(27, (uint32_t)isr_stub_27, 0x08, flags);
    idt_set_gate(28, (uint32_t)isr_stub_28, 0x08, flags);
    idt_set_gate(29, (uint32_t)isr_stub_29, 0x08, flags);
    idt_set_gate(30, (uint32_t)isr_stub_30, 0x08, flags);
    idt_set_gate(31, (uint32_t)isr_stub_31, 0x08, flags);


    idt_set_gate(32, (uint32_t)irq_stub_0, 0x08, flags);
    idt_set_gate(33, (uint32_t)irq_stub_1, 0x08, flags);
    idt_set_gate(34, (uint32_t)irq_stub_2, 0x08, flags);
    idt_set_gate(35, (uint32_t)irq_stub_3, 0x08, flags);
    idt_set_gate(36, (uint32_t)irq_stub_4, 0x08, flags);
    idt_set_gate(37, (uint32_t)irq_stub_5, 0x08, flags);
    idt_set_gate(38, (uint32_t)irq_stub_6, 0x08, flags);
    idt_set_gate(39, (uint32_t)irq_stub_7, 0x08, flags);
    idt_set_gate(40, (uint32_t)irq_stub_8, 0x08, flags);
    idt_set_gate(41, (uint32_t)irq_stub_9, 0x08, flags);
    idt_set_gate(42, (uint32_t)irq_stub_10, 0x08, flags);
    idt_set_gate(43, (uint32_t)irq_stub_11, 0x08, flags);
    idt_set_gate(44, (uint32_t)irq_stub_12, 0x08, flags);
    idt_set_gate(45, (uint32_t)irq_stub_13, 0x08, flags);
    idt_set_gate(46, (uint32_t)irq_stub_14, 0x08, flags);
    idt_set_gate(47, (uint32_t)irq_stub_15, 0x08, flags);


    i = 48;
    while (i < IDT_ENTRIES)
    {
        idt_set_gate((uint8_t)i, (uint32_t)default_int_stub, 0x08, flags);
        i++;
    }


    idt_load();
}

void idt_load(void)
{
    __asm__ volatile ("lidt %0" : : "m"(g_idtp));
}
