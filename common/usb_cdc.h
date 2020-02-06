/*
 * at91sam7s USB CDC device implementation
 *
 * Copyright (c) 2012, Roel Verdult
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holders nor the
 * names of its contributors may be used to endorse or promote products
 * derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * based on the "Basic USB Example" from ATMEL (doc6123.pdf)
 *
 * @file usb_cdc.c
 * @brief
 */

#ifndef USB_CDC_H__
#define USB_CDC_H__

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "usb_cmd.h"

extern void usb_disable();
extern void usb_enable();
extern bool usb_poll();
extern bool usb_poll_validate_length();
extern bool cmd_receive(UsbCommand* cmd);
extern bool cmd_send(uint16_t cmd, uint32_t arg0, uint32_t arg1, uint32_t arg2, void* data, uint16_t datalen); // new variable sized response
extern bool cmd_send_old(uint16_t cmd, uint32_t arg0, uint32_t arg1, uint32_t arg2, void* data, uint16_t datalen); // old fixed size response

#endif // USB_CDC_H__
