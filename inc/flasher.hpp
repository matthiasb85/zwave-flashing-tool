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

#ifndef INC_FLASHER
#define INC_FLASHER

#include "buffer.hpp"
#include "logger.hpp"
#include "serif.hpp"
#include <fstream>
#include <memory>

class flasher {
public:
  flasher(const char *serif, log_t log);
  ~flasher() = default;
  bool connect(unsigned char timeout);
  bool write_flash(std::vector<std::byte> &flash, size_t sector_offset);
  bool read_flash(std::vector<std::byte> &flash, size_t sector_offset);
  bool verify_flash(std::vector<std::byte> &flash);
  bool erase_flash();
  bool read_nvr(std::vector<std::byte> &nvr);
  bool set_nvr(std::vector<std::byte> &nvr);
  bool read_lockbits(std::vector<std::byte> &lockbits);
  bool set_lockbits(std::vector<std::byte> &lockbits);
  bool check_crc();
  bool disable_apm();
  bool reset();

private:
  bool _read_signature();
  bool _write_cmd(std::string out_msg, buffer &buf);
  bool _read_cmd(std::string out_msg, buffer &buf);
  bool _write_sector(unsigned int sector, std::byte *in_buf,
                     unsigned int length);
  bool _write_single_byte(unsigned int address, std::byte byte);
  bool _write_byte_block(std::byte byte1, std::byte byte2, std::byte byte3);
  bool _write_flash(unsigned int sector, unsigned int retry);
  bool _get_state_byte(std::byte &state_byte);
  bool _check_state(unsigned int retry, std::byte mask, bool state);
  bool _generate_crc32();

  serif m_serif;
  log_t m_log;
  std::vector<std::byte> m_file_buffer;
};

#endif /* INC_FLASHER */
