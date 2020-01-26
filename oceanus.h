// -*- C++ -*-

#pragma once

#include <string>
#include <vector>
#include <locale>
#include <codecvt>

using namespace std;

namespace Oceanus {

enum CommandType {
  SYSTEM       = 0x00,
  STREAM       = 0x01,
  RTC          = 0x02,
  MOT          = 0x03,
  NOTIFICATION = 0x07,
  GPIO         = 0x08
};

enum SYSTEM_Command {
  SYSTEM_GetSysRdy           = 0x00,
  SYSTEM_Reset               = 0x01,
  SYSTEM_GetMCUVersion       = 0x02,
  SYSTEM_GetBootVersion      = 0x03,
  SYSTEM_GetASPVersion       = 0x04,
  SYSTEM_GetAllVersion       = 0x05,
  SYSTEM_GetModuleVersion    = 0x06
};

enum STREAM_Command {
  STREAM_Play                = 0x00,
  STREAM_Stop                = 0x02,
  STREAM_Search              = 0x03,
  STREAM_AutoSearch          = 0x04,
  STREAM_StopSearch          = 0x05,
  STREAM_SetLRMode           = 0x06,
  STREAM_GetPlayStatus       = 0x10,
  STREAM_GetPlayMode         = 0x11,
  STREAM_GetPlayIndex        = 0x12,
  STREAM_GetTotalProgram     = 0x13,
  STREAM_GetSearchProgram    = 0x14,
  STREAM_GetSignalStrength   = 0x15,
  STREAM_GetStereo           = 0x16,
  STREAM_GetRSSI             = 0x17,
  STREAM_GetSamplingRate     = 0x18,
  STREAM_SetStereoMode       = 0x20,
  STREAM_GetStereoMode       = 0x21,
  STREAM_SetVolume           = 0x22,
  STREAM_GetVolume           = 0x23,
  STREAM_SetPreset           = 0x24,
  STREAM_GetPreset           = 0x25,
  STREAM_SetEQ               = 0x26,
  STREAM_GetEQ               = 0x27,
  STREAM_SetHeadroom         = 0x28,
  STREAM_GetHeadroom         = 0x29,
  STREAM_SetSorter           = 0x2A,
  STREAM_GetSorter           = 0x2B,
  STREAM_GetProgramType      = 0x2C,
  STREAM_GetProgramName      = 0x2D,
  STREAM_GetProgramText      = 0x2E,
  STREAM_SetAnnouncement     = 0x2F,
  STREAM_GetAnnouncementSet  = 0x30,
  STREAM_GetServCompType     = 0x40,
  STREAM_GetEnsembleName     = 0x41,
  STREAM_GetServiceName      = 0x42,
  STREAM_GetDLSCmd           = 0x43,
  STREAM_IsActive            = 0x44,
  STREAM_PruneStation        = 0x45,
  STREAM_GetFrequency        = 0x46,
  STREAM_GetDataRate         = 0x47,
  STREAM_GetSignalQuality    = 0x48,
  STREAM_ProgramInfo         = 0x49,
  STREAM_DirectTuneProgram   = 0x4A,
  STREAM_GetECC              = 0x4B,
  STREAM_SetDRC              = 0x4C,
  STREAM_GetDRC              = 0x4D,
  STREAM_SetAswFilter        = 0x4E,
  STREAM_GetFigRawData       = 0x4F,
  STREAM_SetFMStereoNoiseThd = 0x62,
  STREAM_GetFMStereoNoiseThd = 0x63,
  STREAM_SetFMSeekRSSIThd    = 0x64,
  STREAM_GetFMSeekRSSIThd    = 0x65,
  STREAM_SetFMSeekNoiseThd   = 0x66,
  STREAM_GetFMSeekNoiseThd   = 0x67,
  STREAM_GetRdsPIcode        = 0x68,
  STREAM_GetBlockErrorRate   = 0x69
};

enum RTC_Command {
  RTC_SetClock               = 0x00,
  RTC_GetClock               = 0x01,
  RTC_EnableSyncClock        = 0x02,
  RTC_GetSyncClockStatus     = 0x03,
  RTC_GetClockStatus         = 0x04
};

enum MOT_Command {
  MOT_GetAppData             = 0x00,
  MOT_GetUserAppType         = 0x01,
  MOT_SetUserAppType         = 0x02
};

enum SLAVE_Command {
  SLAVE_SetNotification      = 0x00,
  SLAVE_GetNotification      = 0x01
};

enum GPIO_Command {
  GPIO_SetFunction           = 0x00,
  GPIO_SetLevel              = 0x01,
  GPIO_GetLevel              = 0x02
};

class Packet
{
public:
  static const int max_payload = 0xF000;

  const uint8_t command_type() const { return _buffer[1]; };
  const uint8_t command() const { return _buffer[2]; };
  const uint8_t sequence_number() const { return _buffer[3]; }
  const unsigned payload_length() const { return _length - 7; }
  const uint8_t* payload() const { return _buffer + 6; }

  const uint8_t* buffer() const { return _buffer; }
  const unsigned length() const { return _length; }

  const bool is_valid() const;
  void validate();

protected:
  static const int max_length = 6 + max_payload + 1; // according to documentation, 6 bytes header + at most 0x101 bytes data + end byte
  uint8_t _buffer[max_length];
  unsigned _length;
};

ostream& operator<<(ostream& os, const Packet& packet);

class Request
  : public Packet
{
public:
  Request(CommandType command_type, uint8_t command, const vector<uint8_t>& arguments);

private:
  static uint8_t _sequence_number;
};

class Response
  : public Packet
{
  friend class Radio;
};

class Radio
{
public:
  Radio(const char* const port);
  ~Radio();

  void set_volume(uint8_t volume);

  enum StereoMode {
    FORCE_MONO         = 0,
    AUTO_DETECT_STEREO = 1
  };
  void set_stereo_mode(StereoMode mode);

  void play_fm(float frequency);
  void play_dab(unsigned program_index);
  void play_i2sin();
  void play_single_tone(unsigned khz);
  void play_noise();
  void play_linein_1();
  void play_linein_2();

  void handle_status();
  void handle_mot();

  void show_status();

  wstring_convert<codecvt_utf8<wchar_t>> _wchar_to_utf8;
  string convert_string(const uint8_t* buf, unsigned length);

  enum PlayStatus {
    Playing   = 0,
    Searching = 1,
    Tuning    = 2,
    Stop      = 3
  };
  PlayStatus get_play_status() const { return _play_status; }

private:
  const unsigned _radio_timeout = 500;
  const unsigned _ready_retries = 5;

  const string _port;
  int _fd;

  ostream& _debug;

  PlayStatus _play_status;
  string _program_name;
  string _program_text;

  void open_port();
  void wait_for_readiness();
  void close_port();

  void read(uint8_t* buffer, const unsigned length);

  shared_ptr<Response> send_command(CommandType command_type, uint8_t command, const vector<uint8_t>& arguments = {});
  shared_ptr<Response> read_response();

  enum StreamPlayMode {
    DAB         = 0x00,
    FM          = 0x01,
    I2SIN       = 0x02,
    SINGLE_TONE = 0x04,
    NOISE       = 0x05,
    LINEIN_1    = 0x06,
    LINEIN_2    = 0x07
  };

  void play_stream(StreamPlayMode mode, uint32_t arg);

  class radio_timeout
    : public exception
  {
  };
};

};
