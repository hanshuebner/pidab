
#include <oceanus.h>

#include <iostream>
#include <iomanip>

#include <unistd.h>

using namespace std;

int
main(int argc, char* argv[])
{
  if (argc != 2) {
    throw invalid_argument("Missing command line argument, expecting serial device name");
  }

  Oceanus::Radio radio(argv[1]);

  radio.send_command(Oceanus::STREAM, Oceanus::STREAM_SetVolume, { 16 });
  radio.send_command(Oceanus::STREAM, Oceanus::STREAM_SetStereoMode, { 1 });

#if 0
  unsigned frequency = 102600;
  radio.send_command(Oceanus::STREAM, Oceanus::STREAM_Play,
                     { 1, (uint8_t) (frequency >> 24), (uint8_t) ((frequency >> 16) & 0xff),
                         (uint8_t) ((frequency >> 8) & 0xff), (uint8_t) (frequency & 0xff)});
#endif
  radio.send_command(Oceanus::STREAM, Oceanus::STREAM_Play,
                     { 0, 0, 0, 0, 42 });
  string playstatus = "";
  while (true) {
    auto response = radio.send_command(Oceanus::STREAM, Oceanus::STREAM_GetPlayStatus);
    auto payload = response->payload();
    string new_playstatus = "unknown";
    switch (payload[0]) {
    case 0: new_playstatus = "playing"; break;
    case 1: new_playstatus = "searching"; break;
    case 2: new_playstatus = "tuning"; break;
    case 3: new_playstatus = "stop"; break;
    }
    if (new_playstatus != playstatus) {
      cout << "Play Status: " << new_playstatus << endl;
      playstatus = new_playstatus;
    }
    if (payload[2] & 0x01) {
      cout << "STREAM_GetProgramName" << endl;
      auto response = radio.send_command(Oceanus::STREAM, Oceanus::STREAM_GetProgramName);
    }
    if (payload[2] & 0x02) {
      cout << "STREAM_GetProgramText" << endl;
      auto response = radio.send_command(Oceanus::STREAM, Oceanus::STREAM_GetProgramText);
    }
    if (payload[2] & 0x04) {
      cout << "STREAM_GetDLSCmd" << endl;
      auto response = radio.send_command(Oceanus::STREAM, Oceanus::STREAM_GetDLSCmd);
    }
    if (payload[2] & 0x08) {
      cout << "STREAM_GetStereo" << endl;
      auto response = radio.send_command(Oceanus::STREAM, Oceanus::STREAM_GetStereo);
    }
    if (payload[2] & 0x10) {
      cout << "STREAM_GetServiceName" << endl;
      auto response = radio.send_command(Oceanus::STREAM, Oceanus::STREAM_GetServiceName);
    }
    if (payload[2] & 0x20) {
      cout << "STREAM_GetSorter" << endl;
      auto response = radio.send_command(Oceanus::STREAM, Oceanus::STREAM_GetSorter);
    }
    if (payload[2] & 0x40) {
      cout << "STREAM_GetFrequency" << endl;
      auto response = radio.send_command(Oceanus::STREAM, Oceanus::STREAM_GetFrequency);
    }
    if (payload[2] & 0x80) {
      cout << "RTC_GetClock" << endl;
      auto response = radio.send_command(Oceanus::RTC, Oceanus::RTC_GetClock);
    }

    usleep(100000);
  }
}
