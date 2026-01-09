#include "shell.h"
#include "stack.h"
#include "gdt.h"
#include "keyboard.h"
#include "vtty.h"
#include "vga.h"
#include "types.h"

extern size_t   k_strlen(const char *s);
extern int      k_strcmp(const char *s1, const char *s2);
extern int      k_strncmp(const char *s1, const char *s2, size_t n);
extern char     *k_strcpy(char *dest, const char *src);
extern void     *k_memset(void *s, int c, size_t n);

extern void printk(const char *fmt, ...);

static char     g_cmd_buffer[SHELL_CMD_MAX_LEN];
static size_t   g_cmd_pos = 0;

static char     g_arg_buffer[SHELL_CMD_MAX_LEN];
static char     *g_argv[SHELL_MAX_ARGS];

static const t_shell_cmd g_commands[] = {
    {"help",    "Display this help message",            cmd_help},
    {"stack",   "Print kernel stack dump",              cmd_stack},
    {"gdt",     "Display GDT entries",                  cmd_gdt},
    {"regs",    "Display CPU registers",                cmd_regs},
    {"clear",   "Clear the screen",                     cmd_clear},
    {"info",    "Display kernel information",           cmd_info},
    {"reboot",  "Reboot the system",                    cmd_reboot},
    {"halt",    "Halt the CPU",                         cmd_halt},
    {NULL,      NULL,                                   NULL}
};

static int parse_cmdline(const char *cmdline)
{
    int     argc = 0;
    size_t  i = 0;
    size_t  j = 0;
    bool_t  in_word = FALSE;


    k_strcpy(g_arg_buffer, cmdline);


    while (g_arg_buffer[i] != '\0' && argc < SHELL_MAX_ARGS)
    {

        if (g_arg_buffer[i] == ' ' || g_arg_buffer[i] == '\t')
        {
            if (in_word)
            {

                g_arg_buffer[j] = '\0';
                j++;
                in_word = FALSE;
            }
            i++;
            continue;
        }


        if (!in_word)
        {
            g_argv[argc] = &g_arg_buffer[j];
            argc++;
            in_word = TRUE;
        }


        g_arg_buffer[j] = g_arg_buffer[i];
        j++;
        i++;
    }


    if (in_word)
    {
        g_arg_buffer[j] = '\0';
    }

    return argc;
}

static const t_shell_cmd *find_command(const char *name)
{
    int i;

    for (i = 0; g_commands[i].name != NULL; i++)
    {
        if (k_strcmp(g_commands[i].name, name) == 0)
        {
            return &g_commands[i];
        }
    }
    return NULL;
}

void    shell_init(void)
{
    k_memset(g_cmd_buffer, 0, SHELL_CMD_MAX_LEN);
    g_cmd_pos = 0;

    printk("\n");
    printk("KFS-2 Shell v1.0\n");
    printk("Type 'help' for available commands.\n\n");
}

void    shell_prompt(void)
{
    printk(SHELL_PROMPT);
}

int     shell_execute(const char *cmdline)
{
    int                 argc;
    const t_shell_cmd   *cmd;


    if (cmdline == NULL || cmdline[0] == '\0')
    {
        return 0;
    }


    argc = parse_cmdline(cmdline);
    if (argc == 0)
    {
        return 0;
    }


    cmd = find_command(g_argv[0]);
    if (cmd == NULL)
    {
        printk("Unknown command: %s\n", g_argv[0]);
        printk("Type 'help' for available commands.\n");
        return -1;
    }

    return cmd->handler(argc, g_argv);
}

void    shell_input(char c)
{

    switch (c)
    {
        case '\n':
            printk("\n");
            g_cmd_buffer[g_cmd_pos] = '\0';
            shell_execute(g_cmd_buffer);
            g_cmd_pos = 0;
            k_memset(g_cmd_buffer, 0, SHELL_CMD_MAX_LEN);
            shell_prompt();
            break;

        case '\b':
            if (g_cmd_pos > 0)
            {
                g_cmd_pos--;
                g_cmd_buffer[g_cmd_pos] = '\0';

                printk("\b \b");
            }
            break;

        case '\t':
            break;

        default:
            if (g_cmd_pos < SHELL_CMD_MAX_LEN - 1 && c >= 32 && c < 127)
            {
                g_cmd_buffer[g_cmd_pos] = c;
                g_cmd_pos++;

                vtty_putchar(c);
            }
            break;
    }
}

void    shell_run(void)
{
    t_key_event event;

    shell_prompt();

    while (1)
    {

        if (keyboard_has_key())
        {
            event = keyboard_get_key();


            if (event.pressed)
            {

                if (keyboard_alt_pressed() &&
                    event.scancode >= KEY_F1 && event.scancode <= KEY_F8)
                {
                    vtty_switch((uint8_t)(event.scancode - KEY_F1));
                }

                else if (event.ascii != 0)
                {
                    shell_input(event.ascii);
                }
            }
        }


        __asm__ __volatile__("hlt");
    }
}

int     cmd_help(int argc, char **argv)
{
    int i;

    (void)argc;
    (void)argv;

    printk("\nAvailable commands:\n");
    printk("-------------------\n");

    for (i = 0; g_commands[i].name != NULL; i++)
    {
        printk("  %-10s - %s\n", g_commands[i].name, g_commands[i].help);
    }
    printk("\n");

    return 0;
}

int     cmd_stack(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    stack_print();
    return 0;
}

int     cmd_gdt(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    gdt_print();
    return 0;
}

int     cmd_regs(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    stack_print_registers();
    return 0;
}

int     cmd_clear(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    vtty_clear();
    return 0;
}

int     cmd_info(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    printk("\n");
    printk("=== KFS-2 Kernel Information ===\n");
    printk("Version:      2.0\n");
    printk("Architecture: i386 (x86 32-bit)\n");
    printk("GDT Address:  0x%x\n", GDT_ADDRESS);
    printk("GDT Entries:  %d\n", GDT_ENTRIES);
    printk("Features:\n");
    printk("  - Custom GDT at 0x800\n");
    printk("  - Kernel & User segments\n");
    printk("  - Stack inspection\n");
    printk("  - PS/2 Keyboard\n");
    printk("  - PS/2 Mouse with scroll\n");
    printk("  - Virtual Terminals\n");
    printk("  - Minimalistic Shell\n");
    printk("\n");

    return 0;
}

int     cmd_reboot(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    printk("Rebooting...\n");


    __asm__ __volatile__("cli");


    uint8_t status;
    do
    {
        __asm__ __volatile__("inb $0x64, %0" : "=a"(status));
    } while (status & 0x02);


    __asm__ __volatile__("outb %0, $0x64" : : "a"((uint8_t)0xFE));


    __asm__ __volatile__("lidt (0)");
    __asm__ __volatile__("int $0x03");


    while (1)
    {
        __asm__ __volatile__("hlt");
    }

    return 0;
}

int     cmd_halt(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    printk("System halted.\n");
    printk("You can safely power off the computer.\n");


    __asm__ __volatile__("cli");

    while (1)
    {
        __asm__ __volatile__("hlt");
    }


    return 0;
}
