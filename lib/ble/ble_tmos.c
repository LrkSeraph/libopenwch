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
 * @ingroup ble_tmos_defines
 * @{
 *
 * One-line forwarders onto WCH's TMOS.  They exist to give the scheduler a
 * name in this project's style and a place to be documented, not to do work.
 */

#include <libopenwch/ble/ble.h>

ble_tmos_task_id_t ble_tmos_task_register(ble_tmos_event_handler_t handler) {
	return TMOS_ProcessEventRegister(handler);
}

void ble_tmos_process(void) {
	TMOS_SystemProcess();
}

ble_status_t ble_tmos_event_set(ble_tmos_task_id_t task,
				ble_tmos_event_t event) {
	return tmos_set_event(task, event);
}

uint8_t ble_tmos_task_start(ble_tmos_task_id_t task,
			    ble_tmos_event_t event,
			    ble_tmos_timer_t timeout) {
	return (uint8_t)tmos_start_task(task, event, timeout);
}

ble_status_t ble_tmos_task_stop(ble_tmos_task_id_t task,
				ble_tmos_event_t event) {
	return tmos_stop_task(task, event);
}

uint8_t *ble_tmos_message_receive(ble_tmos_task_id_t task) {
	return tmos_msg_receive(task);
}

ble_status_t ble_tmos_message_free(uint8_t *message) {
	return tmos_msg_deallocate(message);
}

uint8_t *ble_tmos_message_allocate(uint16_t length) {
	return tmos_msg_allocate(length);
}

void ble_tmos_memcpy(void *dest, const void *src, uint32_t length) {
	tmos_memcpy(dest, src, length);
}

void ble_tmos_memset(void *dest, uint8_t value, uint32_t length) {
	tmos_memset(dest, value, length);
}

uint8_t ble_tmos_memcmp(const void *a, const void *b, uint32_t length) {
	return (uint8_t)tmos_memcmp(a, b, length);
}

/**@}*/
