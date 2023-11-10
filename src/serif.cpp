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

#include "serif.hpp"
#include <iostream>
#include <thread>
#include <vector>

// Linux headers
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

serif::serif(const char *if_name, log_t log) : m_if_name(if_name), m_log(log) {}

serif::~serif() {
  if (m_serif) {
    close(m_serif);
  }
}

bool serif::open(unsigned char timeout) {

  if (m_serif > 0) {
    return true;
  }

  m_log->debug() << "Opening port " << m_if_name << std::endl;
  m_serif = ::open(m_if_name.c_str(), O_RDWR);
  if (m_serif == 0) {
    m_log->error() << "Failed to open " << m_if_name << std::endl;
    return false;
  }

  struct termios tty;

  if (tcgetattr(m_serif, &tty) != 0) {
    m_log->error() << "Failed to read settings " << m_if_name << std::endl;
  }

  tty.c_cflag &= ~PARENB;  // No parity
  tty.c_cflag |= CSTOPB;   // Two stop bits
  tty.c_cflag &= ~CSIZE;   // Clear all the size bits
  tty.c_cflag |= CS8;      // 8 bits per byte (most common)
  tty.c_cflag &= ~CRTSCTS; // No HW flow control

  tty.c_lflag &= ~ICANON; // Disable canonical mode
  tty.c_lflag &= ~ECHO;   // Disable echo
  tty.c_lflag &= ~ECHOE;  // Disable erasure
  tty.c_lflag &= ~ECHONL; // Disable new-line echo
  tty.c_lflag &= ~ISIG;   // Disable interpretation of INTR, QUIT and SUSP

  tty.c_iflag &= ~(IXON | IXOFF | IXANY); // No sw flow ctrl
  tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR |
                   ICRNL); // No special handling of received bytes
  tty.c_iflag |= IGNPAR;   // Ignore framing errors

  tty.c_oflag &= ~OPOST; // No interpretation of output bytes
  tty.c_oflag &= ~ONLCR; // No conv. of newline to carriage return/line feed

  tty.c_cc[VTIME] = timeout; // Wait for up to 1s
  tty.c_cc[VMIN] = 0;        // Wait always for four bytes

  cfsetspeed(&tty, B115200); // Set baud rate to 115200

  if (tcsetattr(m_serif, TCSANOW, &tty) != 0) {
    m_log->error() << "Error " << std::dec << errno << " from tcsetattr"
                   << std::endl;
    return false;
  }
  m_log->debug() << "Port open :-)" << std::endl;
  return true;
}

bool serif::write_raw(std::byte *send, size_t length) {
  for (size_t i = 0; i < length; i++) {
    if (::write(m_serif, &send[i], 1) != 1) {
      return false;
    }
  }
  return true;
}

bool serif::read_raw(std::byte *recv, size_t length) {
  size_t bytes = bytes_available();
  while (bytes < length) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    bytes = bytes_available();
  }
  if (bytes == length) {
    return (::read(m_serif, recv, length) > 0);
  } else {
    std::vector<std::byte> tmp_buffer;
    tmp_buffer.resize(bytes);
    ::read(m_serif, tmp_buffer.data(), bytes);
    recv[0] = tmp_buffer[bytes - 4];
    recv[1] = tmp_buffer[bytes - 3];
    recv[2] = tmp_buffer[bytes - 2];
    recv[3] = tmp_buffer[bytes - 1];
  }
  return true;
}

bool serif::write_cmd(buffer &cmd) {
  m_log->debug() << "Write Cmd " << cmd << std::endl;
  buffer recv;
  if (!write_raw(cmd.data(), 4)) {
    m_log->error() << "Write Cmd failed " << cmd << std::endl;
    return false;
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(1));
  if (!read_raw(recv.data(), 4)) {
    m_log->error() << "Echo Cmd failed " << recv << std::endl;
    return false;
  }
  m_log->debug() << "Read Cmd " << recv << std::endl;
  return (cmd == recv);
}

bool serif::read_cmd(buffer &cmd) {
  m_log->debug() << "Read Cmd " << cmd << std::endl;
  if (!write_raw(cmd.data(), 4)) {
    return false;
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(1));
  if (!read_raw(cmd.data(), 4)) {
    return false;
  }
  m_log->debug() << "Relpy    " << cmd << std::endl;
  return true;
}

size_t serif::bytes_available() {
  int bytes;
  ioctl(m_serif, FIONREAD, &bytes);
  m_log->debug() << "Bytes available " << bytes << std::endl;
  return static_cast<size_t>(bytes);
}
