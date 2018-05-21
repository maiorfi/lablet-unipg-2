/**
 * Copyright (c) 2015 Digi International Inc.,
 * All rights not expressly granted are reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Digi International Inc. 11001 Bren Road East, Minnetonka, MN 55343
 * =======================================================================
 * This sample program originates from the Digi library
 * and it was heavily modified by Andrea Scorzoni (UNIPG)
 * and Lorenzo Maiorfi (Innovactive srl).
 * Extensive comments have also been added to the original
 * Digi sample program.
 * The program reads relative humidity and temperature 
 * from a Si7021 I2C Adafruit sensor and sends collected data
 * to a predefined ZigBee XBee receiving node. 
 * The program needs:
 * 1. XBeeLib_Fixed.lib and folder
 * 2. config.h
 * 3. crc8.cpp
 * 4. Si7021 folder containing Si7021.h and Si7021.cpp
 * For setting the program up please:
 * 1) define RADIO_TX, RADIO_RX and optionally RADIO_RESET in config.h
 * 2) the following lines in config.h should be commented:
 *    //#define ENABLE_LOGGING
 *    //#define ENABLE_ASSERTIONS
 * 3) set the predefined ZigBee XBee receiving node REMOTE_NODE_ADDR64_LSB
 * 4) define the 3-char SENSOR_NODE_ID[] of the sending node
 * 5) set the I2C pins of your mbedOS board, e.g. I2C i2c(PC_9, PA_8);
 * 6) set the bit rate of the sending XBee module to 115200 bps
 * 7) set the bit rate of the debug terminal to 115200 bps
 */

#include "mbed.h"
#include "XBeeLib.h"
#include "Si7021.h" // Si7021 library
#if defined(ENABLE_LOGGING)
#include "DigiLoggerMbedSerial.h"
using namespace DigiLog; // when looking for types try Digilog namespace
#endif

#define Si7021_ADDR (0x80) // Si7021 address is 0x40
                           // but 0x5A<<1 (left shifted) turns to 0x80,
                           // therefore in I2C commands the "address" is 0x80

/* Univocal XBee MAC address
 * of the module/node connected to the PC which
 * the function "send_data_to_remote_node()" 
 * will send to (in our case the Coordinator).
 * The 1st part is the same for all XBee modules.
 * The 2nd part must be customized.
 */
#define REMOTE_NODE_ADDR64_MSB ((uint32_t)0x0013A200)

#define REMOTE_NODE_ADDR64_LSB ((uint32_t)0x40DDE61F) // "1F" XBee PRO Coordinator
//#define REMOTE_NODE_ADDR64_LSB  ((uint32_t)0x40F5367B) // "7B" Coordinator or Router
//#define REMOTE_NODE_ADDR64_LSB  ((uint32_t)0x41056EB2) // "B2" Router
//#define REMOTE_NODE_ADDR64_LSB  ((uint32_t)0x41565EBB) // "BB" Router
//#define REMOTE_NODE_ADDR64_LSB  ((uint32_t)0x41056ECC) // "CC" Router
//#define REMOTE_NODE_ADDR64_LSB  ((uint32_t)0x417C0434) // "34" Router
//#define REMOTE_NODE_ADDR64_LSB  ((uint32_t)0x417C0331) // "31" Router
//#define REMOTE_NODE_ADDR64_LSB  ((uint32_t)0x417C0401) // "01" Router

/* defined in utils.h of XBeeLib_Fixed:
 * #define UINT64(msb,lsb)     (uint64_t)(((uint64_t)(msb) << 32) | (lsb))
 */
#define REMOTE_NODE_ADDR64 UINT64(REMOTE_NODE_ADDR64_MSB, REMOTE_NODE_ADDR64_LSB)

using namespace XBeeLib; // when looking for types try XBeeLib namespace

/* instead of defining "Serial log_serial;" we use a pointer
 * to a Serial type. This is actually a pointer 
 * to an instance which will be created later in the main.
 * However we need to define a prototype here, since
 * all following functions use log_serial global variable
 */
Serial *log_serial;

char SENSOR_NODE_ID[] = "RBB"; // write here the three-digit sensor node ID

static void send_data_to_remote_node(XBeeZB &xbee, const RemoteXBeeZB &RemoteDevice, float RH, float temp)
{
    // const char data[] = "send_data_to_remote_node"; // original string
    // const uint16_t data_len = strlen(data); // original instruction
    /*
     * sprintf example
     * #include <stdio.h>
     * int main ()
     * {
     *   char buffer [50];
     *   int n, a=5, b=3;
     *   n=sprintf (buffer, "%d plus %d is %d", a, b, a+b);
     *   printf ("[%s] is a string %d chars long\n",buffer,n);
     *   return 0;
     * }
     */
    char data[30] = ""; // length more or less exactly tailored
    /* In this case the \r at the end of the message is not needed
     * since the XBee framing introduces a \r by default
     */
    uint16_t data_len = sprintf(data, "ID=%3s,RH=%6.2f,Temp=%6.2f", SENSOR_NODE_ID, RH, temp);
    /*
     * "send_data()" is a function of the XBeeZB class with two overloads:
     * TxStatus XBeeZB::send_data
     * (const RemoteXBee & remote, const uint8_t * const data, uint16_t len, bool syncr)
     * and
     * TxStatus XBeeZB::send_data
     * (const RemoteXBee & remote, uint8_t source_ep, uint8_t dest_ep,
     *  uint16_t cluster_id, uint16_t profile_id,
     *  const uint8_t * const data, uint16_t len, bool syncr)
     * where ep=endpoint, implicit source_ep=0xE0, implicit dest_ep=0xE0,
     * implicit cluster_id=0x11, Digi International profile_id=C105
     * Here we use the first (implicit) form.
     */
    const TxStatus txStatus = xbee.send_data(RemoteDevice, (const uint8_t *)data, data_len);
    if (txStatus == TxStatusSuccess)
        log_serial->printf("send_data_to_remote_node OK, actual length = %d\r\n", data_len);
    else
        log_serial->printf("send_data_to_remote_node failed with %d\r\n", (int)txStatus);
}

int main()
{
    /* Dynamic memory allocation through the "new" operator.
     * The new operator creates an object, and then 
     * returns a pointer containing the address of the memory
     * that has been allocated.
     * log_serial was previously defined as a pointer.
     */
    log_serial = new Serial(DEBUG_TX, DEBUG_RX);
    log_serial->baud(115200); // remember to define the bit rate in XBee module
    //log_serial->printf("Sample application to demo how to send unicast and broadcast data with the XBeeZB\r\n\r\n");
    log_serial->printf("Sample application to demo how to send unicast sensor data with the XBeeZB\r\n\r\n");
    log_serial->printf(XB_LIB_BANNER);

    // Definition of instance i2c of I2C class
    I2C i2c(PC_9, PA_8); // F401RE (and F303RE as well) I2C3 SDA, SCL
    //I2C i2c(PB_9, PB_8); // F401RE (and F303RE as well) I2C1 SDA, SCL

    // Instance definition using Si7021 class
    Si7021 RH_T_sensor(&i2c, Si7021_ADDR);

    // Global variables for sensor
    float RH;   // Relative humidity
    float temp; //temperature in degrees C

#if defined(ENABLE_LOGGING)
    new DigiLoggerMbedSerial(log_serial, LogLevelInfo);
#endif

    /* These are the steps to send data to a remote XBee device:
     * 1) Create an XBee object.
     * 2) Initialize the XBee.
     * 3) Wait for XBee being joined to PAN
     * 4) Send data to remote device(s).
     * Here we create an XBee ZigBee local physical object
     * connected to an mbedOS board
     */
    XBeeZB xbee = XBeeZB(RADIO_TX, RADIO_RX, RADIO_RESET, NC, NC, 115200);

    /* Here we initialize the XBee ZigBee object */
    RadioStatus radioStatus = xbee.init();

    /* MBED_ASSERT is a macro which is not effective in RELEASE mode.
     * Instead, in DEBUG mode and in case the condition is false
     * it terminates the firmware (the stack shows mbed_die()), 
     */
    MBED_ASSERT(radioStatus == Success);

    /* Wait until the device has joined the network */
    log_serial->printf("Waiting for device to join the network: ");
    while (!xbee.is_joined())
    {
        wait_ms(1000);
        log_serial->printf(".");
    }
    log_serial->printf("OK\r\n");

    /* Here we define the remote ZigBee device
     * which the modem will connect to 
     */
    const RemoteXBeeZB remoteDevice = RemoteXBeeZB(REMOTE_NODE_ADDR64);

    while (true)
    {
        log_serial->printf("Node_ID:%3s\r\n", SENSOR_NODE_ID);
        //gets Relative Humidity from sensor via I2C bus
        if (RH_T_sensor.getRH(&RH))
        { // actual parameter is an address
            // or pointer therefore the RH value is available after function call
            // print RH on PC
            log_serial->printf("Relative Humidity is %6.2f %%\r\n\n", RH);
        }
        //gets ambient temperature from sensor via I2C bus
        if (RH_T_sensor.getPrevTemp(&temp))
        { // actual parameter is an address
            // or pointer therefore the temp value is available after function call
            // print temperature on PC
            log_serial->printf("Ambient Temp. associated to RH reading is %6.2f degC\r\n\n", temp);
        }
        send_data_to_remote_node(xbee, remoteDevice, RH, temp);
        log_serial->printf("\r\n");
        wait(4.0);
    }

    delete (log_serial);
}
