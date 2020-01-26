
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

  radio.set_volume(16);
  radio.set_stereo_mode(Oceanus::Radio::AUTO_DETECT_STEREO);

  radio.play_dab(42);

  while (true) {
    radio.handle_status();
    radio.handle_mot();

    usleep(100000);
  }
}
