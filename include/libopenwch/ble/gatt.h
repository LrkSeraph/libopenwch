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

/** @defgroup ble_gatt_defines BLE GATT server
 *
 * @brief Serving an attribute table to a connected central.
 *
 * A service is a static array of `ble_gatt_attribute_t`, terminated by the
 * stack's own bookkeeping, plus a small callback table telling the stack
 * which attributes can be read and which can be written.  Register it once,
 * at start-up, with ble_gatt_server_register_service(); after that every
 * connected client sees it.
 *
 * Notification state - whether a given client asked to be notified - is kept
 * per connection in a `ble_gatt_char_cfg_t` array that the service owns.  It
 * starts uninitialised, so call ble_gatt_server_char_cfg_init() when a client
 * connects.
 *
 * The attribute table and its macros (GATT_PRIMARY_SERVICE_UUID,
 * GATT_CHARACTERISTIC_UUID and friends) are not re-exported here: they are
 * numerous, they are pure data, and the vendor header documents them.  Use
 * those macros to build the table, then hand it to this layer.
 *
 * @ingroup BLE
 * @{
 */

#ifndef LIBOPENWCH_BLE_GATT_H
#define LIBOPENWCH_BLE_GATT_H

#include <libopenwch/ble/common.h>

BEGIN_DECLS

/** One attribute of a service. */
typedef gattAttribute_t ble_gatt_attribute_t;

/** Read/write callbacks for one service. */
typedef gattServiceCBs_t ble_gatt_service_callbacks_t;

/** Per-connection client characteristic configuration. */
typedef gattCharCfg_t ble_gatt_char_cfg_t;

/** Pass to ble_gatt_server_add_service() to register the built-in services
 *  (GAP and GATT). */
#define BLE_GATT_ALL_SERVICES ((uint32_t)GATT_ALL_SERVICES)

/** Number of attributes in a statically declared array. */
#define BLE_GATT_ATTR_COUNT(attrs)                                             \
	((uint16_t)(sizeof(attrs) / sizeof(ble_gatt_attribute_t)))

/** Client asked to be notified. */
#define BLE_GATT_CLIENT_CFG_NOTIFY ((uint16_t)GATT_CLIENT_CFG_NOTIFY)

/** Client asked to be indicated to. */
#define BLE_GATT_CLIENT_CFG_INDICATE ((uint16_t)GATT_CLIENT_CFG_INDICATE)

/** No encryption required for a registered service. */
#define BLE_GATT_ENC_KEY_SIZE_NONE ((uint8_t)0)

/**
 * Register the stack's built-in GAP and GATT services.  Call once, during
 * start-up; a peripheral that omits it will not be discoverable as a GATT
 * server.
 * @param services  BLE_GATT_ALL_SERVICES
 */
ble_status_t ble_gatt_server_add_service(uint32_t services);

/**
 * Make one application service visible to clients.
 * @param attrs      the attribute table
 * @param num_attrs  its length, normally BLE_GATT_ATTR_COUNT(attrs)
 * @param enc_key_size  minimum encryption key size, or
 *                      BLE_GATT_ENC_KEY_SIZE_NONE
 * @param callbacks  the service's read/write callbacks
 */
ble_status_t
ble_gatt_server_register_service(ble_gatt_attribute_t *attrs,
				 uint16_t num_attrs,
				 uint8_t enc_key_size,
				 ble_gatt_service_callbacks_t *callbacks);

/**
 * Reset one connection's notification state.  Call from the
 * BLE_GAP_EVENT_LINK_ESTABLISHED handler, before any notification is sent.
 * @param conn  the connection, or BLE_CONN_HANDLE_INVALID for all
 * @param table the service's configuration array
 */
void ble_gatt_server_char_cfg_init(ble_conn_handle_t conn,
				   ble_gatt_char_cfg_t *table);

/**
 * Read one connection's notification state.
 * @return a bitmask of BLE_GATT_CLIENT_CFG_* bits
 */
uint16_t ble_gatt_server_char_cfg_read(ble_conn_handle_t conn,
				       ble_gatt_char_cfg_t *table);

/**
 * Handle a write to a Client Characteristic Configuration descriptor.
 *
 * Services that support notification do not store that descriptor
 * themselves: they pass the write here, and the stack records the client's
 * choice in the configuration array and, where appropriate, replies.
 * @param conn       the connection the write arrived on
 * @param attr       the descriptor attribute that was written
 * @param value      the bytes written
 * @param len        their length
 * @param offset     offset within the descriptor
 * @param valid_cfg  mask of BLE_GATT_CLIENT_CFG_* bits this characteristic
 *                   accepts
 */
ble_status_t ble_gatt_server_process_ccc_write(ble_conn_handle_t conn,
					       ble_gatt_attribute_t *attr,
					       uint8_t *value,
					       uint16_t len,
					       uint16_t offset,
					       uint16_t valid_cfg);

END_DECLS

/**@}*/

#endif /* LIBOPENWCH_BLE_GATT_H */
