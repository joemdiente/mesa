// Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
// SPDX-License-Identifier: MIT

//Standard Library
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

//MEBA API
#include "microchip/ethernet/board/api.h"
#include "meba_aux.h"

#if 1
#define PORTS_MAX 4
#endif

//My Boards
typedef enum {
    BOARD_NAME_VSC7514_PCB123,
    BOARD_NAME_LAN9668_UNG8290
} my_board_name_t;

//Joem: MAX_PORTS for both evaluation boards
#define MAX_PORTS 11 

typedef struct meba_board_state {
    my_board_name_t       board_type;
    int                   port_cnt;
    meba_port_entry_t     *entry;
    mepa_device_t         *phy_devices[PORTS_MAX];
    mesa_port_status_t    status[PORTS_MAX];
} meba_board_state_t;

//My Meba Implementation
meba_inst_t meba_initialize(size_t callouts_size, const meba_board_interface_t *callouts)
{
    meba_inst_t         instance;
    meba_board_state_t  *board;
    
    //Joem: For board_conf_get
    char                board_name[32];
    uint32_t            target = 0;
    uint32_t            type = 10000;
    uint32_t            board_port_cnt = 10000;
    uint32_t            mux_mode = 0xffffffff;
    int32_t              mep_loop_port = -1;
    
    //Joem: Greetings from MEBA
    fprintf(stdout, "Greetings from MEBA!\r\n");

    if (callouts_size < sizeof(*callouts)) {
        fprintf(stderr, "Callouts size problem, expected %zd, got %zd\n",
                sizeof(*callouts), callouts_size);
    
        return NULL;
    }
    
    //Joem: Get Information
    fprintf(stdout, "Getting board information\r\n");
    if(callouts->conf_get("board", board_name, sizeof(board_name), NULL) == MESA_RC_OK)
    {
    fprintf(stdout, "Board: %s\r\n", board_name);
    }
    else fprintf(stdout, "Failed to get board information\r\n");

    //Joem: Get Target Information

    //Joem: Get Type Information

    //Joem: Get Mux_Mode

    //Joem: Get mep_loop_port

    //Joem: Get pcb

    //Joem: Get pcb_var

    //Allocate and Initialize MEBA Public State
    if(strcmp(board_name,"Ocelot Ref (pcb123)") == 0) 
    {
        if ((instance = meba_state_alloc(callouts,
                                "My Own VSC7514",
                                MESA_TARGET_7514,
                                sizeof(*board))) == NULL) {
            return NULL;
        }
    }
    else if (strcmp(board_name,"ung8290") == 0)  //TODO
    {
        if ((instance = meba_state_alloc(callouts,
                                "My Own LAN9668",
                                MESA_TARGET_LAN9668,
                                sizeof(*board))) == NULL) {
            return NULL;
        }
    }
    else 
    {
        fprintf(stdout, "Failed to Set Board\r\n");
    }
      
    // Initialize our state
    MEBA_ASSERT(instance->private_data != NULL);
    board = INST2BOARD(instance);
    
    //Joem: For Testing Purposes Only
    return NULL;
}

// static uint32_t my_own_vsc7514_capability(meba_inst_t inst, int cap)
// {

// }
// static mesa_rc my_own_vsc7514_reset_t(meba_inst_t inst, meba_reset_point_t reset)
// {

// }
// static mesa_rc my_own_vsc7514_port_entry_get_t(meba_inst_t inst, mesa_port_no_t port_no, meba_port_entry_t *entry)
// {

// }