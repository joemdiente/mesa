// Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
// SPDX-License-Identifier: MIT

//***************************************************************************
//* This file contains board specific functions needed for running the PHY  *
//* API at a Tesla evaluation board. The evaluation board is equipped with *
//* a Rabbit CPU board, which do the communication with the PHY using a     *
//* socket connection. The actual API is running on the host computer. The  *
//* API has been tested with both Linux (Red Hat) and Cygwin.               *
//***************************************************************************

#include "vtss_api.h"   // For BOOL and friends
#include "vtss_phy_api.h"   // For PHY API Pre and Post Resets
#include "vtss_appl.h"  // For board types
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Got from https://stackoverflow.com/questions/1941307/debug-print-macro-in-c
 * #define DEBUG greater than 0 to use DEBUG_PRINT()
 */
#if defined(DEBUG) && DEBUG > 0
 #define DEBUG_PRINT(fmt, args...) fprintf(stderr, "DEBUG: %s:%d:%s(): " fmt, \
    __FILE__, __LINE__, __func__, ##args)
#else
 #define DEBUG_PRINT(fmt, args...) /* Don't do anything in release builds */
#endif

// Define which trace group to use for VTSS printout in this file
#define VTSS_TRACE_GROUP VTSS_TRACE_GROUP_PHY

/* ================================================================= *
 *  Misc. functions 
 * ================================================================= */

// Function defining the port interface.
static vtss_port_interface_t port_interface(vtss_port_no_t port_no) {
    return VTSS_PORT_INTERFACE_SGMII;
}

// Function defining the port interface.
static void viper_phy_pre_reset(void) {
    vtss_rc   rc;
    rc = vtss_phy_pre_reset (NULL, 0);
    return;
}

// Function defining the port interface.
static vtss_rc viper_phy_post_reset(void) {
    return (vtss_phy_post_reset (NULL, 0));
}


/* ================================================================= *
 *  Board specific functions
 * ================================================================= */

// Each board can have it own way of communicating with the chip. The miim read and write function are called by the API 
// when the API needs to do register access.
/* 
 * Came from from gemini.google.com
 * Used by connector for converting string output from mdio-tools to uint16
 */
uint16_t hex_string_to_uint16(const char* hex_str) {
    // Remove the "0x" prefix if present
    char* stripped_str = strdup(hex_str); 
    if (stripped_str[0] == '0' && stripped_str[1] == 'x') {
        memmove(stripped_str, stripped_str + 2, strlen(stripped_str) - 1); 
    }

    if (strlen(stripped_str) != 4) {
        fprintf(stderr, "Error: Invalid hex string length. Expected 4 characters.\n");
        free(stripped_str); 
        return -1; 
    }

    char high_byte[3] = {stripped_str[0], stripped_str[1], '\0'};
    char low_byte[3] = {stripped_str[2], stripped_str[3], '\0'};

    uint8_t high_byte_val = (uint8_t)strtol(high_byte, NULL, 16);
    uint8_t low_byte_val = (uint8_t)strtol(low_byte, NULL, 16);

    free(stripped_str); 

    return (uint16_t)((high_byte_val << 8) | low_byte_val);
}
/* -- */

/* mdio-tools (mt_) read function*/
vtss_rc miim_read (const vtss_inst_t inst, const vtss_port_no_t port_no, uint8_t addr, uint16_t *const value) {
    char output[1024]; //Buffer for storing shell output
    char command[2048];
    int rc = -1;

    //Format command
    snprintf(command, sizeof(command), "sudo mdio gpio-0 phy %i raw %i",  port_no, addr);

    DEBUG_PRINT("%s \r\n",command);


    //Open pipe
    FILE *pipe = popen(command,"r");

    //Check if opening popen failed
    if (pipe == NULL) {
        perror("popen() failed");
        return -1;
    }

    //Get output from pipe
    while (fgets(output, sizeof(output), pipe) != NULL) {
        // Process the output as needed (e.g., print it)
        //Debug
        DEBUG_PRINT("%s", output); 
    }

    /* Expected output string is 0x0000
     * Copy string to up to 6th char and then set 7th to \0 (null ptr) 
     */
    char *reg_val_str;
    output[6] = '\0';
    *value = hex_string_to_uint16(output);
    if (*value == -1 ) {
        DEBUG_PRINT("conversion failed\r\n");
        return -1;
    }
    pclose(pipe);

    return VTSS_RC_OK;
}
/* mdio-tools (mt_) read function*/
vtss_rc miim_write (const vtss_inst_t inst, const vtss_port_no_t port_no, uint8_t addr, uint16_t value) {
    char command[2048];
    char output[512];
    uint8_t phy_no;
    int rc = -1;

    //Format command
    snprintf(command, sizeof(command), "sudo mdio gpio-0 phy %u raw %i %u", port_no, addr, value);
    DEBUG_PRINT(" %s \r\n",command);

    //Open pipe
    FILE *pipe = popen(command,"r");

    //Check if opening popen failed
    if (pipe == NULL) {
        perror("popen() failed");
        return -1;
    }

    //Get output from pipe (unnecessary, just checking if fails)
    //If there is an output then it failed or something.
    if(fgets(output, sizeof(output), pipe) != NULL) {
        //There should be no output after writing.
        DEBUG_PRINT("%s", output); 
        rc = -1;
    }
    else rc = VTSS_RC_OK;
 
    pclose(pipe);

    return rc;
}

#if defined(VTSS_CHIP_10G_PHY)
/* Write code for this for clause 45 (sudo mdio gpio-0 mmd?? maybe?)*/
vtss_rc mmd_read(const vtss_inst_t    inst,
                        const vtss_port_no_t  port_no,
                        const u8              mmd,
                        u16                   addr,
                        u16                   *const value)
{
    /* Must be filled out by the user */
    T_N("mmd_read port_no = %d, mmd = %d addr = %d, value = 0x%X", port_no, mmd, addr);
    return VTSS_RC_OK;
}

vtss_rc mmd_write(const vtss_inst_t    inst,
                         const vtss_port_no_t  port_no,
                         const u8              mmd,
                         u16                   addr,
                         u16                   data)
{
    /* Must be filled out by the user */
    T_N("mmd_write port_no = %d, mmd = %d addr = %d, value = 0x%X", port_no, addr, data);
    return VTSS_RC_OK;
}
#endif /* VTSS_CHIP_10G_PHY */

// Function for initializing the hardware board.
int viper_board_init(int argc, const char **argv, vtss_appl_board_t *board)
{
    board->descr = "Viper_Eval";
    board->target = VTSS_TARGET_CU_PHY;  // 1G Copper PHY
    board->port_count = VTSS_PORTS; //Setup the number of port used 

    board->port_interface = port_interface; // Define the port interface

    board->pre_reset  = viper_phy_pre_reset;
    board->post_reset = viper_phy_post_reset;

    board->init.init_conf->miim_read =  miim_read; // Set pointer to the MIIM read function for this board.
    board->init.init_conf->miim_write = miim_write; // Set pointer to the MIIM write function for this board.

#if defined(VTSS_CHIP_10G_PHY)
    board->init.init_conf->mmd_read =  mmd_read; // Set pointer to the MIIM read function for this board.
    board->init.init_conf->mmd_write = mmd_write; // Set pointer to the MIIM write function for this board.
#endif /* VTSS_CHIP_10G_PHY */

    if (board->init.init_conf->warm_start_enable != TRUE) {
        /*
        * Checking argc,arv
        * Do I need to check these maybe gpio-0.
        */
        
        /* ========== */

        /*
        * Check if mdio-tools and mdio-netlink is installed.
        */
        char test_output[1024];
        //Open pipe then run this command
        FILE *pipe = popen("sudo mdio gpio-0","r");
        if (pipe == NULL) {
            perror("popen() failed");
            return -1;
        }

        //Get stdout from pipe
        fgets(test_output, sizeof(test_output), pipe);

        /* Very unprofessional checking :D
         * The output is expected to be " DEV      PHY-ID  LINK"
         * Just check the first 4 bytes in ASCII.
         */
        if (test_output[0] != 27 && //" "
            test_output[1] != 91 && //"D"
            test_output[2] != 55 && //"E"
            test_output[3] != 109) {//"V"
            printf("sudo mdio gpio-0 command failed\r\n");
            exit(EXIT_SUCCESS);
        }
        // Set signal detect polarity (for SFPs for the board)
        miim_write(NULL, 0, 31, 1);
        miim_write(NULL, 0, 19, 1);
        miim_write(NULL, 0, 31, 0);

        miim_write(NULL, 3, 31, 1);
        miim_write(NULL, 3, 19, 1);
        miim_write(NULL, 3, 31, 0);
    }
    return 0;
}




