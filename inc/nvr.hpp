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

#ifndef INC_NVR
#define INC_NVR

#include <cstddef>
#include <vector>

#include "logger.hpp"

#define NVR_START 0x09
#define NVR_STOP 0xFF

#define NVR_LOCK_BYTES 0x09
#define NVR_PADDING_1 0x07
#define NVR_CRC_OFFSET 0x10
#define NVR_SAW_CF_SIZE 0x03
#define NVR_NVM_SIZE 0x02
#define NVR_NVM_PAGE_SIZE 0x02
#define NVR_UUID_SIZE 0x10
#define NVR_USBID_SIZE 0x02
#define NVR_S2_PUBLIC_KEY_SIZE 0x20
#define NVR_S2_PRIVATE_KEY_SIZE 0x20
#define NVR_PADDING_2 0x0B
#define NVR_CRC16_SIZE 0x02

typedef struct {
  unsigned char lockbits[NVR_LOCK_BYTES];
  unsigned char padding1[NVR_PADDING_1];
  struct {
    unsigned char rev;
    unsigned char c_cal;
    unsigned char pin_swap;
    unsigned char nvm_cs;
    unsigned char saw_cf[NVR_SAW_CF_SIZE];
    unsigned char saw_bBandwidth;
    unsigned char nvm_type;
    unsigned char nvm_size[NVR_NVM_SIZE];
    unsigned char nvm_page_size[NVR_NVM_PAGE_SIZE];
    unsigned char uuid[NVR_UUID_SIZE];
    unsigned char usb_vid[NVR_USBID_SIZE];
    unsigned char usb_pid[NVR_USBID_SIZE];
    unsigned char tx_cal_1;
    unsigned char tx_cal_2;
    unsigned char s2_public_key[NVR_S2_PUBLIC_KEY_SIZE];
    unsigned char s2_private_key[NVR_S2_PRIVATE_KEY_SIZE];
    unsigned char padding2[NVR_PADDING_2];
  } crc_protected;
  unsigned char crc[NVR_CRC16_SIZE];
  unsigned char hw_version;
} nvr_config_t;

typedef struct {
  nvr_config_t config;
  unsigned char application[NVR_STOP - NVR_START - sizeof(nvr_config_t)];
} nvr_t;

namespace nvr {
bool generate_and_set_s2(log_t log, std::vector<std::byte> &nvr);
bool clear_application(log_t log, std::vector<std::byte> &nvr);
} // namespace nvr

#endif /* INC_NVR */
