
#include <oceanus.h>

#include <cstring>
#include <iostream>
#include <iomanip>
#include <cxxabi.h>

#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <unistd.h>

#include <magic_enum.hpp>

namespace Oceanus {

uint8_t Request::_sequence_number = 0;

static void
hexdump(ostream& os, const uint8_t* p, unsigned length) {
  for (int i = 0; i < length; i++) {
    os << ' ' << setbase(16) << setw(2) << setfill('0') << (unsigned) p[i];
  }
}

Radio::Radio(const char* const port)
  : _port(port),
    _fd(-1)
{
  open_port();
  wait_for_readiness();
}

Radio::~Radio()
{
  close_port();
}

void
Radio::open_port()
{
  assert(_fd == -1);

  _fd = open(_port.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);

  if (_fd == -1) {
    throw invalid_argument((string) "Cannot open serial port: " + _port + " - " + strerror(errno));
  }

  fcntl(_fd, F_SETFL, 0);

  struct termios options;

  cfmakeraw(&options);
  cfsetispeed(&options, B115200);
  cfsetospeed(&options, B115200);
  options.c_lflag = options.c_iflag = options.c_oflag = 0;
  options.c_cc[VMIN] = 0;
  options.c_cc[VTIME] = 1;

  tcsetattr(_fd, TCSANOW, &options);

  {
    int status = TIOCM_RTS;
    ioctl(_fd, TIOCMSET, &status);
  }
}

void
Radio::wait_for_readiness()
{
  int retries = _ready_retries;
  while (retries-- > 0) {
    try {
      send_command(Oceanus::SYSTEM, Oceanus::SYSTEM_GetSysRdy);
      break;
    }
    catch (radio_timeout& timeout) {
      usleep(100000);
    }
  }
}

void
Radio::close_port()
{
  close(_fd);
  _fd = -1;
}

void
Radio::read(uint8_t* buffer, const unsigned length)
{
  auto start = chrono::high_resolution_clock::now();
  unsigned remain = length;
  uint8_t* p = buffer;
  while (remain) {
    int result = ::read(_fd, p, remain);
    switch (result) {
    case -1:
      throw system_error(errno, generic_category(), "Error reading from serial port");
    case 0:
      {
        auto now = chrono::high_resolution_clock::now();
        auto elapsed = chrono::duration<double, milli>(now - start).count();
        if (elapsed > _radio_timeout) {
          throw radio_timeout();
        }
      }
      break;
    default:
      p += result;
      remain -= result;
    }
  }
}

shared_ptr<Response>
Radio::read_response()
{
  auto response = make_shared<Response>();
  uint8_t* buffer = response->_buffer;

  read(buffer, 6);
  if (buffer[0] != 0xfe) {
    return nullptr;
  }
  unsigned length = (buffer[4] << 8) | buffer[5];
  read(buffer + 6, length + 1);
  response->validate();
  return response;
}

shared_ptr<Response>
Radio::send_command(CommandType command_type, uint8_t command, const vector<uint8_t>& arguments)
{
  Request request(command_type, command, arguments);

  cout << request;
#ifdef DUMP_PACKETS
  hexdump(cout, request.buffer(), request.length());
#endif
  cout << endl;

  if (write(_fd, request.buffer(), request.length()) != request.length()) {
    throw system_error(errno, generic_category(), "Could not write to serial port");
  }

  shared_ptr<Response> response = read_response();

  if (request.sequence_number() != response->sequence_number()) {
    throw logic_error("Radio communication out of sync");
  }

  if (response) {
    cout << *response;
#ifdef DUMP_PACKETS
    hexdump(cout, response->buffer(), response->length());
#endif
    cout << endl;
  } else {
    cout << "No response" << endl;
  }

  return response;
}

const bool
Packet::is_valid() const
{
  unsigned length = (_buffer[4] << 8) | _buffer[5];
  return
    (_buffer[0] == 0xfe)
    && (length < max_payload)
    && (_length == length + 7)
    && (_buffer[6 + length] == 0xfd);
}

void
Packet::validate()
{
  unsigned length = (_buffer[4] << 8) | _buffer[5];

  if (length > max_payload) {
    throw logic_error("Response packet length too large");  // fixme better exception
  }

  _length = length + 7;

  if (!is_valid()) {
    throw logic_error("Invalid response data");
  }
}

Request::Request(CommandType command_type, uint8_t command, const vector<uint8_t>& arguments)
{
  assert(arguments.size() <= Packet::max_payload);
  _buffer[0] = 0xfe;
  _buffer[1] = command_type;
  _buffer[2] = command;
  _buffer[3] = _sequence_number++;
  _buffer[4] = arguments.size() >> 8;
  _buffer[5] = arguments.size() & 0xff;
  unsigned p = 6;
  for (auto i = arguments.begin(); i != arguments.end(); i++) {
    _buffer[p++] = *i;
  }
  _buffer[p++] = 0xfd;
  _length = p;
}

static const string
command_name(uint8_t command_type, uint8_t command)
{
  switch (command_type) {
  case SYSTEM:
    return string(magic_enum::enum_name((SYSTEM_Command) command));
  case STREAM:
    return string(magic_enum::enum_name((STREAM_Command) command));
  case RTC:
    return string(magic_enum::enum_name((RTC_Command) command));
  case MOT:
    return string(magic_enum::enum_name((MOT_Command) command));
  case GPIO:
    return string(magic_enum::enum_name((GPIO_Command) command));
  default:
    return "Unknown command type";
  }
}

ostream& operator<<(ostream& os, const Packet& packet)
{
  const uint8_t* buffer = packet.buffer();
  int status;
  os << "[" << abi::__cxa_demangle(typeid(packet).name(), 0, 0, &status) << " ";
  if (packet.is_valid()) {
    os << command_name(buffer[1], buffer[2]) << " " << packet.payload_length();
    hexdump(os, packet.buffer() + 6, packet.payload_length());
    os << "]";
  } else {
    os << "(invalid):";
    hexdump(os, packet.buffer(), 6);
    os << "]";
  }
  return os;
}

};
