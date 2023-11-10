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

#ifndef INC_LOGGER
#define INC_LOGGER

#include <iostream>
#include <memory>

class logger {
public:
  typedef enum {
    LOG_QUIET = 0,
    LOG_ERROR = 1,
    LOG_WARN = 2,
    LOG_INFO = 3,
    LOG_DEBUG = 4
  } log_level_t;

  logger(log_level_t level);
  ~logger() = default;
  void set_log_level(log_level_t level);
  std::ostream &msg();
  std::ostream &error();
  std::ostream &warn();
  std::ostream &info();
  std::ostream &debug();

private:
  class _null_buffer : public std::ostream {
  public:
    int overflow(int c);
  };
  std::ostream &_msg(log_level_t level, const char *c_msg);
  log_level_t m_level;
  _null_buffer m_null_buffer;
};

using log_t = std::shared_ptr<logger>;

#endif /* INC_LOGGER */
