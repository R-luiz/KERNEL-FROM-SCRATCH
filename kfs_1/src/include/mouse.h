/* ************************************************************************** */
/*                                                                            */
/*   KFS_1 - Kernel From Scratch                                              */
/*                                                                            */
/*   mouse.h - PS/2 Mouse Driver Interface                                    */
/*                                                                            */
/*   NASA/JPL C Coding Standards Compliant                                    */
/*                                                                            */
/* ************************************************************************** */

#ifndef MOUSE_H
#define MOUSE_H

#include "types.h"

/*
** ==========================================================================
** Mouse Constants
** ==========================================================================
*/

#define MOUSE_DATA_PORT         0x60
#define MOUSE_STATUS_PORT       0x64
#define MOUSE_COMMAND_PORT      0x64

#define MOUSE_CMD_ENABLE_AUX    0xA8
#define MOUSE_CMD_GET_COMPAQ    0x20
#define MOUSE_CMD_SET_COMPAQ    0x60
#define MOUSE_CMD_WRITE_MOUSE   0xD4

#define MOUSE_SET_DEFAULTS      0xF6
#define MOUSE_ENABLE_PACKET     0xF4
#define MOUSE_SET_SAMPLE_RATE   0xF3
#define MOUSE_GET_DEVICE_ID     0xF2

/*
** ==========================================================================
** Mouse Packet Flags (byte 0)
** ==========================================================================
*/

#define MOUSE_LEFT_BTN          0x01
#define MOUSE_RIGHT_BTN         0x02
#define MOUSE_MIDDLE_BTN        0x04
#define MOUSE_ALWAYS_ONE        0x08
#define MOUSE_X_SIGN            0x10
#define MOUSE_Y_SIGN            0x20
#define MOUSE_X_OVERFLOW        0x40
#define MOUSE_Y_OVERFLOW        0x80

/*
** ==========================================================================
** Mouse Event Structure
** ==========================================================================
*/

typedef struct s_mouse_event
{
    int8_t      delta_x;
    int8_t      delta_y;
    int8_t      delta_z;        /* Scroll wheel */
    bool_t      left_btn;
    bool_t      right_btn;
    bool_t      middle_btn;
}   t_mouse_event;

/*
** ==========================================================================
** Function Declarations
** ==========================================================================
*/

void        mouse_init(void);
void        mouse_handler(void);
bool_t      mouse_has_event(void);
t_mouse_event mouse_get_event(void);

#endif /* MOUSE_H */
