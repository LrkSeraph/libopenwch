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
 * @ingroup ble_file
 * @{
 *
 * Starting the vendor Bluetooth stack.
 *
 * WCH's own examples fill a `bleConfig_t` in a HAL function the application
 * is expected to provide.  That function also does three things this layer
 * does not: it configures SysTick, it checks the vendor header against the
 * library by string compare, and it reads the part's own MAC address through
 * `libISP583.a`.
 *
 * The first two are the application's business - SysTick belongs to whoever
 * owns the tick, and a version check that halts on mismatch is a policy, not
 * a mechanism.  The third needs a second closed binary, so the address is
 * taken from the caller instead.
 */

#include <libopenwch/ble/ble.h>

void ble_config_default(ble_config_t *config) {
	config->heap = 0;
	config->heap_size = 0;
	config->mac = 0;

	config->tx_power = LL_TX_POWEER_0_DBM;
	config->peripheral_links = BLE_PERIPHERAL_LINKS_DEFAULT;
	config->central_links = 0;

	config->buffer_max_len = BLE_BUFFER_MAX_LEN_DEFAULT;
	config->buffer_number = BLE_BUFFER_NUMBER_DEFAULT;
	config->tx_events = BLE_TX_EVENTS_DEFAULT;

	config->rand_cb = 0;
	config->sleep_cb = 0;
}

int ble_init(const ble_config_t *config) {
	bleConfig_t cfg;
	uint8_t i;

	if (config->heap == 0 || config->heap_size < BLE_HEAP_SIZE_MIN) {
		return BLE_INIT_ERR_HEAP;
	}

	if (config->mac == 0) {
		return BLE_INIT_ERR_MAC;
	}

	if (((uintptr_t)config->heap & 3u) != 0) {
		return BLE_INIT_ERR_HEAP_ALIGN;
	}

	/*
	 * Zero the struct through the stack's own memset rather than ours or
	 * libc's: this runs before BLE_LibInit(), and tmos_memset() is a pure
	 * memory routine with no stack state behind it.
	 */
	tmos_memset(&cfg, 0, sizeof(cfg));

	cfg.MEMAddr = (uint32_t)(uintptr_t)config->heap;
	cfg.MEMLen = config->heap_size;

	cfg.BufMaxLen = config->buffer_max_len;
	cfg.BufNumber = config->buffer_number;
	cfg.TxNumEvent = config->tx_events;
	/* The stack defaults RxNumEvent to BufNumber when it is zero. */
	cfg.RxNumEvent = 0;
	cfg.TxPower = config->tx_power;

	/* Lower two bits are the peripheral count, the rest the central one. */
	cfg.ConnectNumber = (uint8_t)((config->peripheral_links & 3u) |
				      (uint8_t)(config->central_links << 2));

	cfg.srandCB = config->rand_cb;
	cfg.sleepCB = config->sleep_cb;

	for (i = 0; i < 6; i++) {
		cfg.MacAddr[i] = config->mac[i];
	}

	/*
	 * No SNV callbacks are registered, so the stack will not try to persist
	 * bonding information: it has nowhere to put it.  A part that bonds
	 * across a power cycle needs flash access, which is a separate gap in
	 * this library.
	 */
	return (int)BLE_LibInit(&cfg);
}

const char *ble_version(void) {
	return (const char *)VER_LIB;
}

/**@}*/
