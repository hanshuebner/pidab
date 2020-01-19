
#include <oceanus.h>

#include <iostream>

#include <unistd.h>

using namespace std;

int
main(int argc, char* argv[])
{
  Oceanus::Radio radio("/dev/tty.usbmodem141101");

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
  while (true) {
    radio.send_command(Oceanus::STREAM, Oceanus::STREAM_GetPlayStatus);
    usleep(100000);
  }
}
