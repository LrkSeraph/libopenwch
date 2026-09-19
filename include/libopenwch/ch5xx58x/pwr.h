/** @defgroup pwr_defines PWR Defines

@brief <b>Power control for the CH58x</b>

@ingroup CH5XX58X_defines

@version 1.0.0

@date 1 January 2025

LGPL License Terms @ref lgpl_license
 */
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

/**@{*/

#ifndef LIBOPENWCH_PWR_H
#define LIBOPENWCH_PWR_H

#include <libopenwch/qingke/common.h>
#include <libopenwch/ch5xx58x/memorymap.h>

/*
 * The CH58x has one power-management block, and it lives inside the SYS
 * window.  PWR_BASE is therefore an alias of SYS_BASE rather than a separate
 * peripheral base; there is no second instance and no per-instance function
 * parameter.
 */
#define PWR_BASE			SYS_BASE

#include <libopenwch/ch5xx58x/common/pwr_common_v1.h>

#endif
/**@}*/
