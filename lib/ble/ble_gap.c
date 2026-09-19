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
 * @ingroup ble_gap_defines
 * @{
 *
 * Forwarders onto WCH's GAP and peripheral-role state machine.
 *
 * `ble_gap_role_set_param()` passes its value straight through as a `void *`,
 * the way WCH's API does.  That is deliberate: the same call carries a one
 * byte flag, a two byte interval and a thirty-one byte advertising payload,
 * and the parameter id is what says which.  The typed helpers an application
 * would want on top of it belong in the application, not here.
 */

#include <libopenwch/ble/ble.h>

ble_status_t ble_gap_set_param(uint16_t param, uint16_t value) {
	return GAP_SetParamValue(param, value);
}

ble_status_t ble_gap_role_peripheral_init(void) {
	return GAPRole_PeripheralInit();
}

ble_status_t
ble_gap_role_peripheral_start_device(ble_tmos_task_id_t task,
				     ble_gap_bond_callbacks_t *bond_cbs,
				     ble_gap_role_callbacks_t *role_cbs) {
	return GAPRole_PeripheralStartDevice((uint8_t)task, bond_cbs, role_cbs);
}

ble_status_t
ble_gap_role_set_param(uint16_t param, uint16_t len, const void *value) {
	/* WCH's prototype is not const, but the value is only ever read. */
	return GAPRole_SetParameter(param, len, (void *)(uintptr_t)value);
}

ble_status_t ble_gap_role_get_param(uint16_t param, void *value) {
	return GAPRole_GetParameter(param, value);
}

ble_status_t ble_gap_role_terminate_link(ble_conn_handle_t conn) {
	return GAPRole_TerminateLink(conn);
}

ble_status_t ble_gap_role_conn_param_update(ble_conn_handle_t conn,
					    uint16_t min_interval,
					    uint16_t max_interval,
					    uint16_t latency,
					    uint16_t timeout,
					    ble_tmos_task_id_t task) {
	/*
	 * WCH's own examples pass 0 here; the parameter is unused by the
	 * peripheral role, which notifies the task that started the device.
	 */
	(void)task;
	return GAPRole_PeripheralConnParamUpdateReq(
	    conn, min_interval, max_interval, latency, timeout, 0);
}

ble_status_t ble_gap_role_update_phy(ble_conn_handle_t conn,
				     uint8_t all_phys,
				     uint8_t tx_phys,
				     uint8_t rx_phys) {
	return GAPRole_UpdatePHY(conn, all_phys, tx_phys, rx_phys, 0);
}

ble_status_t ble_gap_role_read_rssi(ble_conn_handle_t conn) {
	return GAPRole_ReadRssiCmd(conn);
}

/**@}*/
