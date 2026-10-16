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

/** @defgroup ble_adv_defines BLE advertising data
 *
 * @brief Small builder for legacy advertising and scan-response payloads.
 *
 * WCH's stack takes advertising data as a raw pointer and length.  Building
 * that buffer by hand is easy once, but it is also the part of a BLE example
 * a user is most likely to get wrong: the length byte counts the AD type plus
 * the payload, the 16-bit fields are little-endian, and the whole legacy
 * payload must fit in 31 bytes.
 *
 * This module keeps those details in one place.  A typical use is:
 *
 * @code
 * ble_adv_data_t adv;
 *
 * ble_adv_data_reset(&adv);
 * ble_adv_data_add_flags(&adv,
 *     BLE_ADV_FLAG_LE_GENERAL_DISCOVERABLE |
 *     BLE_ADV_FLAG_BR_EDR_NOT_SUPPORTED);
 * ble_adv_data_add_name(&adv, "libopenwch", true);
 * ble_adv_data_set_advert(&adv);
 * @endcode
 *
 * The low-level ble_gap_role_set_param() call remains available for callers
 * that need an AD type this builder does not cover.
 *
 * @ingroup BLE
 * @{
 */

#ifndef LIBOPENWCH_BLE_ADV_H
#define LIBOPENWCH_BLE_ADV_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <libopenwch/qingke/common.h>
#include <libopenwch/ble/common.h>

/** Maximum size of a legacy advertising or scan-response payload. */
#define BLE_ADV_DATA_MAX_LEN 31u

/** One assembled advertising or scan-response payload. */
typedef struct {
	uint8_t data[BLE_ADV_DATA_MAX_LEN]; /**< raw AD structures */
	uint8_t len;			    /**< used bytes in data[] */
} ble_adv_data_t;

/** @defgroup ble_adv_types Bluetooth Assigned Numbers: AD types
 * @{ */
#define BLE_ADV_TYPE_FLAGS 0x01
#define BLE_ADV_TYPE_INCOMPLETE_UUID16 0x02
#define BLE_ADV_TYPE_COMPLETE_UUID16 0x03
#define BLE_ADV_TYPE_SHORT_NAME 0x08
#define BLE_ADV_TYPE_COMPLETE_NAME 0x09
#define BLE_ADV_TYPE_MANUFACTURER 0xff
/**@}*/

/** @defgroup ble_adv_flags Flags AD payload bits
 * @{ */
#define BLE_ADV_FLAG_LE_LIMITED_DISCOVERABLE 0x01
#define BLE_ADV_FLAG_LE_GENERAL_DISCOVERABLE 0x02
#define BLE_ADV_FLAG_BR_EDR_NOT_SUPPORTED 0x04
#define BLE_ADV_FLAG_LE_BR_EDR_CONTROLLER 0x08
#define BLE_ADV_FLAG_LE_BR_EDR_HOST 0x10
/**@}*/

BEGIN_DECLS

/**
 * Clear @p adv so it can be built again.
 * @param adv  payload to reset
 */
void ble_adv_data_reset(ble_adv_data_t *adv);

/**
 * Append a Flags AD structure.
 * @param adv    payload being built
 * @param flags  BLE_ADV_FLAG_* bits
 * @return true when the structure fit, false when it did not
 */
bool ble_adv_data_add_flags(ble_adv_data_t *adv, uint8_t flags);

/**
 * Append a local-name AD structure.
 *
 * When @p complete is true, the full name must fit or the function returns
 * false.  When it is false, the name is truncated to the space available and
 * advertised as a shortened local name.
 *
 * @param adv       payload being built
 * @param name      NUL-terminated local name
 * @param complete  true for a complete local name, false for a shortened one
 */
bool ble_adv_data_add_name(ble_adv_data_t *adv,
			   const char *name,
			   bool complete);

/**
 * Append a 16-bit service-class UUID list entry.
 * @param adv       payload being built
 * @param uuid      UUID, host byte order
 * @param complete  true for a complete list, false for an incomplete one
 */
bool ble_adv_data_add_uuid16(ble_adv_data_t *adv, uint16_t uuid, bool complete);

/**
 * Append a Manufacturer Specific Data AD structure.
 *
 * @param adv         payload being built
 * @param company_id  Bluetooth SIG company identifier, host byte order
 * @param data        manufacturer payload, or NULL when @p len is zero
 * @param len         payload length
 */
bool ble_adv_data_add_manufacturer(ble_adv_data_t *adv,
				   uint16_t company_id,
				   const uint8_t *data,
				   uint8_t len);

/**
 * Set @p adv as the peripheral role's advertising payload.
 * @param adv  assembled advertising data
 */
ble_status_t ble_adv_data_set_advert(const ble_adv_data_t *adv);

/**
 * Set @p adv as the peripheral role's scan-response payload.
 * @param adv  assembled scan-response data
 */
ble_status_t ble_adv_data_set_scan_rsp(const ble_adv_data_t *adv);

END_DECLS

/**@}*/

#endif /* LIBOPENWCH_BLE_ADV_H */
