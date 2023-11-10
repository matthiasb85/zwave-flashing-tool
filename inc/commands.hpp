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

#ifndef INC_COMMANDS
#define INC_COMMANDS

#define CONVERT_CMD(a, b, c, d)                                                \
  static_cast<std::byte>((a)), static_cast<std::byte>((b)),                    \
      static_cast<std::byte>((c)), static_cast<std::byte>((d))

// clang-format off
#define CMD_ENABLE_INTERFACE CONVERT_CMD    (0xAC, 0x53, 0xAA, 0x55)
#define CMD_READ_FLASH CONVERT_CMD          (0x10, 0x00, 0xFF, 0xFF)
#define CMD_READ_SRAM CONVERT_CMD           (0x06, 0x00, 0x00, 0xFF)
#define CMD_CONT_READ_SRAM CONVERT_CMD      (0xA0, 0x00, 0x00, 0x00)
#define CMD_WRITE_SRAM CONVERT_CMD          (0x04, 0x00, 0x00, 0x00)
#define CMD_CONT_WRITE_SRAM CONVERT_CMD     (0x80, 0x00, 0x00, 0x00)
#define CMD_ERASE_CHIP CONVERT_CMD          (0x0A, 0x00, 0x00, 0x00)
#define CMD_ERASE_SECTOR CONVERT_CMD        (0x0B, 0x00, 0xFF, 0xFF)
#define CMD_WRITE_FLASH_SECTOR CONVERT_CMD  (0x20, 0x00, 0xFF, 0xFF)
#define CMD_CHECK_STATE CONVERT_CMD         (0x7F, 0xFE, 0x00, 0x00)
#define CMD_READ_SIGNATURE CONVERT_CMD      (0x30, 0x00, 0xFF, 0xFF)
#define CMD_DISABLE_EOOS CONVERT_CMD        (0xD0, 0xFF, 0xFF, 0xFF)
#define CMD_ENABLE_EOOS CONVERT_CMD         (0xC0, 0x00, 0x00, 0x00)
#define CMD_SET_LOCK_BITS CONVERT_CMD       (0xF0, 0x00, 0xFF, 0x00)
#define CMD_READ_LOCK_BITS CONVERT_CMD      (0xF1, 0x00, 0xFF, 0xFF)
#define CMD_SET_NVR CONVERT_CMD             (0xFE, 0x00, 0x00, 0x00)
#define CMD_READ_NVR CONVERT_CMD            (0xF2, 0x00, 0x00, 0xFF)
#define CMD_RUN_CRC_CHECK CONVERT_CMD       (0xC3, 0x00, 0x00, 0x00)
#define CMD_RESET_CHIP CONVERT_CMD          (0xFF, 0xFF, 0xFF, 0xFF)

#define CMD_CRC_BUSY_BIT    static_cast<std::byte>(1 << 0)
#define CMD_CRC_DONE_BIT    static_cast<std::byte>(1 << 1)
#define CMD_CRC_FAILED_BIT  static_cast<std::byte>(1 << 2)
#define CMD_FLASH_STATE_BIT static_cast<std::byte>(1 << 3)
// clang-format on

#endif /* INC_COMMANDS */
