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

/** @defgroup ble_common_defines BLE common definitions
 *
 * @brief Shared types for the Bluetooth LE layer.
 *
 * The BLE layer is a thin, lowercase_snake_case veneer over WCH's
 * closed-source Bluetooth stack (LIBCH58xBLE.a).  It does not reimplement the
 * radio: it gives the stack a stable, documented API in this project's style,
 * and it keeps the stack's own event-driven model rather than hiding it.  A
 * program using this layer still runs WCH's TMOS scheduler and still answers
 * WCH's callbacks.
 *
 * Because the layer is thin, the underlying header
 * <libopenwch/ble/wch/CH58xBLE_LIB.h> is installed alongside it.  Anything
 * this layer does not wrap is still reachable through WCH's own names; the
 * vendor header is the reference for those.
 *
 * @ingroup BLE
 * @{
 */

#ifndef LIBOPENWCH_BLE_COMMON_H
#define LIBOPENWCH_BLE_COMMON_H

/*
 * The stack is CH58x-only: LIBCH58xBLE.a is rv32imac and the vendor header is
 * written against the CH583 register map.  Fail clearly rather than letting
 * the vendor header produce something baffling.
 */
#if !defined(CH5XX58X)
#error The libopenwch BLE layer supports the CH58x family only (build with -DCH5XX58X)
#endif

#include <libopenwch/ble/wch/CH58xBLE_LIB.h>

BEGIN_DECLS

/** Status type returned by most BLE calls; 0 (SUCCESS) means it worked. */
typedef bStatus_t ble_status_t;

/** Connection handle.  BLE_CONN_HANDLE_INVALID means "no connection". */
typedef uint16_t ble_conn_handle_t;

/** @defgroup ble_common_status Status codes
 * @{ */
#define BLE_STATUS_SUCCESS ((ble_status_t)SUCCESS)
#define BLE_STATUS_FAILURE ((ble_status_t)FAILURE)
#define BLE_STATUS_INVALID_PARAMETER ((ble_status_t)INVALIDPARAMETER)
/**@}*/

#define BLE_CONN_HANDLE_INVALID ((ble_conn_handle_t)INVALID_CONNHANDLE)

/** 16-bit Bluetooth SIG UUID size, in bytes. */
#define BLE_UUID_SIZE_16 ((uint8_t)ATT_BT_UUID_SIZE)

END_DECLS

/**@}*/

#endif /* LIBOPENWCH_BLE_COMMON_H */
