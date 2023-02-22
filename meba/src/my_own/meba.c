// Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
// SPDX-License-Identifier: MIT

//Standard Library
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

//MEBA API
#include "microchip/ethernet/board/api.h"

#if 1
#define PORTS_MAX 4
#endif

//My Own Boards
typedef enum {
    BOARD_TYPE_VSC7514_PCB123,
    BOARD_TYPE_OCELOT_PCB123,
    BOARD_TYPE_OCELOT_PCB123_LAN8814
} board_type_t;

typedef struct meba_board_state {
    board_type_t          type;
    int                   port_cnt;
    meba_port_entry_t     *entry;
    mepa_device_t         *phy_devices[PORTS_MAX];
    mesa_port_status_t    status[PORTS_MAX];
} meba_board_state_t;

//My Own Meba Implementation
meba_inst_t meba_initialize(size_t callouts_size, const meba_board_interface_t *callouts)
{
    meba_inst_t instance;
    meba_board_state_t *board;
    
    //Joem: Greetings from MEBA
    fprintf(stdout, "Greetings from MEBA!\r\n");

    if (callouts_size < sizeof(*callouts)) {
        fprintf(stderr, "Callouts size problem, expected %zd, got %zd\n",
                sizeof(*callouts), callouts_size);
    
        return NULL;
    }
    
    char *data;
    size_t data_size;
    
    //Get Information
    fprintf(stdout, "Get Information From The Board\r\n");

    //Joem: Get Board Information
    printf(callouts->conf_get("board", data, 10, &data_size));
    fprintf(stdout, "Board: %s, %u\r\n", data, data_size);
    fprintf(stdout, "Board Done!\r\n");

    //Joem: Get Target Information

    //Joem: Get Type Information

    //Joem: Get Mux_Mode

    //Joem: Get mep_loop_port

    //Joem: Get pcb

    //Joem: Get pcb_var

    //Allocate and Initialize MEBA Public State
    // if(instance->props.target == MESA_TARGET_7514) {
    //         if ((instance = meba_state_alloc(callouts,
    //                                 "My Own VSC7514",
    //                                 MESA_TARGET_7514,
    //                                 sizeof(*board))) == NULL) {
    //         return NULL;
    //     }
    // } else if (instance->props.target == MESA_TARGET_LAN9668) {
    //         if ((instance = meba_state_alloc(callouts,
    //                                 "My Own LAN9668",
    //                                 MESA_TARGET_LAN9668,
    //                                 sizeof(*board))) == NULL) {
    //         return NULL;
    //     }
    // }

    // // Initialize our state
    // MEBA_ASSERT(instance->private_data != NULL);
    // board = INST2BOARD(instance);
    
    //Joem: For Testing Purposes Only
    return NULL;
}

static uint32_t my_own_vsc7514_capability(meba_inst_t inst, int cap)
{

}
static mesa_rc my_own_vsc7514_reset_t(meba_inst_t inst, meba_reset_point_t reset)
{

}
static mesa_rc my_own_vsc7514_port_entry_get_t(meba_inst_t inst, mesa_port_no_t port_no, meba_port_entry_t *entry)
{

}