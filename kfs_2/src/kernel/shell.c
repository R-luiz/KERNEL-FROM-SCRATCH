/* ************************************************************************** */
/*                                                                            */
/*   KFS_2 - Kernel From Scratch                                              */
/*                                                                            */
/*   shell.c - Minimalistic Kernel Shell Implementation                       */
/*                                                                            */
/*   Bonus feature: A simple command-line shell for debugging.                */
/*                                                                            */
/* ************************************************************************** */

#include "shell.h"
#include "stack.h"
#include "gdt.h"
#include "keyboard.h"
#include "vga.h"
#include "types.h"

/* String library functions */
extern size_t   k_strlen(const char *s);
extern int      k_strcmp(const char *s1, const char *s2);
extern int      k_strncmp(const char *s1, const char *s2, size_t n);
extern char     *k_strcpy(char *dest, const char *src);
extern void     *k_memset(void *s, int c, size_t n);

/* External printk */
extern void printk(const char *fmt, ...);

/* ============================================================================
 * Static Variables
 * ============================================================================ */

/* Command line buffer */
static char     g_cmd_buffer[SHELL_CMD_MAX_LEN];
static size_t   g_cmd_pos = 0;

/* Argument buffer for parsing */
static char     g_arg_buffer[SHELL_CMD_MAX_LEN];
static char     *g_argv[SHELL_MAX_ARGS];

/* ============================================================================
 * Command Table
 * ============================================================================ */

/* Table of available commands */
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

/* ============================================================================
 * Helper Functions
 * ============================================================================ */

/*
 * Parse command line into argc/argv format.
 * Modifies g_arg_buffer and fills g_argv.
 *
 * Returns number of arguments (argc).
 */
static int parse_cmdline(const char *cmdline)
{
    int     argc = 0;
    size_t  i = 0;
    size_t  j = 0;
    bool_t  in_word = FALSE;

    /* Copy command line to argument buffer */
    k_strcpy(g_arg_buffer, cmdline);

    /* Parse into arguments */
    while (g_arg_buffer[i] != '\0' && argc < SHELL_MAX_ARGS)
    {
        /* Skip whitespace */
        if (g_arg_buffer[i] == ' ' || g_arg_buffer[i] == '\t')
        {
            if (in_word)
            {
                /* End of word */
                g_arg_buffer[j] = '\0';
                j++;
                in_word = FALSE;
            }
            i++;
            continue;
        }

        /* Start of new word */
        if (!in_word)
        {
            g_argv[argc] = &g_arg_buffer[j];
            argc++;
            in_word = TRUE;
        }

        /* Copy character */
        g_arg_buffer[j] = g_arg_buffer[i];
        j++;
        i++;
    }

    /* Null-terminate last argument */
    if (in_word)
    {
        g_arg_buffer[j] = '\0';
    }

    return argc;
}

/*
 * Find command in command table.
 *
 * Returns pointer to command structure, or NULL if not found.
 */
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

/* ============================================================================
 * Shell Core Functions
 * ============================================================================ */

/*
 * Initialize the shell.
 */
void    shell_init(void)
{
    k_memset(g_cmd_buffer, 0, SHELL_CMD_MAX_LEN);
    g_cmd_pos = 0;

    printk("\n");
    printk("KFS-2 Shell v1.0\n");
    printk("Type 'help' for available commands.\n\n");
}

/*
 * Display shell prompt.
 */
void    shell_prompt(void)
{
    printk(SHELL_PROMPT);
}

/*
 * Execute a command line.
 */
int     shell_execute(const char *cmdline)
{
    int                 argc;
    const t_shell_cmd   *cmd;

    /* Skip empty command lines */
    if (cmdline == NULL || cmdline[0] == '\0')
    {
        return 0;
    }

    /* Parse command line */
    argc = parse_cmdline(cmdline);
    if (argc == 0)
    {
        return 0;
    }

    /* Find and execute command */
    cmd = find_command(g_argv[0]);
    if (cmd == NULL)
    {
        printk("Unknown command: %s\n", g_argv[0]);
        printk("Type 'help' for available commands.\n");
        return -1;
    }

    return cmd->handler(argc, g_argv);
}

/*
 * Process a single character of input.
 */
void    shell_input(char c)
{
    /* Handle special characters */
    switch (c)
    {
        case '\n':  /* Enter - execute command */
            printk("\n");
            g_cmd_buffer[g_cmd_pos] = '\0';
            shell_execute(g_cmd_buffer);
            g_cmd_pos = 0;
            k_memset(g_cmd_buffer, 0, SHELL_CMD_MAX_LEN);
            shell_prompt();
            break;

        case '\b':  /* Backspace */
            if (g_cmd_pos > 0)
            {
                g_cmd_pos--;
                g_cmd_buffer[g_cmd_pos] = '\0';
                /* Move cursor back, print space, move back again */
                printk("\b \b");
            }
            break;

        case '\t':  /* Tab - ignore for now */
            break;

        default:    /* Regular character */
            if (g_cmd_pos < SHELL_CMD_MAX_LEN - 1 && c >= 32 && c < 127)
            {
                g_cmd_buffer[g_cmd_pos] = c;
                g_cmd_pos++;
                /* Echo character */
                vga_putchar(c);
            }
            break;
    }
}

/*
 * Run the shell main loop.
 * Processes keyboard events and handles commands.
 */
void    shell_run(void)
{
    t_key_event event;

    shell_prompt();

    while (1)
    {
        /* Check for keyboard input */
        if (keyboard_has_key())
        {
            event = keyboard_get_key();

            /* Only process key press events (not releases) */
            if (event.pressed && event.ascii != 0)
            {
                shell_input(event.ascii);
            }
        }

        /* Small delay to avoid busy-waiting */
        __asm__ __volatile__("hlt");
    }
}

/* ============================================================================
 * Built-in Command Handlers
 * ============================================================================ */

/*
 * Display help for all commands.
 */
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

/*
 * Print kernel stack dump.
 * This is the main KFS-2 mandatory feature.
 */
int     cmd_stack(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    stack_print();
    return 0;
}

/*
 * Display GDT entries.
 */
int     cmd_gdt(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    gdt_print();
    return 0;
}

/*
 * Display CPU registers.
 */
int     cmd_regs(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    stack_print_registers();
    return 0;
}

/*
 * Clear the screen.
 */
int     cmd_clear(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    vga_clear();
    return 0;
}

/*
 * Display kernel information.
 */
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

/*
 * Reboot the system.
 * Uses the keyboard controller to trigger a CPU reset.
 */
int     cmd_reboot(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    printk("Rebooting...\n");

    /*
     * Reboot via keyboard controller.
     * Writing 0xFE to port 0x64 causes the CPU to reset.
     *
     * First, disable interrupts and wait for keyboard controller.
     */
    __asm__ __volatile__("cli");

    /* Wait for keyboard controller input buffer to be empty */
    uint8_t status;
    do
    {
        __asm__ __volatile__("inb $0x64, %0" : "=a"(status));
    } while (status & 0x02);

    /* Send reset command to keyboard controller */
    __asm__ __volatile__("outb %0, $0x64" : : "a"((uint8_t)0xFE));

    /* If that didn't work, try triple fault */
    __asm__ __volatile__("lidt (0)");  /* Load invalid IDT */
    __asm__ __volatile__("int $0x03"); /* Trigger interrupt -> triple fault */

    /* Should never reach here */
    while (1)
    {
        __asm__ __volatile__("hlt");
    }

    return 0;
}

/*
 * Halt the CPU.
 * Disables interrupts and enters infinite halt loop.
 */
int     cmd_halt(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    printk("System halted.\n");
    printk("You can safely power off the computer.\n");

    /* Disable interrupts and halt */
    __asm__ __volatile__("cli");

    while (1)
    {
        __asm__ __volatile__("hlt");
    }

    /* Never reached */
    return 0;
}
