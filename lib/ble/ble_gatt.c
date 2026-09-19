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

/**
 * @ingroup ble_gatt_defines
 * @{
 *
 * Forwarders onto WCH's GATT server.
 */

#include <libopenwch/ble/ble.h>

ble_status_t ble_gatt_server_add_service(uint32_t services) {
	return GATTServApp_AddService(services);
}

ble_status_t
ble_gatt_server_register_service(ble_gatt_attribute_t *attrs,
				 uint16_t num_attrs,
				 uint8_t enc_key_size,
				 ble_gatt_service_callbacks_t *callbacks) {
	return GATTServApp_RegisterService(attrs, num_attrs, enc_key_size,
					   callbacks);
}

void ble_gatt_server_char_cfg_init(ble_conn_handle_t conn,
				   ble_gatt_char_cfg_t *table) {
	GATTServApp_InitCharCfg(conn, table);
}

uint16_t ble_gatt_server_char_cfg_read(ble_conn_handle_t conn,
				       ble_gatt_char_cfg_t *table) {
	return GATTServApp_ReadCharCfg(conn, table);
}

ble_status_t ble_gatt_server_process_ccc_write(ble_conn_handle_t conn,
					       ble_gatt_attribute_t *attr,
					       uint8_t *value,
					       uint16_t len,
					       uint16_t offset,
					       uint16_t valid_cfg) {
	return GATTServApp_ProcessCCCWriteReq(conn, attr, value, len, offset,
					      valid_cfg);
}

/**@}*/
