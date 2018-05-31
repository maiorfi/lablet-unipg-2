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
 */

#ifndef __CONFIG_H_
#define __CONFIG_H_

/** Library configuration options */
//#define ENABLE_LOGGING
//#define ENABLE_ASSERTIONS
#define FRAME_BUFFER_SIZE 4
#define MAX_FRAME_PAYLOAD_LEN 128

//#define ENABLE_THREAD_SAFE_LOGGING // added in the new version

#define SYNC_OPS_TIMEOUT_MS 2000

//#define RADIO_TX PE_1   /* for DISCO_F429ZI, Serial TX pin connected to the XBee module DIN pin */
//#define RADIO_RX PE_0   /* for DISCO_F429ZI, Serial RX pin connected to the XBee module DOUT pin */
#define RADIO_TX PA_9  /* for F303RE, Serial TX pin connected to the XBee module DIN pin */
#define RADIO_RX PA_10 /* for F303RE, Serial RX pin connected to the XBee module DOUT pin */
//#define RADIO_RTS               NC /* TODO: specify your setup's Serial RTS# pin connected to the XBee module RTS# pin */
//#define RADIO_CTS               NC /* TODO: specify your setup's Serial CTS# pin connected to the XBee module CTS# pin */
/* TODO: specify your setup's GPIO (output)
 * connected to the XBee module's reset pin
 * or just write "NC"
 */
#define RADIO_RESET NC // for F303RE // NC
//#define RADIO_RESET D3 // for NUCLEO boards // NC
//#define RADIO_SLEEP_REQ         NC /* TODO: specify your setup's GPIO (output) connected to the XBee module's SLEEP_RQ pin */
//#define RADIO_ON_SLEEP          NC /* TODO: specify your setup's GPIO (input) connected to the XBee module's ON_SLEEP pin */
#define DEBUG_TX USBTX /* TODO: specify your setup's Serial TX for debugging */
#define DEBUG_RX USBRX /* TODO: specify your setup's Serial RX for debugging (optional) */

#if !defined(RADIO_TX)
#error "Please define RADIO_TX pin"
#endif

#if !defined(RADIO_RX)
#error "Please define RADIO_RX pin"
#endif

#if !defined(RADIO_RESET)
#define RADIO_RESET NC
#warning "RADIO_RESET not defined, defaulted to 'NC'"
#endif

#if defined(ENABLE_LOGGING)
#if !defined(DEBUG_TX)
#error "Please define DEBUG_TX"
#endif
#if !defined(DEBUG_RX)
#define DEBUG_RX NC
#warning "DEBUG_RX not defined, defaulted to 'NC'"
#endif
#endif

#endif /* __CONFIG_H_ */
