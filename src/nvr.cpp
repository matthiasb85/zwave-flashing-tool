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

#include "nvr.hpp"

#include <sodium/crypto_scalarmult_curve25519.h>
#include <sys/random.h>
#include <unistd.h>

constexpr uint16_t crc16_polynom = 0x1021;

void _calc_crc(log_t log, nvr_config_t *config) {
  unsigned char *data = reinterpret_cast<unsigned char *>(config);
  uint16_t crc16 = 0x1D0F;
  for (size_t i = 0; i < sizeof(config->crc_protected); i++) {
    unsigned char byte = data[i];
    for (size_t bit = 0; bit < 8; bit++) {
      unsigned char value =
          static_cast<unsigned char>((byte & (1 << (7 - bit))) >> (7 - bit));
      if (value) {
        crc16 = (crc16 << 1) ^ crc16_polynom;
      } else {
        crc16 = (crc16 << 1);
      }
    }
  }
  log->debug() << "Calculated NVR CRC: " << crc16 << std::endl;
  config->crc[0] = ((crc16 & 0xFF00) >> 8);
  config->crc[1] = ((crc16 & 0x00FF));
}

void dump_key(log_t log, const char *msg, unsigned char *array) {
  size_t i = 0;
  log->debug() << msg << std::hex << static_cast<int>(array[0])
               << static_cast<int>(array[1]) << static_cast<int>(array[2])
               << static_cast<int>(array[3]) << static_cast<int>(array[4])
               << static_cast<int>(array[5]) << static_cast<int>(array[6])
               << static_cast<int>(array[7]) << static_cast<int>(array[8])
               << static_cast<int>(array[9]) << static_cast<int>(array[10])
               << static_cast<int>(array[11]) << static_cast<int>(array[12])
               << static_cast<int>(array[13]) << static_cast<int>(array[14])
               << static_cast<int>(array[15]) << static_cast<int>(array[16])
               << static_cast<int>(array[17]) << static_cast<int>(array[18])
               << static_cast<int>(array[19]) << static_cast<int>(array[20])
               << static_cast<int>(array[21]) << static_cast<int>(array[22])
               << static_cast<int>(array[23]) << static_cast<int>(array[24])
               << static_cast<int>(array[25]) << static_cast<int>(array[26])
               << static_cast<int>(array[27]) << static_cast<int>(array[28])
               << static_cast<int>(array[29]) << static_cast<int>(array[30])
               << static_cast<int>(array[31]) << std::endl;
}

bool _calc_s2(log_t log, nvr_config_t *config) {
  ssize_t r = getrandom(config->crc_protected.s2_private_key,
                        NVR_S2_PRIVATE_KEY_SIZE, 0);
  if (r != 32) {
    log->error() << "Failed to initialize s2 private key" << std::endl;
    return false;
  }

  r = crypto_scalarmult_curve25519_base(config->crc_protected.s2_public_key,
                                        config->crc_protected.s2_private_key);
  if (r != 0) {
    log->error() << "Failed to initialize s2 public key" << std::endl;
    return false;
  }

  dump_key(log, "S2 pubkey: ", config->crc_protected.s2_public_key);
  dump_key(log, "S2 privkey: ", config->crc_protected.s2_private_key);

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