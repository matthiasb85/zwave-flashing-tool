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

#include "buffer.hpp"

std::ostream &operator<<(std::ostream &os, buffer b) {
  return os << "0x" << std::hex << std::to_integer<int>(b[0]) << " "
            << "0x" << std::hex << std::to_integer<int>(b[1]) << " "
            << "0x" << std::hex << std::to_integer<int>(b[2]) << " "
            << "0x" << std::hex << std::to_integer<int>(b[3]) << " ";
}

buffer::buffer() {
  buffer(std::byte(0), std::byte(0), std::byte(0), std::byte(0));
}

buffer::buffer(std::byte b0, std::byte b1, std::byte b2, std::byte b3) {
  m_data[0] = b0;
  m_data[1] = b1;
  m_data[2] = b2;
  m_data[3] = b3;
}

std::byte &buffer::operator[](int idx) { return m_data[idx]; }

std::byte buffer::operator[](int idx) const { return m_data[idx]; }

bool buffer::operator==(buffer &rhs) {
  return (m_data[0] == rhs[0]) && (m_data[1] == rhs[1]) &&
         (m_data[2] == rhs[2]) && (m_data[3] == rhs[3]);
}

std::byte *buffer::data() { return m_data; }
