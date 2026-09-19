/** @addtogroup opa_defines OPA Defines

@brief <b>Operational Amplifier for the CH32V00x</b>

@ingroup CH32V0_defines

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

/* THIS FILE SHOULD NOT BE INCLUDED DIRECTLY, BUT ONLY VIA OPA.H */

/** @cond */
#if defined(LIBOPENWCH_OPA_H) || defined(LIBOPENWCH_OPA_COMMON_V1_H)
/** @endcond */
#ifndef LIBOPENWCH_OPA_COMMON_V1_H
#define LIBOPENWCH_OPA_COMMON_V1_H

#include <libopenwch/qingke/common.h>

/**@{*/

BEGIN_DECLS

/** Clear the op-amp configuration back to its reset state (and disable it). */
void opa_reset(uint32_t exten);

/** Select the positive and negative input pins.  Configure before enabling. */
void opa_set_inputs(uint32_t exten, opa_psel_t psel, opa_nsel_t nsel);

void opa_enable(uint32_t exten);
void opa_disable(uint32_t exten);

END_DECLS

#endif
/** @cond */
#else
#warning "opa_common_v1.h should not be included explicitly, only via opa.h"
#endif
/** @endcond */
/**@}*/
