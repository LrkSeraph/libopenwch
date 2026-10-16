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

/** @ingroup ble_adv_defines
 * @{
 *
 * Advertising-data builder.
 *
 * Each AD structure is one length byte, one type byte, and the payload.  The
 * length byte counts the type byte plus the payload, not itself.
 */

#include <libopenwch/ble/ble.h>

static uint8_t adv_name_length(const char *name) {
	uint8_t len = 0;

	while (len < BLE_ADV_DATA_MAX_LEN && name[len] != '\0') {
		len++;
	}

	return len;
}

static bool adv_append(ble_adv_data_t *adv,
		       uint8_t type,
		       const uint8_t *data,
		       uint8_t len) {
	uint16_t total;

	if (adv == NULL) {
		return false;
	}

	total = (uint16_t)adv->len + 2u + (uint16_t)len;

	if (total > BLE_ADV_DATA_MAX_LEN) {
		return false;
	}

	adv->data[adv->len++] = (uint8_t)(1u + len);
	adv->data[adv->len++] = type;

	for (uint8_t i = 0; i < len; i++) {
		adv->data[adv->len++] = data[i];
	}

	return true;
}

void ble_adv_data_reset(ble_adv_data_t *adv) {
	if (adv != NULL) {
		adv->len = 0;
	}
}

bool ble_adv_data_add_flags(ble_adv_data_t *adv, uint8_t flags) {
	return adv_append(adv, BLE_ADV_TYPE_FLAGS, &flags, 1u);
}

bool ble_adv_data_add_name(ble_adv_data_t *adv,
			   const char *name,
			   bool complete) {
	uint8_t max_len;
	uint8_t name_len;

	if (adv == NULL || name == NULL ||
	    adv->len + 2u > BLE_ADV_DATA_MAX_LEN) {
		return false;
	}

	max_len = (uint8_t)(BLE_ADV_DATA_MAX_LEN - adv->len - 2u);
	name_len = adv_name_length(name);

	if (name_len > max_len) {
		if (complete) {
			return false;
		}

		name_len = max_len;
	}

	return adv_append(adv,
			  complete ? BLE_ADV_TYPE_COMPLETE_NAME
				   : BLE_ADV_TYPE_SHORT_NAME,
			  (const uint8_t *)name, name_len);
}

bool ble_adv_data_add_uuid16(ble_adv_data_t *adv,
			     uint16_t uuid,
			     bool complete) {
	uint8_t data[2];

	data[0] = (uint8_t)uuid;
	data[1] = (uint8_t)(uuid >> 8);

	return adv_append(adv,
			  complete ? BLE_ADV_TYPE_COMPLETE_UUID16
				   : BLE_ADV_TYPE_INCOMPLETE_UUID16,
			  data, sizeof(data));
}

bool ble_adv_data_add_manufacturer(ble_adv_data_t *adv,
				   uint16_t company_id,
				   const uint8_t *data,
				   uint8_t len) {
	uint8_t payload[2u + 27u];
	uint8_t payload_len;

	if (len > 27u || (len != 0u && data == NULL)) {
		return false;
	}

	payload[0] = (uint8_t)company_id;
	payload[1] = (uint8_t)(company_id >> 8);

	for (uint8_t i = 0; i < len; i++) {
		payload[2u + i] = data[i];
	}

	payload_len = (uint8_t)(2u + len);

	return adv_append(adv, BLE_ADV_TYPE_MANUFACTURER, payload, payload_len);
}

ble_status_t ble_adv_data_set_advert(const ble_adv_data_t *adv) {
	if (adv == NULL || adv->len > BLE_ADV_DATA_MAX_LEN) {
		return BLE_STATUS_INVALID_PARAMETER;
	}

	return ble_gap_role_set_param(BLE_GAP_ROLE_PARAM_ADVERT_DATA, adv->len,
				      adv->data);
}

ble_status_t ble_adv_data_set_scan_rsp(const ble_adv_data_t *adv) {
	if (adv == NULL || adv->len > BLE_ADV_DATA_MAX_LEN) {
		return BLE_STATUS_INVALID_PARAMETER;
	}

	return ble_gap_role_set_param(BLE_GAP_ROLE_PARAM_SCAN_RSP_DATA,
				      adv->len, adv->data);
}

/**@}*/
