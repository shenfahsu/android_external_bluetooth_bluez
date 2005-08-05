/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2000-2001  Qualcomm Incorporated
 *  Copyright (C) 2002-2003  Maxim Krasnyansky <maxk@qualcomm.com>
 *  Copyright (C) 2002-2005  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation;
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 *  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF THIRD PARTY RIGHTS.
 *  IN NO EVENT SHALL THE COPYRIGHT HOLDER(S) AND AUTHOR(S) BE LIABLE FOR ANY
 *  CLAIM, OR ANY SPECIAL INDIRECT OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES 
 *  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN 
 *  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF 
 *  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *  ALL LIABILITY, INCLUDING LIABILITY FOR INFRINGEMENT OF ANY PATENTS, 
 *  COPYRIGHTS, TRADEMARKS OR OTHER RIGHTS, RELATING TO USE OF THIS 
 *  SOFTWARE IS DISCLAIMED.
 *
 *
 *  $Id$
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <sys/socket.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>

void baswap(bdaddr_t *dst, const bdaddr_t *src)
{
	register unsigned char *d = (unsigned char *) dst;
	register const unsigned char *s = (const unsigned char *) src;
	register int i;
	for (i = 0; i < 6; i++)
		d[i] = s[5-i];
}

char *batostr(const bdaddr_t *ba)
{
	char *str = malloc(18);
	if (!str)
		return NULL;

	sprintf(str, "%2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X",
		ba->b[0], ba->b[1], ba->b[2], 
		ba->b[3], ba->b[4], ba->b[5]);
	return str;
}

bdaddr_t *strtoba(const char *str)
{
	const char *ptr = str;
	int i;

	uint8_t *ba = malloc(sizeof(bdaddr_t));
	if (!ba)
		return NULL;

	for(i = 0; i < 6; i++) {
		ba[i] = (uint8_t) strtol(ptr, NULL, 16);
		if (i != 5 && !(ptr = strchr(ptr,':')))
			ptr = ":00:00:00:00:00";
		ptr++;
	}
	return (bdaddr_t *) ba;
}

int ba2str(const bdaddr_t *ba, char *str)
{
	uint8_t b[6];

	baswap((bdaddr_t *) b, ba);
	return sprintf(str, "%2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X",
		b[0], b[1], b[2], b[3], b[4], b[5]);
}

int str2ba(const char *str, bdaddr_t *ba)
{
	uint8_t b[6];
	const char *ptr = str;
	int i;

	for (i = 0; i < 6; i++) {
		b[i] = (uint8_t) strtol(ptr, NULL, 16);
		if (i != 5 && !(ptr = strchr(ptr, ':')))
			ptr = ":00:00:00:00:00";
		ptr++;
	}
	baswap(ba, (bdaddr_t *) b);
	return 0;
}

int ba2oui(const bdaddr_t *ba, char *str)
{
	uint8_t b[6];

	baswap((bdaddr_t *) b, ba);
	return sprintf(str, "%2.2X-%2.2X-%2.2X", b[0], b[1], b[2]);
}

int baprintf(const char *format, ...)
{
	va_list ap;
	int len;

	va_start(ap, format);
	len = vprintf(format, ap);
	va_end(ap);

	return len;
}

int bafprintf(FILE *stream, const char *format, ...)
{
	va_list ap;
	int len;

	va_start(ap, format);
	len = vfprintf(stream, format, ap);
	va_end(ap);

	return len;
}

int basprintf(char *str, const char *format, ...)
{
	va_list ap;
	int len;

	va_start(ap, format);
	len = vsnprintf(str, (~0U) >> 1, format, ap);
	va_end(ap);

	return len;
}

int basnprintf(char *str, size_t size, const char *format, ...)
{
	va_list ap;
	int len;

	va_start(ap, format);
	len = vsnprintf(str, size, format, ap);
	va_end(ap);

	return len;
}

void *bt_malloc(size_t size)
{
	return malloc(size);
}

void bt_free(void *ptr)
{
	free(ptr);
}

/* Bluetooth error codes to Unix errno mapping */
int bt_error(uint16_t code)
{
	switch (code) {
	case 0:
		return 0;
	case HCI_UNKNOWN_COMMAND:
		return EBADRQC;
	case HCI_NO_CONNECTION:
		return ENOTCONN;
	case HCI_HARDWARE_FAILURE:
		return EIO;
	case HCI_PAGE_TIMEOUT:
		return EHOSTDOWN;
	case HCI_AUTHENTICATION_FAILURE:
		return EACCES;
	case HCI_PIN_OR_KEY_MISSING:
		return EINVAL;
	case HCI_MEMORY_FULL:
		return ENOMEM;
	case HCI_CONNECTION_TIMEOUT:
		return ETIMEDOUT;
	case HCI_MAX_NUMBER_OF_CONNECTIONS:
	case HCI_MAX_NUMBER_OF_SCO_CONNECTIONS:
		return EMLINK;
	case HCI_ACL_CONNECTION_EXISTS:
		return EALREADY;
	case HCI_COMMAND_DISALLOWED:
	case HCI_TRANSACTION_COLLISION:
	case HCI_ROLE_SWITCH_PENDING:
		return EBUSY;
	case HCI_REJECTED_LIMITED_RESOURCES:
	case HCI_REJECTED_PERSONAL:
	case HCI_QOS_REJECTED:
		return ECONNREFUSED;
	case HCI_HOST_TIMEOUT:
		return ETIMEDOUT;
	case HCI_UNSUPPORTED_FEATURE:
	case HCI_QOS_NOT_SUPPORTED:
	case HCI_PAIRING_NOT_SUPPORTED:
	case HCI_CLASSIFICATION_NOT_SUPPORTED:
	case HCI_UNSUPPORTED_LMP_PARAMETER_VALUE:
	case HCI_PARAMETER_OUT_OF_RANGE:
	case HCI_QOS_UNACCEPTABLE_PARAMETER:
		return EOPNOTSUPP;
	case HCI_INVALID_PARAMETERS:
	case HCI_SLOT_VIOLATION:
		return EINVAL;
	case HCI_OE_USER_ENDED_CONNECTION:
	case HCI_OE_LOW_RESOURCES:
	case HCI_OE_POWER_OFF:
		return ECONNRESET;
	case HCI_CONNECTION_TERMINATED:
		return ECONNABORTED;
	case HCI_REPEATED_ATTEMPTS:
		return ELOOP;
	case HCI_REJECTED_SECURITY:
	case HCI_PAIRING_NOT_ALLOWED:
	case HCI_INSUFFICIENT_SECURITY:
		return EACCES;
	case HCI_UNSUPPORTED_REMOTE_FEATURE:
		return EPROTONOSUPPORT;
	case HCI_SCO_OFFSET_REJECTED:
		return ECONNREFUSED;
	case HCI_UNKNOWN_LMP_PDU:
	case HCI_INVALID_LMP_PARAMETERS:
	case HCI_LMP_ERROR_TRANSACTION_COLLISION:
	case HCI_LMP_PDU_NOT_ALLOWED:
	case HCI_ENCRYPTION_MODE_NOT_ACCEPTED:
		return EPROTO;
	default:
		return ENOSYS;
	}
}

char *bt_compidtostr(int compid)
{
	switch (compid) {
	case 0:
		return "Ericsson Technology Licensing";
	case 1:
		return "Nokia Mobile Phones";
	case 2:
		return "Intel Corp.";
	case 3:
		return "IBM Corp.";
	case 4:
		return "Toshiba Corp.";
	case 5:
		return "3Com";
	case 6:
		return "Microsoft";
	case 7:
		return "Lucent";
	case 8:
		return "Motorola";
	case 9:
		return "Infineon Technologies AG";
	case 10:
		return "Cambridge Silicon Radio";
	case 11:
		return "Silicon Wave";
	case 12:
		return "Digianswer A/S";
	case 13:
		return "Texas Instruments Inc.";
	case 14:
		return "Parthus Technologies Inc.";
	case 15:
		return "Broadcom Corporation";
	case 16:
		return "Mitel Semiconductor";
	case 17:
		return "Widcomm, Inc.";
	case 18:
		return "Zeevo, Inc.";
	case 19:
		return "Atmel Corporation";
	case 20:
		return "Mitsubishi Electric Corporation";
	case 21:
		return "RTX Telecom A/S";
	case 22:
		return "KC Technology Inc.";
	case 23:
		return "Newlogic";
	case 24:
		return "Transilica, Inc.";
	case 25:
		return "Rohde & Schwartz GmbH & Co. KG";
	case 26:
		return "TTPCom Limited";
	case 27:
		return "Signia Technologies, Inc.";
	case 28:
		return "Conexant Systems Inc.";
	case 29:
		return "Qualcomm";
	case 30:
		return "Inventel";
	case 31:
		return "AVM Berlin";
	case 32:
		return "BandSpeed, Inc.";
	case 33:
		return "Mansella Ltd";
	case 34:
		return "NEC Corporation";
	case 35:
		return "WavePlus Technology Co., Ltd.";
	case 36:
		return "Alcatel";
	case 37:
		return "Philips Semiconductors";
	case 38:
		return "C Technologies";
	case 39:
		return "Open Interface";
	case 40:
		return "R F Micro Devices";
	case 41:
		return "Hitachi Ltd";
	case 42:
		return "Symbol Technologies, Inc.";
	case 43:
		return "Tenovis";
	case 44:
		return "Macronix International Co. Ltd.";
	case 45:
		return "GCT Semiconductor";
	case 46:
		return "Norwood Systems";
	case 47:
		return "MewTel Technology Inc.";
	case 48:
		return "ST Microelectronics";
	case 49:
		return "Synopsys";
	case 50:
		return "Red-M (Communications) Ltd";
	case 51:
		return "Commil Ltd";
	case 52:
		return "Computer Access Technology Corporation (CATC)";
	case 53:
		return "Eclipse (HQ Espana) S.L.";
	case 54:
		return "Renesas Technology Corp.";
	case 55:
		return "Mobilian Corporation";
	case 56:
		return "Terax";
	case 57:
		return "Integrated System Solution Corp.";
	case 58:
		return "Matsushita Electric Industrial Co., Ltd.";
	case 59:
		return "Gennum Corporation";
	case 60:
		return "Research In Motion";
	case 61:
		return "IPextreme, Inc.";
	case 62:
		return "Systems and Chips, Inc";
	case 65535:
		return "internal use";
	default:
		return "not assigned";
	}
}
