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

#ifndef INC_CRC
#define INC_CRC

#include <cstddef>
#include <cstdint>

template <typename T>
T _crc_generic(unsigned char *data, size_t size, T crc_start, T crc_polynom) {
  T crc = crc_start;
  for (size_t i = 0; i < size; i++) {
    unsigned char byte = data[i];
    for (size_t bit = 0; bit < 8; bit++) {
      unsigned char value =
          static_cast<unsigned char>((byte & (1 << (7 - bit))) >> (7 - bit));
      if (((crc >> (sizeof(T) * 8 - 1)) & 0x0001) != value) {
        crc = (crc << 1) ^ crc_polynom;
      } else {
        crc = (crc << 1);
      }
    }
  }
  return crc;
}

namespace crc {
uint16_t crc16(unsigned char *data, size_t size);
uint32_t crc32(unsigned char *data, size_t size);
} // namespace crc

#endif /* INC_CRC */
