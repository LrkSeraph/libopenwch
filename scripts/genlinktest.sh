#!/bin/sh

# This script tests the python program genlink.py, the linker script generator
# feature.
#
# Derived from libopencm3's scripts/genlinktest.sh (LGPL-3-or-later).
#
# Difference from libopencm3: the test does NOT carry its own copy of the
# device database.  It queries the real ld/devices.data, so a test can never
# silently disagree with the device tree it is supposed to validate.  The test
# name is the file name; the expected output lives in ld/tests/<name>.result.
#
# Usage: scripts/genlinktest.sh ld/tests/<device>
#
# See ld/README for more info.

# This file is part of the libopenwch project.
#
# Copyright (C) 2025 libopenwch contributors
#
# This library is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this library.  If not, see <http://www.gnu.org/licenses/>.

set -e

DATA=ld/devices.data
PY=scripts/genlink.py

if [ ! -x "$PY" ] && [ ! -f "$PY" ]; then
	echo "$0: cannot find $PY; run from the libopenwch root" >&2
	exit 1
fi

DEVICE=`basename "$1"`

# Every mode a user project or the Makefile layer may query.
{
	for mode in CPPFLAGS DEFS FAMILY SUBFAMILY MARCH MABI ZMMUL ENTRY GP HICODE CPU FPU; do
		printf '%s: ' "$mode"
		"$PY" "$DATA" "$DEVICE" "$mode" 2>/dev/null || printf '<error>'
		printf '\n'
	done
} > "$1.out"

if ! diff -q "$1.out" "$1.result" >/dev/null 2>&1; then
	exit 1
fi

# Remove the working output only when the test passed.
rm -f "$1.out"

exit 0
