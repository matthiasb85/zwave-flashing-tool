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

#ifndef INC_BUFFER
#define INC_BUFFER

#include <cstddef>
#include <iostream>

class buffer {
public:
  buffer();
  buffer(std::byte b0, std::byte b1, std::byte b2, std::byte b3);
  ~buffer() = default;
  std::byte *data();
  std::byte &operator[](int idx);
  std::byte operator[](int idx) const;
  bool operator==(buffer &rhs);

private:
  std::byte m_data[4];
};

std::ostream &operator<<(std::ostream &os, buffer b);

#endif /* INC_BUFFER */
