// Copyright (c) 2004-2023 Microchip Technology Inc. and its subsidiaries.
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
    BOARD_TYPE_VSC7514_PCB123,
    BOARD_TYPE_LAN9668_UNG8290
} my_board_type_t;

//Joem: MAX_PORTS for both evaluation boards
#define MAX_PORTS 11 

typedef struct meba_board_state {
    my_board_type_t       board_type;
    int                   port_cnt;
    meba_port_entry_t     *entry;
    mepa_device_t         *phy_devices[PORTS_MAX];
    mesa_port_status_t    status[PORTS_MAX];
} meba_board_state_t;

//Joem: TOFIGUREOUT
/* Observation: meba_aux is only used for VSC* switches not in LAN9668.
 */
static const meba_aux_rawio_t rawio = 
{
    .base = 0,
    .gcb = 0x07,
    .miim = {
        .status = 0x27+0,
        .cmd    = 0x27+2,
        .data   = 0x27+3,
        .cfg    = 0x27+4,
    },
    .gpio = {
        .alt_0  = 0x15,
    }
};

static uint32_t my_own_vsc7514_capability(meba_inst_t inst, int cap)
{

}
static mesa_rc my_own_vsc7514_reset_t(meba_inst_t inst, meba_reset_point_t reset)
{

}
static mesa_rc my_own_vsc7514_port_entry_get_t(meba_inst_t inst, mesa_port_no_t port_no, meba_port_entry_t *entry)
{

}

static mesa_rc pcb123_init_board(meba_inst_t inst)
{
    mesa_rc rc;
    mesa_sgpio_conf_t conf;
    uint32_t port, gpio_no;

    /* Configure GPIOs for MIIM/MDIO and I2C */
    for (gpio_no = 14; gpio_no <= 17; gpio_no++) {
        (void)mesa_gpio_mode_set(NULL, 0, gpio_no, MESA_GPIO_ALT_0);
        printf("gpio_no: %i\r\n",gpio_no);
    }

    /* GPIO pins 0-3 are used for SGPIOs. */
    (void)mesa_gpio_mode_set(NULL, 0, 0, MESA_GPIO_ALT_0);  // SGPIO Grp 0 / CLK
    (void)mesa_gpio_mode_set(NULL, 0, 1, MESA_GPIO_ALT_0);  // SGPIO Grp 0 / DO
    (void)mesa_gpio_mode_set(NULL, 0, 2, MESA_GPIO_ALT_0);  // SGPIO Grp 0 / DI
    (void)mesa_gpio_mode_set(NULL, 0, 3, MESA_GPIO_ALT_0);  // SGPIO Grp 0 / LD

    printf("sgpio\r\n");
    /* Setup SGPIO group 0 */
    if ((rc = mesa_sgpio_conf_get(NULL, 0, 0, &conf)) == MESA_RC_OK) {
        
        printf("sgpio setup \r\n");
        /* The blink mode 0 is 5 HZ for link activity and collisions in half duplex. */
        conf.bmode[0] = MESA_SGPIO_BMODE_5;

        /* Enable two bits per port */
        conf.bit_count = 2;

        /* Enable SLED ports 10:0 as port status LEDs */
        for (port = 0; port <= 10; port++) {
            conf.port_conf[port].enabled = true;
            conf.port_conf[port].mode[0] = MESA_SGPIO_MODE_ON;
            conf.port_conf[port].mode[1] = MESA_SGPIO_MODE_ON;
            conf.port_conf[port].int_pol_high[0] = true; /* LOS of signal is active high */
        }

        /* Enable SLED port 11 as system status LED */
        conf.port_conf[11].enabled = true;
        conf.port_conf[11].mode[0] = MESA_SGPIO_MODE_ON;
        conf.port_conf[11].mode[1] = MESA_SGPIO_MODE_OFF;

        /* Enable SGPIO output ports 23:12 as LED_SEL_x (dual-media), MUX_SELx (I2C),
           RS422_xOE (IEEE1588 RS422), SFP control signals and CardDetect from uSD slot */
        for (port = 12; port <= 23; port++) {
            conf.port_conf[port].enabled = true;
            conf.port_conf[port].mode[0] = MESA_SGPIO_MODE_OFF;
            conf.port_conf[port].mode[1] = MESA_SGPIO_MODE_OFF;
        }

        /* MUX_SELx (I2C) is controlled by the BSP driver */
        conf.port_conf[13].mode[0] = MESA_SGPIO_MODE_NO_CHANGE;
        conf.port_conf[13].mode[1] = MESA_SGPIO_MODE_NO_CHANGE;
        conf.port_conf[14].mode[0] = MESA_SGPIO_MODE_NO_CHANGE;

        /* SFP RateSel = enabled */
        for (port = 16; port <= 19; port++) {
            conf.port_conf[port].mode[0] = MESA_SGPIO_MODE_ON;
            conf.port_conf[port].mode[1] = MESA_SGPIO_MODE_ON;
        }

        /* SFP TxDisable = enabled */
        for (port = 20; port <= 23; port++) {
            conf.port_conf[port].mode[0] = MESA_SGPIO_MODE_ON;
            conf.port_conf[port].mode[1] = MESA_SGPIO_MODE_ON;
        }

        (void)mesa_sgpio_conf_set(NULL, 0, 0, &conf);
    }
    return rc;
}

//My Meba Implementation
meba_inst_t meba_initialize(size_t callouts_size, const meba_board_interface_t *callouts)
{
    meba_inst_t         instance;
    meba_board_state_t  *board;
    mesa_rc             rc = 0;

    //Joem: For board_conf_get
    char                board_name[32];
    uint32_t            target = 0;
    uint32_t            type = 10000;
    uint32_t            board_port_cnt = 10000;
    uint32_t            mux_mode = 0xffffffff;
    int32_t             mep_loop_port = -1;
    uint16_t            data = 0;
    uint32_t            i;
    
    
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

    /* 
     * Some information below are board-specific 
     * so not all of them return values. 
     * (see board_conf_get()).
     */
    // //Joem: Get Target Information
    // fprintf(stdout, "Getting target information\r\n");
    // if(callouts->conf_get("target", (char*)target, sizeof(target), NULL) == MESA_RC_OK)
    // {
    //     fprintf(stdout, "Target: 0x%x\r\n", target);
    // }
    // else fprintf(stdout, "Failed to get target information\r\n");

    // //Joem: Get Type Information
    // fprintf(stdout, "Getting type information\r\n");
    // if(callouts->conf_get("type", (char*)type, sizeof(type), NULL) == MESA_RC_OK)
    // {
    //     fprintf(stdout, "Ttype: %u\r\n", type);
    // }
    // else fprintf(stdout, "Failed to get type information\r\n");

    // //Joem: Get Mux_Mode
    // fprintf(stdout, "Getting mux_mode information\r\n");
    // if(callouts->conf_get("mux_mode", (char*)mux_mode, sizeof(mux_mode), NULL) == MESA_RC_OK)
    // {
    //     fprintf(stdout, "mux_mode: %u\r\n", mux_mode);
    // }
    // else fprintf(stdout, "Failed to get mux_mode information\r\n");

    // //Joem: Get mep_loop_port
    // fprintf(stdout, "Getting mep_loop_port information\r\n");
    // if(callouts->conf_get("mep_loop_port", (char*)mep_loop_port, sizeof(mep_loop_port), NULL) == MESA_RC_OK)
    // {
    //     fprintf(stdout, "mep_loop_port: %u\r\n", mep_loop_port);
    // }
    // else fprintf(stdout, "Failed to get mep_loop_port information\r\n");

    // //Joem: Get pcb
    // fprintf(stdout, "Getting pcb information\r\n");
    // if(callouts->conf_get("pcb", (char*)type, sizeof(type), NULL) == MESA_RC_OK)
    // {
    //     fprintf(stdout, "pcb: %u\r\n", type);
    // }
    // else fprintf(stdout, "Failed to get pcb information\r\n");

    // //Joem: Get pcb_var
    // fprintf(stdout, "Getting pcb_var information\r\n");
    // if(callouts->conf_get("pcb_var", (char*)board_port_cnt, sizeof(board_port_cnt), NULL) == MESA_RC_OK)
    // {
    //     fprintf(stdout, "pcb_var: %u\r\n", board_port_cnt);
    // }
    // else fprintf(stdout, "Failed to get pcb_var information\r\n");

    //Joem: Allocate and Initialize MEBA Public State
    if(strcmp(board_name,"Ocelot Ref (pcb123)") == 0) 
    {
        if ((instance = meba_state_alloc(callouts,
                                "My Own VSC7514",
                                MESA_TARGET_7514,
                                sizeof(*board))) == NULL) {
            return NULL;
        }
        printf("Allocate and Init Success for VSC7514EV\r\n");
    } 
    else if (strcmp(board_name,"ung8290") == 0)  //Joem: TODO
    {
        if ((instance = meba_state_alloc(callouts,
                                "My Own LAN9668",
                                MESA_TARGET_LAN9668,
                                sizeof(*board))) == NULL) {
            return NULL;
        }
        printf("Allocate and Init Success for EVB-LAN9668\r\n");
    }
    else 
    {
        fprintf(stdout, "Failed to Set Board\r\n");
    }
      
    // Initialize our state
    printf("Init MEBA\r\n");
    MEBA_ASSERT(instance->private_data != NULL);
    board = INST2BOARD(instance);

    if (meba_conf_get_u32(instance, "mux_mode", &i) == MESA_RC_OK) {
        instance->props.mux_mode = (mesa_port_mux_mode_t) i;
        printf("mux_mode: %i\r\n", instance->props.mux_mode);
    } else {
        // Defaults
        T_D(instance, "Failed to read 'mux_mode' from the configuration file, reverting to defaults.");
        if (instance->props.target == MESA_TARGET_7513) {
            instance->props.mux_mode = MESA_PORT_MUX_MODE_0;
        } else if (instance->props.target == MESA_TARGET_7514) {
            instance->props.mux_mode = MESA_PORT_MUX_MODE_4;
        }
    }

    //Joem: Sets GPIO_14 and GPIO_15 to use their Alternative Functions (MDC an MDIO)
    (void)mebaux_gpio_mode_set(instance, &rawio, 14, MESA_GPIO_ALT_0);
    (void)mebaux_gpio_mode_set(instance, &rawio, 15, MESA_GPIO_ALT_0);

    //Joem: Perform Read PHYs (7 = miim_addr, 2 = reg_addr, &oui = data)
    rc = mebaux_miim_rd(instance, &rawio, MESA_MIIM_CONTROLLER_1, 7, 2, &data);
    printf("Data from PHY reg 2: 0x%x\r\n", data);

    //Joem: HARDCODED
    printf("Hardcoded Board Type\r\n");
    board->board_type = BOARD_TYPE_VSC7514_PCB123;
    strncpy(instance->props.name, "Ocelot Ref", sizeof(instance->props.name));
    board->port_cnt = 10; //no NPI port
    
    printf("Hardcoded Actual Number of Ports\r\n");
    // The actual number of ports the HW design has, not the one exposed by board->port_cnt
    uint32_t count = (board->board_type == BOARD_TYPE_VSC7514_PCB123) ? 10 :
                     ((board->board_type == BOARD_TYPE_LAN9668_UNG8290) ? 8 : 10);
    // board->sgpio_port = (uint32_t*) calloc(count, sizeof(uint32_t));
    // if (board->sgpio_port == NULL) {
    //     fprintf(stderr, "Board to SGPIO port table malloc failure\n");
    //     goto error_out;
    // }

    //Joem: Manual Port Table Allocation
    printf("Direct PCB123_INIT_BOARD call\r\n");
    rc = pcb123_init_board(instance);
    if (rc != MESA_RC_OK) 
    {
        printf("Failed to fill port mapping table\r\n");
        goto error_out;
    }
    instance->props.board_type = (vtss_board_type_t) (VTSS_BOARD_OCELOT_REF + board->board_type);    // Exposed temporarily
    printf("Board: %s, type %d, target %4x, %d ports, port_cfg %d\r\n",
        instance->props.name, board->board_type, instance->props.target, board->port_cnt, instance->props.mux_mode);

    // Hook up board API functions
    printf("Hooking up board API\r\n");
    instance->api.meba_capability                 = my_own_vsc7514_capability;
    instance->api.meba_port_entry_get             = my_own_vsc7514_port_entry_get_t;
    instance->api.meba_reset                      = my_own_vsc7514_reset_t;
    instance->api.meba_sensor_get                 = NULL;
    instance->api.meba_sfp_i2c_xfer               = NULL;
    instance->api.meba_sfp_insertion_status_get   = NULL;
    instance->api.meba_sfp_status_get             = NULL;
    instance->api.meba_port_admin_state_set       = NULL;
    instance->api.meba_status_led_set             = NULL;
    instance->api.meba_port_led_update            = NULL;
    //inst->api.meba_led_intensity_set          = ocelot_led_intensity_set;
    instance->api.meba_irq_handler                = NULL;
    instance->api.meba_irq_requested              = NULL;
    instance->api.meba_event_enable               = NULL;
    instance->api.meba_deinitialize               = meba_deinitialize;
    instance->api.meba_ptp_rs422_conf_get         = NULL;
    instance->api.meba_ptp_external_io_conf_get   = NULL;

    instance->api_synce = NULL;;
    instance->api_tod = NULL;;
    instance->api_poe = NULL;;

    //Joem: For Testing Purposes Only
    printf("End\r\n");

    return instance;

error_out:
    free(instance);
    return NULL;
}