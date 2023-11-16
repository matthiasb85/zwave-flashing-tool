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

#include <cstring>

#include "crc.hpp"
#include "nvr.hpp"

#include <sodium/crypto_scalarmult_curve25519.h>
#include <sys/syscall.h>
#include <unistd.h>

void _calc_crc(log_t log, nvr_config_t *config) {

  uint32_t crc16 =
      crc::crc16(reinterpret_cast<unsigned char *>(&config->crc_protected),
                 sizeof(config->crc_protected));

  log->info() << "Calculated NVR CRC: " << crc16 << std::endl;
  config->crc[0] = ((crc16 & 0xFF00) >> 8);
  config->crc[1] = ((crc16 & 0x00FF));
}

void _dump_key(log_t log, const char *msg, unsigned char *array) {
  size_t i = 0;
  log->info()
      << msg << std::hex << "0x" << static_cast<int>(array[0]) << ", 0x"
      << static_cast<int>(array[1]) << ", 0x" << static_cast<int>(array[2])
      << ", 0x" << static_cast<int>(array[3]) << ", 0x"
      << static_cast<int>(array[4]) << ", 0x" << static_cast<int>(array[5])
      << ", 0x" << static_cast<int>(array[6]) << ", 0x"
      << static_cast<int>(array[7]) << ", 0x" << static_cast<int>(array[8])
      << ", 0x" << static_cast<int>(array[9]) << ", 0x"
      << static_cast<int>(array[10]) << ", 0x" << static_cast<int>(array[11])
      << ", 0x" << static_cast<int>(array[12]) << ", 0x"
      << static_cast<int>(array[13]) << ", 0x" << static_cast<int>(array[14])
      << ", 0x" << static_cast<int>(array[15]) << ", 0x"
      << static_cast<int>(array[16]) << ", 0x" << static_cast<int>(array[17])
      << ", 0x" << static_cast<int>(array[18]) << ", 0x"
      << static_cast<int>(array[19]) << ", 0x" << static_cast<int>(array[20])
      << ", 0x" << static_cast<int>(array[21]) << ", 0x"
      << static_cast<int>(array[22]) << ", 0x" << static_cast<int>(array[23])
      << ", 0x" << static_cast<int>(array[24]) << ", 0x"
      << static_cast<int>(array[25]) << ", 0x" << static_cast<int>(array[26])
      << ", 0x" << static_cast<int>(array[27]) << ", 0x"
      << static_cast<int>(array[28]) << ", 0x" << static_cast<int>(array[29])
      << ", 0x" << static_cast<int>(array[30]) << ", 0x"
      << static_cast<int>(array[31]) << std::endl;
}

bool _calc_s2(log_t log, nvr_config_t *config) {

  int r = syscall(SYS_getrandom, config->crc_protected.s2_private_key,
                  NVR_S2_PRIVATE_KEY_SIZE, 0);
  if (r != 32) {
    log->error() << "Failed to initialize s2 private key" << std::endl;
    return false;
  }

  crypto_scalarmult_curve25519_base(config->crc_protected.s2_public_key,
                                    config->crc_protected.s2_private_key);

  _dump_key(log, "S2 pubkey:  ", config->crc_protected.s2_public_key);
  _dump_key(log, "S2 privkey: ", config->crc_protected.s2_private_key);

  return true;
}

bool nvr::generate_and_set_s2(log_t log, std::vector<std::byte> &nvr) {
  auto data_ptr = nvr.data();
  nvr_t *nvr_ptr = reinterpret_cast<nvr_t *>(data_ptr);
  nvr_config_t *config = &(nvr_ptr->config);

  // Calculate s2 key pair
  if (_calc_s2(log, config) == false) {
    return false;
  }

  // Always update to revision 2
  config->crc_protected.rev = 2;

  // Calculate crc
  _calc_crc(log, config);

  return true;
}

bool nvr::clear_application(log_t log, std::vector<std::byte> &nvr) {
  auto data_ptr = nvr.data();
  nvr_t *nvr_ptr = reinterpret_cast<nvr_t *>(data_ptr);
  std::memset(&nvr_ptr->application, 0xFF, sizeof(nvr_ptr->application));
  return true;
}