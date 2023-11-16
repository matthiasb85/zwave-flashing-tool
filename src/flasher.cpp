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

#include <algorithm>
#include <array>
#include <bitset>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

#include "buffer.hpp"
#include "commands.hpp"
#include "crc.hpp"
#include "flasher.hpp"
#include "nvr.hpp"

constexpr unsigned int polling_timeout = 100;
constexpr unsigned int retry_count = 50;
constexpr unsigned int connect_count = 4;
constexpr size_t sector_size = 2048;
constexpr size_t max_sectors = 64;
constexpr size_t signature_bytes = 7;

flasher::flasher(const char *serif, log_t log)
    : m_serif(serif, log), m_log(log) {}

bool flasher::_write_cmd(std::string out_msg, buffer &buf) {
  m_log->debug() << "Flasher: " << out_msg << std::endl;
  return m_serif.write_cmd(buf);
}

bool flasher::_read_cmd(std::string out_msg, buffer &buf) {
  m_log->debug() << "Flasher: " << out_msg << std::endl;
  return m_serif.read_cmd(buf);
}

bool flasher::_write_sector(unsigned int sector, std::byte *in_buf,
                            unsigned int length) {
  unsigned int begin = 0;
  unsigned int end = length;

  while (begin < length && in_buf[begin] == static_cast<std::byte>(0xFF)) {
    begin++;
  }

  if (begin == length) {
    return true;
  }

  while (end > 0 && in_buf[end - 1] == static_cast<std::byte>(0xFF)) {
    end--;
  }

  unsigned int actual_length = end - begin;
  unsigned int offset = (actual_length - 1) % 3;

  for (int i = 0; i < offset; i++) {
    if (!_write_single_byte(begin, in_buf[begin])) {
      return false;
    }
    if (!_write_flash(sector, retry_count)) {
      return false;
    }
    begin++;
  }

  if (!_write_single_byte(begin, in_buf[begin])) {
    return false;
  }
  begin++;

  while (begin < end) {
    std::byte b0 = in_buf[begin++];
    std::byte b1 = in_buf[begin++];
    std::byte b2 = in_buf[begin++];
    if (!_write_byte_block(b0, b1, b2)) {
      return false;
    }
  }

  if (!_write_flash(sector, retry_count)) {
    return false;
  }

  return true;
}

bool flasher::_write_single_byte(unsigned int address, std::byte byte) {
  buffer cmd(CMD_WRITE_SRAM);
  cmd[1] = static_cast<std::byte>((address & 0xFF00) >> 8);
  cmd[2] = static_cast<std::byte>(address & 0x00FF);
  cmd[3] = byte;
  return _write_cmd("Write single byte to SRAM", cmd);
}

bool flasher::_write_byte_block(std::byte byte1, std::byte byte2,
                                std::byte byte3) {
  buffer cmd(CMD_CONT_WRITE_SRAM);
  cmd[1] = byte1;
  cmd[2] = byte2;
  cmd[3] = byte3;
  return _write_cmd("Write byte block to SRAM", cmd);
}

bool flasher::_write_flash(unsigned int sector, unsigned int retry) {
  buffer write(CMD_WRITE_FLASH_SECTOR);
  write[1] = static_cast<std::byte>(sector & 0xFF);
  if (!_write_cmd("Write flash", write)) {
    return false;
  }
  return _check_state(retry, CMD_FLASH_STATE_BIT, false);
}

bool flasher::_generate_crc32() {

  uint32_t crc32 =
      crc::crc32(reinterpret_cast<unsigned char *>(m_file_buffer.data()),
                 m_file_buffer.size());

  m_log->info() << "Calculated flash CRC: " << crc32 << std::endl;

  m_file_buffer.push_back(static_cast<std::byte>((crc32 & 0xFF000000) << 24));
  m_file_buffer.push_back(static_cast<std::byte>((crc32 & 0x00FF0000) << 16));
  m_file_buffer.push_back(static_cast<std::byte>((crc32 & 0x0000FF00) << 8));
  m_file_buffer.push_back(static_cast<std::byte>((crc32 & 0x000000FF)));

  return true;
}

bool flasher::_get_state_byte(std::byte &state_byte) {
  buffer check(CMD_CHECK_STATE);
  if (_read_cmd("Get state", check)) {
    state_byte = check[3];
    return true;
  }
  return false;
}

bool flasher::_check_state(unsigned int retry, std::byte mask, bool state) {
  bool done = false;
  while (retry && !done) {
    std::byte state_byte;
    if (!_get_state_byte(state_byte)) {
      return false;
    }
    done = (((state_byte & mask) == mask));
    done = (done == state);
    if (!done) {
      std::this_thread::sleep_for(std::chrono::milliseconds(polling_timeout));
    }
    retry--;
  }
  return done;
}

bool flasher::_read_signature() {
  unsigned char i = 0;
  int signature[signature_bytes];
  for (i = 0; i < signature_bytes; i++) {
    buffer read_signature(CMD_READ_SIGNATURE);
    read_signature[1] = std::byte{i};
    if (!_read_cmd("Read signature", read_signature)) {
      m_log->error() << "Failed!" << std::endl;
      return false;
    }
    signature[i] = static_cast<int>(read_signature[3]);
  }
  m_log->msg() << "Signature: ";
  for (i = 0; i < signature_bytes; i++) {
    m_log->msg() << "0x" << std::hex << signature[i] << " ";
  }
  std::cout << std::endl;
  return _check_state(10, CMD_FLASH_STATE_BIT, false);
}

bool flasher::connect(unsigned char timeout) {
  buffer cmd(CMD_ENABLE_INTERFACE);
  int cnt = 0;

  if (!m_serif.open(timeout)) {
    m_log->error() << "Failed to open serial device" << std::endl;
    return false;
  }

  while (cnt < connect_count) {
    m_log->info() << "Trying to connect" << std::endl;
    m_serif.write_raw(cmd.data(), 4);
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    size_t residual = m_serif.bytes_available();
    if (residual == 2 || residual == 4) {
      size_t o = residual - 2;
      std::vector<std::byte> recv;
      recv.resize(residual);
      if (m_serif.read_raw(recv.data(), residual)) {
        if (recv[o] == cmd[2] && recv[o + 1] == cmd[3]) {
          return _read_signature();
        }
      }
    }
    std::byte dummy{0};
    m_serif.write_raw(&dummy, 1);
    std::this_thread::sleep_for(std::chrono::milliseconds(polling_timeout));
    cnt++;
  }
  return false;
}

bool flasher::write_flash(std::vector<std::byte> &flash, size_t sector_offset) {
  std::transform(flash.begin(), flash.end(),
                 std::back_insert_iterator(m_file_buffer),
                 [](std::byte in) { return in; });

  m_file_buffer.resize(max_sectors * sector_size - 4,
                       static_cast<std::byte>(0xFF));
  _generate_crc32();

  m_log->info() << "Writing " << std::dec << m_file_buffer.size()
                << " bytes in " << max_sectors << " sectors" << std::endl;
  for (size_t sector = sector_offset; sector < max_sectors; sector++) {
    m_log->info() << "Write sector " << sector << std::endl;
    if (!_write_sector(sector, &(m_file_buffer.data()[sector * sector_size]),
                       sector_size)) {
      return false;
    }
  }

  return check_crc();
}

bool flasher::read_flash(std::vector<std::byte> &flash, size_t sector_offset) {
  size_t sector = 0;
  size_t bytes_read = 0;
  auto const append_byte = [](std::vector<std::byte> &flash,
                              size_t sector_offset, size_t cnt,
                              std::byte byte) {
    if ((cnt / sector_size) >= sector_offset) {
      flash.push_back(byte);
    }
  };
  while (sector < max_sectors) {
    buffer read_flash(CMD_READ_FLASH);
    read_flash[1] = static_cast<std::byte>(sector);
    if (!_read_cmd("Read flash", read_flash)) {
      m_log->error() << "Failed " << read_flash << std::endl;
      return false;
    }
    bytes_read++;
    append_byte(flash, sector_offset, bytes_read, read_flash[3]);
    for (size_t i = 0; i < (((sector_size * 32) - 1) / 3); i++) {
      buffer read_cont(CMD_CONT_READ_SRAM);
      if (!_read_cmd("Read cont", read_cont)) {
        m_log->error() << "Failed " << read_cont << std::endl;
        return false;
      }
      append_byte(flash, sector_offset, bytes_read++, read_cont[1]);
      append_byte(flash, sector_offset, bytes_read++, read_cont[2]);
      append_byte(flash, sector_offset, bytes_read++, read_cont[3]);
    }
    sector += 32;
  }

  return true;
}

bool flasher::verify_flash(std::vector<std::byte> &flash) {
  for (size_t i = 0; i < m_file_buffer.size(); i++) {
    if (m_file_buffer[i] != flash[i]) {
      m_log->error() << "Verify flash failed at position " << std::dec << i
                     << std::endl;
      m_log->error() << "0x" << std::hex
                     << std::to_integer<int>(m_file_buffer[i]) << " != "
                     << "0x" << std::hex << std::to_integer<int>(flash[i])
                     << std::endl;
      return false;
    }
  }
  return true;
}

bool flasher::erase_flash() {
  buffer cmd(CMD_ERASE_CHIP);
  if (!_write_cmd("Erasing flash", cmd)) {
    return false;
  }
  return _check_state(10, CMD_FLASH_STATE_BIT, false);
}

bool flasher::read_nvr(std::vector<std::byte> &nvr) {
  for (int i = NVR_START; i <= NVR_STOP; i++) {
    buffer read_nvr(CMD_READ_NVR);
    read_nvr[2] = static_cast<std::byte>(i);
    if (!_read_cmd("Read nvr", read_nvr)) {
      m_log->error() << "Failed " << read_nvr << std::endl;
      return false;
    }
    nvr.push_back(read_nvr[3]);
  }
  return true;
}

bool flasher::set_nvr(std::vector<std::byte> &nvr) {
  for (int i = NVR_START; i <= NVR_STOP; i++) {
    buffer set_nvr(CMD_SET_NVR);
    set_nvr[2] = static_cast<std::byte>(i);
    set_nvr[3] = nvr.data()[i - NVR_START];
    if (!_write_cmd("Set nvr", set_nvr)) {
      m_log->error() << "Failed " << set_nvr << std::endl;
      return false;
    }
  }
  return true;
}

bool flasher::read_lockbits(std::vector<std::byte> &lockbits) {
  unsigned char i = 0;
  for (i = 0; i < NVR_LOCK_BYTES; i++) {
    buffer read_lockbits(CMD_READ_LOCK_BITS);
    read_lockbits[1] = std::byte{i};
    if (!_read_cmd("Read lockbits", read_lockbits)) {
      m_log->error() << "Failed!" << std::endl;
      return false;
    }
    lockbits.push_back(read_lockbits[3]);
    m_log->info() << "Lockbyte[" << static_cast<int>(i) << "]: 0b"
                  << std::bitset<8>(
                         static_cast<unsigned char>(read_lockbits[3]))
                  << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(polling_timeout));
  }
  return true;
}

bool flasher::set_lockbits(std::vector<std::byte> &lockbits) {
  unsigned char i = 0;
  for (i = 0; i < NVR_LOCK_BYTES; i++) {
    buffer set_lockbits(CMD_SET_LOCK_BITS);
    set_lockbits[1] = std::byte{i};
    set_lockbits[3] = lockbits.data()[i];
    if (!_write_cmd("Write lockbits", set_lockbits)) {
      m_log->error() << "Failed!" << std::endl;
      return false;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(polling_timeout));
  }
  return true;
}

bool flasher::check_crc() {
  buffer cmd(CMD_RUN_CRC_CHECK);
  _write_cmd("Check CRC", cmd);
  _check_state(50, CMD_CRC_BUSY_BIT, false);
  std::byte state_byte;
  _get_state_byte(state_byte);
  if ((state_byte & CMD_CRC_DONE_BIT) == CMD_CRC_DONE_BIT) {
    m_log->debug() << "CRC check done" << std::endl;
    return true;
  } else if ((state_byte & CMD_CRC_FAILED_BIT) == CMD_CRC_FAILED_BIT) {
    m_log->debug() << "CRC check failed" << std::endl;
  } else {
    m_log->debug() << "CRC check failed (unknown)" << std::endl;
  }
  return false;
}

bool flasher::disable_apm() {
  buffer cmd(CMD_SET_LOCK_BITS);
  cmd[1] = std::byte(8);
  cmd[3] = std::byte(0b11111001);
  _write_cmd("Disable APM", cmd);
  return _check_state(10, CMD_FLASH_STATE_BIT, false);
}

bool flasher::reset() {
  buffer cmd(CMD_RESET_CHIP);
  _write_cmd("Reset", cmd);
  return true;
}