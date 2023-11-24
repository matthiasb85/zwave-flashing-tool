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

#include <nlohmann/json.hpp>
#include <sodium/crypto_scalarmult_curve25519.h>
#include <sys/syscall.h>
#include <unistd.h>

using json = nlohmann::json;

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

nvr_t *_get_nvr_pointer(std::vector<std::byte> &nvr) {
  auto data_ptr = nvr.data();
  return reinterpret_cast<nvr_t *>(data_ptr);
}

nvr_config_t *_get_nvr_config_pointer(std::vector<std::byte> &nvr) {
  return &(_get_nvr_pointer(nvr)->config);
}

bool _get_multi_byte_json_array(log_t log, json &j, std::string key,
                                size_t bytes, unsigned char *value) {
  bool ret = true;
  try {
    auto json_array = j.at(key);
    std::vector<unsigned char> temp_buffer;
    temp_buffer.reserve(bytes);
    for (size_t i = 0; i < bytes; i++) {
      if (json_array[i].type() == json::value_t::number_unsigned) {
        temp_buffer.push_back(json_array[i]);
      } else {
        log->debug() << "Wrong format for json entry " << key << "[" << i << "]"
                     << std::endl;
        return false;
      }
    }
    memcpy(value, temp_buffer.data(), bytes);
  } catch (const json::type_error &e) {
    ret = false;
  }
  return ret;
}

bool _get_multi_byte_json_value(log_t log, json &j, std::string key,
                                size_t bytes, unsigned char *value) {
  bool ret = true;
  try {
    auto json_entry = j.at(key);
    if (json_entry.type() == json::value_t::number_unsigned) {
      unsigned long long val = json_entry;
      for (size_t i = 0; i < bytes; i++) {
        value[i] = static_cast<unsigned char>((val >> (8 * (bytes - i - 1))) &
                                              static_cast<unsigned char>(0xFF));
      }
    } else {
      log->debug() << "Wrong format for json entry " << key << std::endl;
      ret = false;
    }
  } catch (const json::type_error &e) {
    ret = false;
  }

  for (size_t i = 0; i < bytes; i++) {
  }
  return ret;
}

bool _get_unsigned_json_value(log_t log, json &j, std::string key,
                              unsigned char *value) {
  return _get_multi_byte_json_value(log, j, key, 1, value);
}

bool nvr::generate_and_set_s2(log_t log, std::vector<std::byte> &nvr) {
  nvr_config_t *config = _get_nvr_config_pointer(nvr);

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
  nvr_t *nvr_ptr = _get_nvr_pointer(nvr);
  std::memset(&nvr_ptr->application, 0xFF, sizeof(nvr_ptr->application));
  return true;
}

bool nvr::set_preset(log_t log, std::vector<std::byte> &nvr,
                     std::vector<std::byte> &preset) {
  nvr_config_t *config = _get_nvr_config_pointer(nvr);
  std::string input(reinterpret_cast<char *>(preset.data()), preset.size());
  json j = input;

  _get_multi_byte_json_value(log, j, "rev", 1, &(config->crc_protected.rev));
  _get_multi_byte_json_value(log, j, "c_cal", 1,
                             &(config->crc_protected.c_cal));
  _get_multi_byte_json_value(log, j, "pin_swap", 1,
                             &(config->crc_protected.pin_swap));
  _get_multi_byte_json_value(log, j, "nvm_cs", 1,
                             &(config->crc_protected.nvm_cs));
  _get_multi_byte_json_value(log, j, "saw_cf", NVR_SAW_CF_SIZE,
                             (config->crc_protected.saw_cf));
  _get_multi_byte_json_value(log, j, "saw_bBandwidth", 1,
                             &(config->crc_protected.saw_bBandwidth));
  _get_multi_byte_json_value(log, j, "nvm_type", 1,
                             &(config->crc_protected.nvm_type));
  _get_multi_byte_json_value(log, j, "nvm_size", NVR_NVM_SIZE,
                             (config->crc_protected.nvm_size));
  _get_multi_byte_json_value(log, j, "nvm_page_size", NVR_NVM_PAGE_SIZE,
                             (config->crc_protected.nvm_page_size));
  _get_multi_byte_json_array(log, j, "uuid", NVR_UUID_SIZE,
                             (config->crc_protected.uuid));
  _get_multi_byte_json_value(log, j, "usb_vid", NVR_USBID_SIZE,
                             (config->crc_protected.usb_vid));
  _get_multi_byte_json_value(log, j, "usb_pid", NVR_USBID_SIZE,
                             (config->crc_protected.usb_pid));
  _get_multi_byte_json_value(log, j, "tx_cal_1", 1,
                             &(config->crc_protected.tx_cal_1));
  _get_multi_byte_json_value(log, j, "tx_cal_2", 1,
                             &(config->crc_protected.tx_cal_2));

  return true;
}

bool nvr::export_preset(log_t log, std::string &of,
                        std::vector<std::byte> &nvr) {
  nvr_config_t *config = _get_nvr_config_pointer(nvr);

  json j;
  j["rev"] = config->crc_protected.rev;
  j["c_cal"] = config->crc_protected.rev;
  j["pin_swap"] = config->crc_protected.pin_swap;
  j["nvm_cs"] = config->crc_protected.nvm_cs;
  j["saw_cf"] = config->crc_protected.saw_cf[0] << 16 |
                config->crc_protected.saw_cf[1] << 8 |
                config->crc_protected.saw_cf[2];
  j["saw_bBandwidth"] = config->crc_protected.saw_bBandwidth;
  j["nvm_type"] = config->crc_protected.nvm_type;
  j["nvm_size"] = config->crc_protected.nvm_size[1] << 8 |
                  config->crc_protected.nvm_size[2];
  j["nvm_page_size"] = config->crc_protected.nvm_page_size[1] << 8 |
                       config->crc_protected.nvm_page_size[2];
  j["uuid"] = json::array(
      {config->crc_protected.uuid[0], config->crc_protected.uuid[1],
       config->crc_protected.uuid[2], config->crc_protected.uuid[3],
       config->crc_protected.uuid[4], config->crc_protected.uuid[5],
       config->crc_protected.uuid[6], config->crc_protected.uuid[7],
       config->crc_protected.uuid[8], config->crc_protected.uuid[9],
       config->crc_protected.uuid[10], config->crc_protected.uuid[11],
       config->crc_protected.uuid[12], config->crc_protected.uuid[13],
       config->crc_protected.uuid[14], config->crc_protected.uuid[15]});
  j["usb_vid"] =
      config->crc_protected.usb_vid[1] << 8 | config->crc_protected.usb_vid[2];
  j["usb_pid"] =
      config->crc_protected.usb_pid[1] << 8 | config->crc_protected.usb_pid[2];
  j["tx_cal_1"] = config->crc_protected.tx_cal_1;
  j["tx_cal_2"] = config->crc_protected.tx_cal_2;

  of = j.dump(4);

  return true;
}