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

/** @defgroup ble_tmos_defines BLE TMOS
 *
 * @brief The task and message scheduler the Bluetooth stack runs on.
 *
 * WCH's stack is not polled and not interrupt-driven from the application's
 * point of view: it is driven by a tiny cooperative scheduler called TMOS.
 * An application registers one event handler, and the stack calls it with
 * event bits.  Timers and inter-task messages are TMOS services.
 *
 * The shape an application follows is therefore fixed:
 *
 * @code
 * static ble_tmos_task_id_t my_task;
 *
 * static ble_tmos_event_t my_event_handler(ble_tmos_task_id_t task,
 *                                          ble_tmos_event_t events)
 * {
 *      if (events & BLE_TMOS_EVENT_MSG) {
 *              uint8_t *msg = ble_tmos_message_receive(task);
 *              ...
 *              ble_tmos_message_free(msg);
 *              return events ^ BLE_TMOS_EVENT_MSG;
 *      }
 *      if (events & MY_EVENT) {
 *              ...
 *              return events ^ MY_EVENT;
 *      }
 *      return 0;
 * }
 *
 * my_task = ble_tmos_task_register(my_event_handler);
 * ...
 * for (;;) {
 *      ble_tmos_process();
 * }
 * @endcode
 *
 * The handler runs on the calling stack, must not block, and must return the
 * event bits it consumed.
 *
 * @ingroup BLE
 * @{
 */

#ifndef LIBOPENWCH_BLE_TMOS_H
#define LIBOPENWCH_BLE_TMOS_H

#include <libopenwch/ble/common.h>

BEGIN_DECLS

/** Task identifier returned by ble_tmos_task_register(). */
typedef tmosTaskID ble_tmos_task_id_t;

/** A bitmask of events delivered to one task. */
typedef tmosEvents ble_tmos_event_t;

/** Timer value, in units of 625 us (the BLE clock). */
typedef tmosTimer ble_tmos_timer_t;

/** The event handler an application registers. */
typedef pTaskEventHandlerFn ble_tmos_event_handler_t;

/** Set in `events` when a message is waiting; consume it with
 *  ble_tmos_message_receive(). */
#define BLE_TMOS_EVENT_MSG ((ble_tmos_event_t)SYS_EVENT_MSG)

/** Returned by ble_tmos_task_register() on failure. */
#define BLE_TMOS_INVALID_TASK_ID ((ble_tmos_task_id_t)INVALID_TASK_ID)

/** One TMOS tick is 625 microseconds, the Bluetooth LE clock. */
#define BLE_TMOS_TICK_US 625u

/**
 * Register an event handler and get a task id for it.
 * @param handler called with this task's id and its pending event bits
 * @return the task id, or BLE_TMOS_INVALID_TASK_ID if none is free
 */
ble_tmos_task_id_t ble_tmos_task_register(ble_tmos_event_handler_t handler);

/**
 * Run one round of the scheduler.  Call this from the main loop, forever.
 * It returns when there is nothing left to do, so a bare
 * `for (;;) ble_tmos_process();` is the normal shape.
 */
void ble_tmos_process(void);

/**
 * Queue an event to a task, including a task other than the caller's.
 * @return BLE_STATUS_SUCCESS, or BLE_STATUS_FAILURE if the task is unknown
 */
ble_status_t ble_tmos_event_set(ble_tmos_task_id_t task,
				ble_tmos_event_t event);

/**
 * Start a one-shot timer that sets @p event on @p task after @p timeout
 * ticks (625 us each).
 * @return TRUE if the timer was armed
 */
uint8_t ble_tmos_task_start(ble_tmos_task_id_t task,
			    ble_tmos_event_t event,
			    ble_tmos_timer_t timeout);

/**
 * Cancel a timer armed by ble_tmos_task_start().
 * @return BLE_STATUS_SUCCESS if a timer was cancelled
 */
ble_status_t ble_tmos_task_stop(ble_tmos_task_id_t task,
				ble_tmos_event_t event);

/**
 * Take the message that BLE_TMOS_EVENT_MSG announced, if any.
 * @return the message, or NULL.  Free it with ble_tmos_message_free().
 */
uint8_t *ble_tmos_message_receive(ble_tmos_task_id_t task);

/** Release a message obtained from ble_tmos_message_receive(). */
ble_status_t ble_tmos_message_free(uint8_t *message);

/**
 * Allocate a message to send to another task.  The first byte is the
 * destination task id and the second is the event, as in WCH's SDK; use
 * ble_tmos_message_header() to fill them.
 * @return the message buffer, or NULL if the heap is exhausted
 */
uint8_t *ble_tmos_message_allocate(uint16_t length);

/**
 * The destination-task / event pair at the start of an allocated message.
 * The stack reads these two bytes to decide where the message goes.
 */
typedef struct {
	uint8_t task;  /**< destination task id */
	uint8_t event; /**< event bit to set on that task */
} ble_tmos_message_header_t;

/**
 * Copy @p length bytes, from the stack's own heap-agnostic implementation.
 * Provided because the stack exposes it and it is useful inside event
 * handlers, not because it is faster than the compiler's.
 */
void ble_tmos_memcpy(void *dest, const void *src, uint32_t length);

/** Fill @p length bytes with @p value. */
void ble_tmos_memset(void *dest, uint8_t value, uint32_t length);

/**
 * Compare @p length bytes.
 * @return non-zero when the two ranges are equal
 */
uint8_t ble_tmos_memcmp(const void *a, const void *b, uint32_t length);

END_DECLS

/**@}*/

#endif /* LIBOPENWCH_BLE_TMOS_H */
