/*
 * Copyright (c) 2010-2012 Digi International Inc.,
 * All rights not expressly granted are reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Digi International Inc. 11001 Bren Road East, Minnetonka, MN 55343
 * =======================================================================
 */
 /*
    This sample demonstrates the use of ZDO and ZCL discovery to dump a list
    of all ZCL attributes (with their values) of all clusters of all endpoints
    of a device.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "xbee/platform.h"
#include "xbee/device.h"
#include "xbee/atcmd.h"
#include "xbee/wpan.h"

#include "zigbee/zdo.h"
#include "zigbee/zcl_commissioning.h"
#include "zigbee/zcl_basic.h"

#include "_zigbee_walker.h"

 //#include "parse_serial_args.h"

 /*
    TODO

    Need a ZCL endpoint that we can change around as needed.  It should have
    a single cluster, based on the current cluster we're testing (client to
    server or server to client, correct profile ID).

 */

xbee_dev_t my_xbee;
addr64 target[4] = {
    {0x00, 0x13, 0xA2, 0x00, 0x40, 0xB5, 0x8C, 0x22}, //R
    {0x00, 0x13, 0xA2, 0x00, 0x41, 0x03, 0x74, 0x38}, //C
    {0x00, 0x12, 0x4B, 0x00, 0x14, 0xDA, 0xDD, 0x41}, //E non sleep, ptvo
    {0x00, 0x12, 0x4B, 0x00, 0x22, 0xCD, 0x15, 0xD9}, //E sleep, SNZB-03
};

uint8_t startWalking = 0;
uint16_t network_addr_of_target = WPAN_NET_ADDR_UNDEFINED;

#if 1
const zcl_comm_startup_param_t zcl_comm_default_sas =
{
    0xFFFE,                 // short_address
    ZCL_COMM_GLOBAL_EPID,   // extended_panid
    0xFFFF,                 // panid (0xFFFF = not joined)
    UINT32_C(0x7FFF) << 11, // channel_mask
    0x02,                   // startup_control
                            /*
{ { 0, 0, 0, 0, 0, 0, 0, 0 } },        // trust_ctr_addr
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                                       // trust_ctr_master_key
*/
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        // network_key
        FALSE, // use_insecure_join
        // { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        {'Z', 'i', 'g', 'B', 'e', 'e', 'A', 'l', 'l', 'i', 'a', 'n', 'c', 'e', '0', '9'},
        // preconfig_link_key
        0x00,                       // network_key_seq_num
        ZCL_COMM_KEY_TYPE_STANDARD, // network_key_type
        0x0000,                     // network_mgr_addr

        ZCL_COMM_SCAN_ATTEMPTS_DEFAULT,       // scan_attempts
        ZCL_COMM_TIME_BETWEEN_SCANS_DEFAULT,  // time_between_scans
        ZCL_COMM_REJOIN_INTERVAL_DEFAULT,     // rejoin_interval
        ZCL_COMM_MAX_REJOIN_INTERVAL_DEFAULT, // max_rejoin_interval

        {
            FALSE,                                // concentrator.flag
            ZCL_COMM_CONCENTRATOR_RADIUS_DEFAULT, // concentrator.radius
            0x00                                  // concentrato111111r.discovery_time
        },
};
#endif

// void parse_args(int argc, char *argv[])
// {
//    int i;

//    for (i = 1; i < argc; ++i)
//    {
//       if (strncmp(argv[i], "--mac=", 6) == 0)
//       {
//          if (addr64_parse(&target, &argv[i][6]))
//          {
//             fprintf(stderr, "ERROR: couldn't parse MAC %s\n", &argv[i][6]);
//             exit(EXIT_FAILURE);
//          }
//       }
//    }

//    if (addr64_is_zero(&target))
//    {
//       fprintf(stderr, "ERROR: must pass address of target on command-line\n");
//       fprintf(stderr, "\t(--mac=01:23:45:67:89:AB:CD:EF)\n");
//       exit(EXIT_FAILURE);
//    }
// }

int zdo_device_announce_handler(const wpan_envelope_t FAR* envelope, void FAR* context)
{
   // addr64 addr_be;
   char buffer[ADDR64_STRING_LENGTH];
   zdo_device_annce_t FAR* annce;

   annce = (zdo_device_annce_t*)envelope->payload;
   memcpy_letobe(&target[0], &annce->ieee_address_le, 8);
   printf("Device Announce %" PRIsFAR " (0x%04x) cap 0x%02x\n",
      addr64_format(buffer, &target[0]), le16toh(annce->network_addr_le),
      annce->capability);


   sample_endpoints.zdo.cluster_table = NULL;
   sample_endpoints.zdo.handler = &zdo_handler;

   network_addr_of_target = annce->network_addr_le;
   startWalking = 1;

   return 0;
}

int main(int argc, char* argv[])
{
   int status;
   xbee_serial_t XBEE_SERPORT;
   char cmdstr[80];

   // set serial port
   //parse_serial_arguments( argc, argv, &XBEE_SERPORT);

   XBEE_SERPORT.baudrate = 115200;
   memcpy(XBEE_SERPORT.device, "/dev/ttyUSB0", sizeof(XBEE_SERPORT.device));

   // parse args for this program
   //parse_args( argc, argv);

   // initialize the serial and device layer for this XBee device
   if (xbee_dev_init(&my_xbee, &XBEE_SERPORT, NULL, NULL))
   {
      printf("Failed to initialize XBee device.\n");
      return -1;
   }

   // Initialize the WPAN layer of the XBee device driver.  This layer enables
   // endpoints and clusters, and is required for all ZigBee layers.
   xbee_wpan_init(&my_xbee, &sample_endpoints.zdo);

   // Initialize the AT Command layer for this XBee device and have the
   // driver query it for basic information (hardware version, firmware version,
   // serial number, IEEE address, etc.)
   xbee_cmd_init_device(&my_xbee);
   do
   {
      status = xbee_dev_tick(&my_xbee);
      if (status >= 0)
      {
         status = xbee_cmd_query_status(&my_xbee);
      }
   } while (status == -EBUSY);

   if (status == 0)
   {
      printf("waiting to device announce.\ntype \"QUIT\" to return\n");

      //wait till the device is not announce.
      int linelen;
      do
      {
         // linelen = xbee_readline(cmdstr, sizeof cmdstr);
         status = wpan_tick(&my_xbee.wpan_dev);
         if (status < 0)
         {
            printf("Error %d from wpan_tick().\n", status);
            return -1;
         }
         xbee_cmd_tick();
      } while ((0 == startWalking)); //(linelen == -EAGAIN) && 

      // if (linelen == -ENODATA || !strcmpi(cmdstr, "quit"))
      // {
      //    return 0;
      // }

      walker_init(&my_xbee.wpan_dev, &target[0], network_addr_of_target, WALKER_FLAGS_NONE);
      while ((status = wpan_tick(&my_xbee.wpan_dev)) >= 0)
      {
         switch (walker_tick())
         {
         case WALKER_DONE:
            return 0;
         case WALKER_ERROR:
            return -1;
         default:
            break; // continue processing
         }
      }
   }

   if (status < 0)
   {
      printf("Error %d.\n", status);
      return -1;
   }
}
