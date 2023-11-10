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

#ifndef INC_SERIF
#define INC_SERIF

#include "buffer.hpp"
#include "logger.hpp"
#include <string>

class serif {
public:
  serif(const char *if_name, log_t log);
  ~serif();
  bool open(unsigned char timeout);
  bool write_cmd(buffer &cmd);
  bool read_cmd(buffer &cmd);
  bool write_raw(std::byte *send, size_t length);
  bool read_raw(std::byte *recv, size_t length);
  size_t bytes_available();

private:
  int m_serif = 0;
  std::string m_if_name;
  log_t m_log;
};

#endif /* INC_SERIF */
