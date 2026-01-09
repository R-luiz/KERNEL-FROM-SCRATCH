#ifndef SHELL_H
# define SHELL_H

# include "types.h"

# define SHELL_CMD_MAX_LEN      256

# define SHELL_MAX_ARGS         16

# define SHELL_PROMPT           "kfs> "

typedef int (*t_cmd_handler)(int argc, char **argv);

typedef struct s_shell_cmd
{
    const char      *name;
    const char      *help;
    t_cmd_handler   handler;
}   t_shell_cmd;

void    shell_init(void);

void    shell_run(void);

void    shell_input(char c);

int     shell_execute(const char *cmdline);

void    shell_prompt(void);

int     cmd_help(int argc, char **argv);

int     cmd_stack(int argc, char **argv);

int     cmd_gdt(int argc, char **argv);

int     cmd_reboot(int argc, char **argv);

int     cmd_halt(int argc, char **argv);

int     cmd_clear(int argc, char **argv);

int     cmd_info(int argc, char **argv);

int     cmd_regs(int argc, char **argv);

#endif
