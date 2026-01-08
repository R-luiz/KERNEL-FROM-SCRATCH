/* ************************************************************************** */
/*                                                                            */
/*   KFS_2 - Kernel From Scratch                                              */
/*                                                                            */
/*   shell.h - Minimalistic Kernel Shell Interface                            */
/*                                                                            */
/*   Bonus feature: A simple command-line shell for debugging.                */
/*   Commands include: help, stack, gdt, reboot, halt, clear                  */
/*                                                                            */
/* ************************************************************************** */

#ifndef SHELL_H
# define SHELL_H

# include "types.h"

/* ============================================================================
 * Shell Constants
 * ============================================================================ */

/* Maximum command line length */
# define SHELL_CMD_MAX_LEN      256

/* Maximum number of arguments per command */
# define SHELL_MAX_ARGS         16

/* Shell prompt string */
# define SHELL_PROMPT           "kfs> "

/* ============================================================================
 * Command Handler Type
 * ============================================================================ */

/*
 * Command handler function pointer type.
 *
 * Parameters:
 *   argc - Number of arguments (including command name)
 *   argv - Array of argument strings
 *
 * Returns:
 *   0 on success, non-zero on error
 */
typedef int (*t_cmd_handler)(int argc, char **argv);

/* ============================================================================
 * Command Structure
 * ============================================================================ */

/*
 * Represents a shell command with its handler and help text.
 */
typedef struct s_shell_cmd
{
    const char      *name;          /* Command name */
    const char      *help;          /* Help text / description */
    t_cmd_handler   handler;        /* Function to execute */
}   t_shell_cmd;

/* ============================================================================
 * Function Prototypes
 * ============================================================================ */

/*
 * Initialize the shell.
 * Sets up command buffer and displays initial prompt.
 */
void    shell_init(void);

/*
 * Run the shell main loop.
 * Processes keyboard input and executes commands.
 * This function does not return (infinite loop).
 */
void    shell_run(void);

/*
 * Process a single character of input.
 * Called from keyboard handler for each key press.
 *
 * Parameters:
 *   c - Character received from keyboard
 */
void    shell_input(char c);

/*
 * Execute a command line.
 * Parses the command and arguments, then calls the appropriate handler.
 *
 * Parameters:
 *   cmdline - The command line string to execute
 *
 * Returns:
 *   0 on success, -1 if command not found, other values for command errors
 */
int     shell_execute(const char *cmdline);

/*
 * Display the shell prompt.
 */
void    shell_prompt(void);

/* ============================================================================
 * Built-in Command Handlers
 * ============================================================================ */

/* Display help for all commands */
int     cmd_help(int argc, char **argv);

/* Print kernel stack (KFS-2 mandatory feature) */
int     cmd_stack(int argc, char **argv);

/* Print GDT information */
int     cmd_gdt(int argc, char **argv);

/* Reboot the system */
int     cmd_reboot(int argc, char **argv);

/* Halt the CPU */
int     cmd_halt(int argc, char **argv);

/* Clear the screen */
int     cmd_clear(int argc, char **argv);

/* Print kernel info / version */
int     cmd_info(int argc, char **argv);

/* Print CPU registers */
int     cmd_regs(int argc, char **argv);

#endif /* SHELL_H */
