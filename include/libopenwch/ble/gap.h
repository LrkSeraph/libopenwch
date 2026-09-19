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

/** @defgroup ble_gap_defines BLE GAP (peripheral)
 *
 * @brief Advertising, connections and the peripheral role.
 *
 * Two layers of configuration exist, and WCH's naming does not make the
 * difference obvious:
 *
 *   * `ble_gap_set_param()` sets a stack-wide GAP parameter (how often to
 *     advertise, whether to be told about scan requests).  Its ids are
 *     BLE_GAP_PARAM_*.
 *   * `ble_gap_role_set_param()` sets a parameter of the *peripheral role
 *     state machine* - the advertising payload, the connection interval to
 *     accept, whether advertising is on.  Its ids are
 *     BLE_GAP_ROLE_PARAM_*.
 *
 * The state machine drives itself once started.  The application supplies two
 * callback tables to ble_gap_role_peripheral_start_device() and receives
 * connection events as messages on its TMOS task, as BLE_GAP_EVENT_* opcodes
 * inside the BLE_TMOS_EVENT_MSG event.
 *
 * @ingroup BLE
 * @{
 */

#ifndef LIBOPENWCH_BLE_GAP_H
#define LIBOPENWCH_BLE_GAP_H

#include <libopenwch/ble/common.h>

BEGIN_DECLS

/** Peripheral-role callback table, passed to
 *  ble_gap_role_peripheral_start_device(). */
typedef gapRolesCBs_t ble_gap_role_callbacks_t;

/** Pairing/bonding callback table.  Pass a zeroed one to disable pairing. */
typedef gapBondCBs_t ble_gap_bond_callbacks_t;

/** @defgroup ble_gap_role_params Peripheral role parameters
 *  For ble_gap_role_set_param() / ble_gap_role_get_param().
 * @{ */
#define BLE_GAP_ROLE_PARAM_PROFILE_ROLE ((uint16_t)GAPROLE_PROFILEROLE)
#define BLE_GAP_ROLE_PARAM_BD_ADDR ((uint16_t)GAPROLE_BD_ADDR)
#define BLE_GAP_ROLE_PARAM_ADVERT_ENABLED ((uint16_t)GAPROLE_ADVERT_ENABLED)
#define BLE_GAP_ROLE_PARAM_ADVERT_DATA ((uint16_t)GAPROLE_ADVERT_DATA)
#define BLE_GAP_ROLE_PARAM_SCAN_RSP_DATA ((uint16_t)GAPROLE_SCAN_RSP_DATA)
#define BLE_GAP_ROLE_PARAM_ADV_EVENT_TYPE ((uint16_t)GAPROLE_ADV_EVENT_TYPE)
#define BLE_GAP_ROLE_PARAM_ADV_CHANNEL_MAP ((uint16_t)GAPROLE_ADV_CHANNEL_MAP)
#define BLE_GAP_ROLE_PARAM_ADV_FILTER_POLICY                                   \
	((uint16_t)GAPROLE_ADV_FILTER_POLICY)
#define BLE_GAP_ROLE_PARAM_STATE ((uint16_t)GAPROLE_STATE)
#define BLE_GAP_ROLE_PARAM_MIN_CONN_INTERVAL                                   \
	((uint16_t)GAPROLE_MIN_CONN_INTERVAL)
#define BLE_GAP_ROLE_PARAM_MAX_CONN_INTERVAL                                   \
	((uint16_t)GAPROLE_MAX_CONN_INTERVAL)
/**@}*/

/** @defgroup ble_gap_params Stack GAP parameters
 *  For ble_gap_set_param().
 * @{ */
#define BLE_GAP_PARAM_DISC_ADV_INT_MIN ((uint16_t)TGAP_DISC_ADV_INT_MIN)
#define BLE_GAP_PARAM_DISC_ADV_INT_MAX ((uint16_t)TGAP_DISC_ADV_INT_MAX)
#define BLE_GAP_PARAM_ADV_SCAN_REQ_NOTIFY ((uint16_t)TGAP_ADV_SCAN_REQ_NOTIFY)
/**@}*/

/** @defgroup ble_gap_role_states Peripheral role states
 *  Read with BLE_GAP_ROLE_PARAM_STATE.
 * @{ */
#define BLE_GAP_ROLE_STATE_INIT ((uint8_t)GAPROLE_INIT)
#define BLE_GAP_ROLE_STATE_STARTED ((uint8_t)GAPROLE_STARTED)
#define BLE_GAP_ROLE_STATE_ADVERTISING ((uint8_t)GAPROLE_ADVERTISING)
#define BLE_GAP_ROLE_STATE_WAITING ((uint8_t)GAPROLE_WAITING)
#define BLE_GAP_ROLE_STATE_CONNECTED ((uint8_t)GAPROLE_CONNECTED)
#define BLE_GAP_ROLE_STATE_CONNECTED_ADV ((uint8_t)GAPROLE_CONNECTED_ADV)
/**@}*/

/** @defgroup ble_gap_events GAP event opcodes
 *  Delivered inside the BLE_TMOS_EVENT_MSG event; read them from the
 *  `gapEventHdr_t` that heads the message.
 * @{ */
#define BLE_GAP_EVENT_MSG ((uint8_t)GAP_MSG_EVENT)
#define BLE_GAP_EVENT_MAKE_DISCOVERABLE_DONE                                   \
	((uint8_t)GAP_MAKE_DISCOVERABLE_DONE_EVENT)
#define BLE_GAP_EVENT_END_DISCOVERABLE_DONE                                    \
	((uint8_t)GAP_END_DISCOVERABLE_DONE_EVENT)
#define BLE_GAP_EVENT_ADV_DATA_UPDATE_DONE                                     \
	((uint8_t)GAP_ADV_DATA_UPDATE_DONE_EVENT)
#define BLE_GAP_EVENT_LINK_ESTABLISHED ((uint8_t)GAP_LINK_ESTABLISHED_EVENT)
#define BLE_GAP_EVENT_LINK_TERMINATED ((uint8_t)GAP_LINK_TERMINATED_EVENT)
#define BLE_GAP_EVENT_LINK_PARAM_UPDATE ((uint8_t)GAP_LINK_PARAM_UPDATE_EVENT)
#define BLE_GAP_EVENT_PHY_UPDATE ((uint8_t)GAP_PHY_UPDATE_EVENT)
#define BLE_GAP_EVENT_SCAN_REQUEST ((uint8_t)GAP_SCAN_REQUEST_EVENT)
#define BLE_GAP_EVENT_AUTHENTICATION_COMPLETE                                  \
	((uint8_t)GAP_AUTHENTICATION_COMPLETE_EVENT)
#define BLE_GAP_EVENT_PASSKEY_NEEDED ((uint8_t)GAP_PASSKEY_NEEDED_EVENT)
#define BLE_GAP_EVENT_BOND_COMPLETE ((uint8_t)GAP_BOND_COMPLETE_EVENT)
/**@}*/

/**
 * Set a stack-wide GAP parameter.
 * @param param  a BLE_GAP_PARAM_* id
 * @param value  the new value
 */
ble_status_t ble_gap_set_param(uint16_t param, uint16_t value);

/**
 * Initialise the peripheral role.  Call once, after ble_init().
 * @return BLE_STATUS_SUCCESS on success
 */
ble_status_t ble_gap_role_peripheral_init(void);

/**
 * Start the peripheral role: it begins advertising as soon as
 * BLE_GAP_ROLE_PARAM_ADVERT_ENABLED is set.
 * @param task        the application's TMOS task
 * @param bond_cbs    pairing callbacks, or NULL to disable pairing
 * @param role_cbs    state and RSSI callbacks; must not be NULL
 */
ble_status_t
ble_gap_role_peripheral_start_device(ble_tmos_task_id_t task,
				     ble_gap_bond_callbacks_t *bond_cbs,
				     ble_gap_role_callbacks_t *role_cbs);

/**
 * Set a peripheral-role parameter, e.g. the advertising payload.
 * @param param  a BLE_GAP_ROLE_PARAM_* id
 * @param len    size of the value, in bytes
 * @param value  pointer to the new value
 */
ble_status_t
ble_gap_role_set_param(uint16_t param, uint16_t len, const void *value);

/**
 * Read a peripheral-role parameter, e.g. the current state.
 * @param param  a BLE_GAP_ROLE_PARAM_* id
 * @param value  receives the value
 */
ble_status_t ble_gap_role_get_param(uint16_t param, void *value);

/** Drop the link.  The application is told via BLE_GAP_EVENT_LINK_TERMINATED. */
ble_status_t ble_gap_role_terminate_link(ble_conn_handle_t conn);

/**
 * Ask the central to change the connection parameters.
 * @param min_interval  minimum interval, in 1.25 ms units
 * @param max_interval  maximum interval, in 1.25 ms units
 * @param latency       slave latency, in connection events
 * @param timeout       supervision timeout, in 10 ms units
 * @param task          task to notify when the update completes
 */
ble_status_t ble_gap_role_conn_param_update(ble_conn_handle_t conn,
					    uint16_t min_interval,
					    uint16_t max_interval,
					    uint16_t latency,
					    uint16_t timeout,
					    ble_tmos_task_id_t task);

/**
 * Request a PHY change.
 * @param all_phys  0 to allow either PHY to change, 1 to change only tx,
 *                  2 for only rx, 3 to change both
 * @param tx_phys   bitmask of allowed TX PHYs (1 = 1M, 2 = 2M, 4 = coded)
 * @param rx_phys   bitmask of allowed RX PHYs
 */
ble_status_t ble_gap_role_update_phy(ble_conn_handle_t conn,
				     uint8_t all_phys,
				     uint8_t tx_phys,
				     uint8_t rx_phys);

/** Ask the controller to report RSSI through the role callback table. */
ble_status_t ble_gap_role_read_rssi(ble_conn_handle_t conn);

END_DECLS

/**@}*/

#endif /* LIBOPENWCH_BLE_GAP_H */
