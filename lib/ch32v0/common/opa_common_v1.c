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

/** @addtogroup opa_file OPA
 *
 * @ingroup CH32V0
 *
 * @brief <b>Operational Amplifier for the CH32V00x</b>
 *
 * @version 1.0.0
 *
 * @date 1 January 2025
 *
 * WCH's EVT splits the enable and the input selection across OPA_Cmd() and
 * OPA_Init(); libopenwch keeps the same split (configure, then enable)
 * because the input multiplexers should not be switched while the amplifier is
 * running.
 *
 * LGPL License Terms @ref lgpl_license
 */
/**@{*/

#include <libopenwch/ch32v0/opa.h>
#include <libopenwch/qingke/assert.h>

void opa_reset(uint32_t exten) {
	EXTEN_CTR(exten) &= ~(uint32_t)EXTEN_CTR_OPA_MASK;
}

void opa_set_inputs(uint32_t exten, opa_psel_t psel, opa_nsel_t nsel) {
	uint32_t reg = EXTEN_CTR(exten);

	openwch_assert(psel <= OPA_PSEL_CHP1);
	openwch_assert(nsel <= OPA_NSEL_CHN1);

	reg &= ~(uint32_t)(EXTEN_CTR_OPA_PSEL | EXTEN_CTR_OPA_NSEL);
	if (psel) {
		reg |= EXTEN_CTR_OPA_PSEL;
	}
	if (nsel) {
		reg |= EXTEN_CTR_OPA_NSEL;
	}
	EXTEN_CTR(exten) = reg;
}

void opa_enable(uint32_t exten) {
	EXTEN_CTR(exten) |= EXTEN_CTR_OPA_EN;
}

void opa_disable(uint32_t exten) {
	EXTEN_CTR(exten) &= ~(uint32_t)EXTEN_CTR_OPA_EN;
}
/**@}*/
