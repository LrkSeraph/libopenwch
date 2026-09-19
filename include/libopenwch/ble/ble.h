/*
 * This file is part of the libopenwch project.
 *
 * Copyright (C) 2025 libopenwch contributors
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

/** @defgroup ble_file Bluetooth LE
 *
 * @brief Bluetooth LE peripheral support for the CH582/CH583.
 *
 * This is a thin layer over WCH's closed-source Bluetooth stack,
 * `LIBCH58xBLE.a`.  It does not reimplement Bluetooth and it is not a
 * protocol stack of its own: it renames and documents the parts of WCH's API
 * that a peripheral needs, and it leaves the stack's event-driven design
 * intact.
 *
 * Every application on this layer has the same shape:
 *
 * @code
 * BLE_HEAP_DEFINE(ble_heap, BLE_HEAP_SIZE_DEFAULT);
 *
 * static ble_tmos_task_id_t app_task;
 *
 * static ble_tmos_event_t app_event_handler(ble_tmos_task_id_t task,
 *                                          ble_tmos_event_t events)
 * {
 *      if (events & BLE_TMOS_EVENT_MSG) {
 *              uint8_t *msg = ble_tmos_message_receive(task);
 *              gapEventHdr_t *hdr = (gapEventHdr_t *)msg;
 *
 *              switch (hdr->opcode) {
 *              case BLE_GAP_EVENT_LINK_ESTABLISHED:
 *                      ...
 *                      break;
 *              case BLE_GAP_EVENT_LINK_TERMINATED:
 *                      ...
 *                      break;
 *              default:
 *                      break;
 *              }
 *              ble_tmos_message_free(msg);
 *              return events ^ BLE_TMOS_EVENT_MSG;
 *      }
 *      return 0;
 * }
 *
 * int main(void)
 * {
 *      static const uint8_t mac[6] = { ... };
 *      ble_config_t cfg;
 *
 *      ble_config_default(&cfg);
 *      cfg.heap = ble_heap;
 *      cfg.heap_size = sizeof(ble_heap);
 *      cfg.mac = mac;
 *      if (ble_init(&cfg) != 0) {
 *              for (;;) {
 *              }
 *      }
 *
 *      ble_gap_role_peripheral_init();
 *      app_task = ble_tmos_task_register(app_event_handler);
 *      ... set advertising data and enable advertising ...
 *
 *      for (;;) {
 *              ble_tmos_process();
 *      }
 * }
 * @endcode
 *
 * Linking.  The stack is closed-source and is *not* part of
 * `libopenwch_ch5xx58x.a`: an application that uses this layer must also link
 * `lib/ble/wch/LIBCH58xBLE.a` and `libopenwch_mini_libc_ch5xx58x.a` (the
 * stack needs `memcpy`).  Application projects built from `template/` get
 * this by setting `LIBOPENWCH_BLE=1`.  The binary stays under WCH's Apache-2.0
 * licence; see NOTICE.
 *
 * What is not here.  Central, observer and broadcaster roles, the bonding
 * manager, OTA and the mesh stack are all absent.  They are reachable through
 * WCH's own names in the vendor header, which is installed alongside this
 * layer.
 *
 * @ingroup BLE
 * @{
 */

#ifndef LIBOPENWCH_BLE_BLE_H
#define LIBOPENWCH_BLE_BLE_H

#include <libopenwch/qingke/common.h>
#include <libopenwch/ble/common.h>
#include <libopenwch/ble/tmos.h>
#include <libopenwch/ble/gap.h>
#include <libopenwch/ble/gatt.h>

BEGIN_DECLS

/** The stack refuses to start with a heap smaller than this. */
#define BLE_HEAP_SIZE_MIN (4 * 1024)

/** Heap size WCH's own examples use. */
#define BLE_HEAP_SIZE_DEFAULT (6 * 1024)

/*
 * Defaults for the controller's buffers and link counts.  These live in WCH's
 * application-side config.h and not in the vendor library header, so the
 * values are repeated here; they are the ones WCH's examples ship with.
 */

/** Maximum data-packet length, in octets, that the controller will buffer. */
#define BLE_BUFFER_MAX_LEN_DEFAULT 27

/** Packets the controller buffers.  Must exceed the number of links. */
#define BLE_BUFFER_NUMBER_DEFAULT 5

/** Packets to transmit per connection event. */
#define BLE_TX_EVENTS_DEFAULT 1

/** Peripheral links WCH's examples allow. */
#define BLE_PERIPHERAL_LINKS_DEFAULT 1

/** @defgroup ble_init_status ble_init() return values
 *  Non-zero values up to BLE_INIT_ERR_LAST are this layer's own; anything
 *  above that comes from `BLE_LibInit` and is documented by WCH.
 * @{ */
#define BLE_INIT_OK 0
#define BLE_INIT_ERR_HEAP 1
#define BLE_INIT_ERR_MAC 2
#define BLE_INIT_ERR_HEAP_ALIGN 3
#define BLE_INIT_ERR_LAST BLE_INIT_ERR_HEAP_ALIGN
/**@}*/

/**
 * Declare the heap the stack allocates from.  It must be word-aligned and at
 * least BLE_HEAP_SIZE_MIN bytes; the stack does not allocate it, and there is
 * no default.
 *
 * @param name  the array to declare
 * @param size  its size in bytes
 */
#define BLE_HEAP_DEFINE(name, size) uint32_t name[(size) / 4] OPENWCH_ALIGN4

/**
 * Everything ble_init() needs to start the stack.
 *
 * Start from ble_config_default() and change only what matters.
 */
typedef struct {
	/** The heap declared with BLE_HEAP_DEFINE().  Required. */
	uint32_t *heap;

	/** Its size, in bytes.  Required, and at least BLE_HEAP_SIZE_MIN. */
	uint16_t heap_size;

	/**
	 * This device's address, six bytes, **little-endian** (so the first
	 * byte is the least significant octet of the address).
	 *
	 * There is no "use the chip's own address" option.  WCH's helper for
	 * reading the address burned into the part is a macro over
	 * `libISP583.a`, another closed binary this project does not vendor,
	 * so the address is the application's to supply.
	 */
	const uint8_t *mac;

	/** Transmit power, as an LL_TX_POWEER_* value from the vendor header. */
	uint8_t tx_power;

	/** Simultaneous peripheral links to support. */
	uint8_t peripheral_links;

	/** Simultaneous central links to support.  Zero for a pure peripheral. */
	uint8_t central_links;

	/** Maximum length of a data packet, in octets.  Must exceed 69 for a
	 *  secure connection. */
	uint16_t buffer_max_len;

	/** Packets the controller may buffer.  Must exceed the link count. */
	uint8_t buffer_number;

	/** Packets to send per connection event. */
	uint8_t tx_events;

	/** Random seed source, or NULL. */
	pfnSrandCB rand_cb;

	/** Called when the stack goes idle; NULL if the part never sleeps. */
	pfnIdleCB sleep_cb;
} ble_config_t;

/**
 * Fill @p config with working defaults: a pure peripheral, one link, WCH's
 * usual buffer sizes.  The heap and MAC address are deliberately left NULL -
 * they are the two things only the application can supply.
 */
void ble_config_default(ble_config_t *config);

/**
 * Start the stack.  Call before any other BLE function.
 *
 * The MAC address is copied, so @p config need not outlive the call.
 *
 * @param config  a filled-in configuration
 * @return 0 on success, or the stack's own error code: 1 if the heap is
 *         missing or too small, 2 if the MAC address is missing, 3 if the
 *         heap address is not word-aligned, or the raw `BLE_LibInit` code for
 *         anything the stack itself rejects
 */
int ble_init(const ble_config_t *config);

/**
 * The version of the vendor stack that is linked in.  If this does not match
 * the version the vendor header was cut from, the two disagree about the
 * layout of every struct in this API, so it is worth asserting on.
 */
const char *ble_version(void);

END_DECLS

/**@}*/

#endif /* LIBOPENWCH_BLE_BLE_H */
