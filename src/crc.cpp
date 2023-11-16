// Copyright (C) 2023 Matthias Beckert
//
// This file is part of zwave-flashing-tool.
//
// zwave-flashing-tool is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// zwave-flashing-tool is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with zwave-flashing-tool.  If not, see <http://www.gnu.org/licenses/>.

#include "crc.hpp"

constexpr uint16_t crc16_start = 0x1D0F;
constexpr uint16_t crc16_polynom = 0x1021;
constexpr uint32_t crc32_start = 0xFFFFFFFF;
constexpr uint32_t crc32_polynom = 0x04C11DB7;

uint16_t crc::crc16(unsigned char *data, size_t size) {
  return _crc_generic(data, size, crc16_start, crc16_polynom);
}

uint32_t crc::crc32(unsigned char *data, size_t size) {
  return _crc_generic(data, size, crc32_start, crc32_polynom);
}