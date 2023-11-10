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

#include <chrono>
#include <functional>
#include <iostream>
#include <thread>
#include <vector>

#include <pthread.h>
#include <unistd.h>

#include "flasher.hpp"
#include "logger.hpp"

struct {
  char *device = nullptr;
  char *flash_if = nullptr;
  char *flash_of = nullptr;
  char *nvr_if = nullptr;
  char *nvr_of = nullptr;
  unsigned char timeout = 1;
  bool reset = false;
  bool con_test = false;
  logger::log_level_t level = logger::LOG_ERROR;
} args;

void evaluate_args(log_t log) {
  if (args.device == nullptr) {
    log->msg() << "Please specify device with -d" << std::endl;
    exit(-1);
  }
}

void print_help(char *exec_name, log_t log) {
  log->msg() << "Usage: " << exec_name
             << " -d <device> -f <file> -o <file> -n <file> -t <timeout>"
             << std::endl
             << "        -d <device>    Serial device" << std::endl
             << "        -f <file>      Input hex file" << std::endl
             << "        -o <file>      Output hex file" << std::endl
             << "        -n <file>      Input NVR file" << std::endl
             << "        -m <file>      Output NVR file" << std::endl
             << "        -t <timeout>   Serial receive timeout" << std::endl
             << "        -v <level>     Log level 0..4" << std::endl;
}

bool evaluate_call(log_t log, std::string call, std::string fail,
                   std::function<bool()> cmd) {
  log->msg() << call << std::endl;
  if (!cmd()) {
    log->error() << fail << std::endl;
    return false;
  }
  return true;
}

bool dump_generic(log_t log, std::string call, std::vector<std::byte> &buffer,
                  char *filename) {
  log->msg() << call << filename << std::endl;
  std::ofstream fs;
  fs.open(filename, std::ios::binary);
  for (size_t i = 0; i < buffer.size(); i++) {
    char value = static_cast<char>(buffer[i]);
    fs.write(&value, 1);
  }
  fs.close();
  return true;
}

bool connect(log_t log, flasher &zft) {
  bool connected = false;
  while (!connected) {
    connected = zft.connect(args.timeout);
    if (!connected) {
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  }
  return true;
}

bool read_in_file(log_t log, const char *file,
                  std::vector<std::byte> &out_vector) {
  log->msg() << "Reading input file: " << file << std::endl;
  std::ifstream fs;
  fs.open(file, std::ios::binary);
  if (!fs) {
    log->error() << "Failed to open " << file << std::endl;
    return false;
  }

  fs.seekg(0, fs.end);
  int length = fs.tellg();
  fs.seekg(0, fs.beg);

  std::vector<char> temp_file_buffer;

  temp_file_buffer.resize(length);
  out_vector.reserve(length);
  fs.read(temp_file_buffer.data(), length);

  std::transform(temp_file_buffer.begin(), temp_file_buffer.end(),
                 std::back_insert_iterator(out_vector),
                 [](char in) { return static_cast<std::byte>(in); });
  return true;
}

bool read_nvr(log_t log, flasher &zft, std::vector<std::byte> &nvr) {
  std::function<bool()> cmd = [&]() { return zft.read_nvr(nvr); };
  return evaluate_call(log, "Reading NVR", "Reading NVR failed", cmd);
}

bool set_nvr(log_t log, flasher &zft, std::vector<std::byte> &nvr) {
  std::function<bool()> cmd = [&]() { return zft.set_nvr(nvr); };
  return evaluate_call(log, "Setting NVR", "Setting NVR failed", cmd);
}

bool reset_nvr(log_t log, flasher &zft, std::vector<std::byte> &nvr) {
  std::function<bool()> cmd = [&]() {
    std::fill(nvr.begin(), nvr.end(), static_cast<std::byte>(0xFF));
    return true;
  };
  return evaluate_call(log, "Reset NVR", "Reset NVR failed", cmd);
}

bool read_lockbits(log_t log, flasher &zft, std::vector<std::byte> &lockbits) {
  std::function<bool()> cmd = [&]() { return zft.read_lockbits(lockbits); };
  return evaluate_call(log, "Reading lockbits", "Reading lockbits failed", cmd);
}

bool set_lockbits(log_t log, flasher &zft, std::vector<std::byte> &lockbits) {
  std::function<bool()> cmd = [&]() { return zft.set_lockbits(lockbits); };
  return evaluate_call(log, "Setting lockbits", "Setting lockbits failed", cmd);
}

bool erase_flash(log_t log, flasher &zft) {
  std::function<bool()> cmd = [&]() { return zft.erase_flash(); };
  return evaluate_call(log, "Erasing flash", "Failed erase", cmd);
}

bool write_flash(log_t log, flasher &zft, std::vector<std::byte> &flash) {
  log->msg() << "Flashing file" << std::endl;
  if (zft.write_flash(flash, 0)) {
    log->msg() << "Flashing done" << std::endl;
    return true;
  }
  log->error() << "Flashing failed" << std::endl;
  return false;
}

bool read_flash(log_t log, flasher &zft, std::vector<std::byte> &flash) {
  std::function<bool()> cmd = [&]() { return zft.read_flash(flash, 0); };
  return evaluate_call(log, "Reading flash", "Reading flash failed", cmd);
}

bool verify_flash(log_t log, flasher &zft, std::vector<std::byte> &flash) {
  std::function<bool()> cmd = [&]() { return zft.verify_flash(flash); };
  return evaluate_call(log, "Verify flash", "Verify flash failed", cmd);
}

bool dump_flash(log_t log, std::vector<std::byte> &flash, char *filename) {
  return dump_generic(log, "Writing flash to ", flash, filename);
}

bool dump_nvr(log_t log, std::vector<std::byte> &nvr, char *filename) {
  return dump_generic(log, "Writing NVR to ", nvr, filename);
}

int main(int argc, char **argv) {
  log_t log(new logger(logger::LOG_ERROR));
  std::vector<std::byte> nvr;
  std::vector<std::byte> lockbits;
  std::vector<std::byte> i_flash;
  std::vector<std::byte> o_flash;

  int policy = SCHED_RR;
  struct sched_param param;

  bool connected = false;
  int opt;
  while ((opt = getopt(argc, argv, "d:f:o:n:m:t:v:rh?")) != -1) {
    switch (opt) {
    case 'd':
      args.device = optarg;
      break;
    case 'f':
      args.flash_if = optarg;
      break;
    case 'o':
      args.flash_of = optarg;
      break;
    case 'n':
      args.nvr_if = optarg;
      break;
    case 'm':
      args.nvr_of = optarg;
      break;
    case 't':
      args.timeout = static_cast<unsigned char>(atoi(optarg));
      break;
    case 'v': {
      const int min = static_cast<int>(logger::LOG_QUIET);
      const int max = static_cast<int>(logger::LOG_DEBUG);
      int level = std::max(min, std::min(atoi(optarg), max));
      args.level = static_cast<logger::log_level_t>(level);
      break;
    }
    case 'r':
      args.reset = true;
      break;
    case '?':
    case 'h':
      print_help(argv[0], log);
      exit(0);
      break;
    default:
      log->error() << "Unknown option: '" << char(optopt) << "'!" << std::endl;
      print_help(argv[0], log);
      exit(-1);
      break;
    }
  }

  param.sched_priority = 50;
  pthread_setschedparam(pthread_self(), policy, &param);

  evaluate_args(log);

  log->set_log_level(args.level);

  flasher zft(args.device, log);

  enum function_id {
    FUNC_CONNECT = 0,
    FUNC_READ_IN_FLASH,
    FUNC_READ_IN_NVR,
    FUNC_READ_NVR,
    FUNC_SET_NVR,
    FUNC_RESET_NVR,
    FUNC_READ_LOCKBITS,
    FUNC_SET_LOCKBITS,
    FUNC_ERASE_FLASH,
    FUNC_WRITE_FLASH,
    FUNC_READ_FLASH,
    FUNC_VERIFY_FLASH,
    FUNC_DUMP_FLASH,
    FUNC_DUMP_NVR,
    FUNC_MAX
  };

  std::function<bool()> function_table[FUNC_MAX] = {
      [log, &zft]() { return connect(log, zft); },
      [log, &i_flash]() { return read_in_file(log, args.flash_if, i_flash); },
      [log, &nvr]() { return read_in_file(log, args.nvr_if, nvr); },
      [log, &zft, &nvr]() { return read_nvr(log, zft, nvr); },
      [log, &zft, &nvr]() { return set_nvr(log, zft, nvr); },
      [log, &zft, &nvr]() { return reset_nvr(log, zft, nvr); },
      [log, &zft, &lockbits]() { return read_lockbits(log, zft, lockbits); },
      [log, &zft, &lockbits]() { return set_lockbits(log, zft, lockbits); },
      [log, &zft]() { return erase_flash(log, zft); },
      [log, &zft, &i_flash]() { return write_flash(log, zft, i_flash); },
      [log, &zft, &o_flash]() { return read_flash(log, zft, o_flash); },
      [log, &zft, &o_flash]() { return verify_flash(log, zft, o_flash); },
      [log, &o_flash]() { return dump_flash(log, o_flash, args.flash_of); },
      [log, &nvr]() { return dump_nvr(log, nvr, args.nvr_of); },
  };

  std::vector<std::function<bool()>> command_list;

  // Always connect
  command_list.push_back(function_table[FUNC_CONNECT]);

  // Read flash input file to byte vector
  if (args.flash_if) {
    command_list.push_back(function_table[FUNC_READ_IN_FLASH]);
  }

  // Read nvr input file to byte vector
  if (args.nvr_if) {
    command_list.push_back(function_table[FUNC_READ_IN_NVR]);
  }

  // Read NVR if we want to dump it or if flashing is requested and no nvr input
  // file is defined
  if ((args.nvr_of || args.flash_if) && !args.nvr_if) {
    command_list.push_back(function_table[FUNC_READ_NVR]);
  }

  if (args.reset) {
    command_list.push_back(function_table[FUNC_RESET_NVR]);
  }

  // Flashing is requested part 1
  if (args.flash_if) {
    command_list.push_back(function_table[FUNC_READ_LOCKBITS]);
    command_list.push_back(function_table[FUNC_ERASE_FLASH]);
  }

  // Write NVR if we have an input file or flashing is requested
  if (args.nvr_if || args.flash_if) {
    command_list.push_back(function_table[FUNC_SET_NVR]);
  }

  // Flashing is requested part 2
  if (args.flash_if) {
    command_list.push_back(function_table[FUNC_WRITE_FLASH]);
    command_list.push_back(function_table[FUNC_READ_FLASH]);
    command_list.push_back(function_table[FUNC_VERIFY_FLASH]);
    command_list.push_back(function_table[FUNC_SET_LOCKBITS]);
  }

  // Standalone flash read requested
  if (args.flash_of && !args.flash_if) {
    command_list.push_back(function_table[FUNC_READ_FLASH]);
  }

  // Dump flash to output file
  if (args.flash_of) {
    command_list.push_back(function_table[FUNC_DUMP_FLASH]);
  }

  // Dump NVR to output file
  if (args.nvr_of) {
    command_list.push_back(function_table[FUNC_DUMP_NVR]);
  }

  // Run all requested commands
  for (auto &command : command_list) {
    if (!command()) {
      return 1;
    }
  }

  return 0;
}