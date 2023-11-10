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

#include "logger.hpp"
#include "string.h"

constexpr const char *msg_error = "[ERROR]";
constexpr const char *msg_warn = "[WARNING]";
constexpr const char *msg_info = "[INFO]";
constexpr const char *msg_debug = "[DEBUG]";
constexpr size_t indent_size =
    std::max(strlen(msg_error),
             std::max(strlen(msg_warn),
                      std::max(strlen(msg_info), strlen(msg_debug)))) +
    1;

logger::logger(log_level_t level) : m_level(level) {}

int logger::_null_buffer::overflow(int c) { return c; }

void logger::set_log_level(log_level_t level) { m_level = level; }

std::ostream &logger::_msg(log_level_t level, const char *c_msg) {
  if (m_level >= level) {
    std::string msg(c_msg);
    for (int i = msg.length(); i < indent_size; i++) {
      msg.append(" ");
    }

    std::cout << msg;
    return std::cout;
  } else {
    return m_null_buffer;
  }
}

std::ostream &logger::msg() { return std::cout; }

std::ostream &logger::error() { return _msg(LOG_ERROR, msg_error); }

std::ostream &logger::warn() { return _msg(LOG_WARN, msg_warn); }

std::ostream &logger::info() { return _msg(LOG_INFO, msg_info); }

std::ostream &logger::debug() { return _msg(LOG_DEBUG, msg_debug); }