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

/** @addtogroup dbgmcu_file DBGMCU
 *
 * @ingroup CH32V0
 *
 * @brief <b>Debug MCU for the CH32V00x</b>
 *
 * @version 1.0.0
 *
 * @date 1 January 2025
 *
 * The identifier word carries the device part number in its low half and the
 * revision in its high half.  The configuration word holds the low-power and
 * peripheral "stop while the core is halted" bits.
 *
 * LGPL License Terms @ref lgpl_license
 */
/**@{*/

#include <libopenwch/ch32v0/dbgmcu.h>
#include <libopenwch/qingke/csr.h>

/* CSR 0x7c0: the CH32V00x debug control register (WCH calls it CFGR0). */
#define DBGMCU_CSR_CONTROL 0x7c0

uint32_t dbgmcu_get_revision_id(void) {
	return DBGMCU_ID_WORD >> DBGMCU_ID_REVISION_SHIFT;
}

uint32_t dbgmcu_get_device_id(void) {
	return DBGMCU_ID_WORD & DBGMCU_ID_DEVICE_MASK;
}

uint32_t dbgmcu_get_control(void) {
	return OPENWCH_CSR_READ_NUM(DBGMCU_CSR_CONTROL);
}

void dbgmcu_set_control(uint32_t value) {
	OPENWCH_CSR_WRITE_NUM(DBGMCU_CSR_CONTROL, value);
}

void dbgmcu_stop_peripheral(uint32_t mask) {
	dbgmcu_set_control(dbgmcu_get_control() | mask);
}

void dbgmcu_resume_peripheral(uint32_t mask) {
	dbgmcu_set_control(dbgmcu_get_control() & ~mask);
}
/**@}*/
