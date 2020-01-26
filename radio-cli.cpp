
#include <oceanus.h>
#include <iostream>
#include <iomanip>
#include <regex>
#include <map>

#include <unistd.h>

using namespace std;

vector<string>
split(const string& s, string re_str = "\\s+") {
  vector<string> elems;

  regex re(re_str);

  sregex_token_iterator iter(s.begin(), s.end(), re, -1);
  sregex_token_iterator end;

  while (iter != end) {
    if (iter->length()) {
      elems.push_back(*iter);
    }
    ++iter;
  }

  return elems;
}

class RadioCLI {
public:
  RadioCLI(const char* device_name);

  void run();

private:
  Oceanus::Radio _radio;

  bool input_available();

  void handle_command(string command);

  using command_handler = void (RadioCLI::*)(vector<string> arguments);

  map<string, command_handler> _command_handlers;

  // Commands
  void dab(vector<string>);
  void fm(vector<string>);
  void volume(vector<string>);
  void scan(vector<string>);
};

RadioCLI::RadioCLI(const char* device_name)
  : _radio(device_name)
{
  _command_handlers["dab"] = &RadioCLI::dab;
  _command_handlers["fm"] = &RadioCLI::fm;
  _command_handlers["volume"] = &RadioCLI::volume;
  _command_handlers["scan"] = &RadioCLI::scan;

  _radio.set_volume(10);
  _radio.set_stereo_mode(Oceanus::Radio::AUTO_DETECT_STEREO);

  _radio.play_dab(42);
}

bool
RadioCLI::input_available()
{
  struct timeval tv;
  fd_set fds;

  tv.tv_sec = tv.tv_usec = 0;

  FD_ZERO(&fds);
  FD_SET(0, &fds);
  return ::select(1, &fds, 0, 0, &tv) == 1;
}

void
RadioCLI::handle_command(string command)
{
  cout << "Command: " << command << endl;
  auto args = split(command);
  if (args.size()) {
    auto command = args[0];
    args.erase(args.begin());
    if (_command_handlers.count(command)) {
      auto handler = _command_handlers[command];
      (this->*handler)(args);
    } else {
      cout << "Unknown command: " << command << endl;
    }
  }
}

void
RadioCLI::dab(vector<string> args)
{
  unsigned channel = stoul(args.at(0));

  _radio.play_dab(channel);
}

void
RadioCLI::fm(vector<string> args)
{
  unsigned frequency = stof(args.at(0));

  _radio.play_fm(frequency);
}


void
RadioCLI::volume(vector<string> args)
{
  unsigned volume = stoul(args.at(0));

  _radio.set_volume(volume);
}

void
RadioCLI::scan(vector<string>)
{
  _radio.reset(Oceanus::Radio::CLEAR_DATABASE);
  _radio.auto_search(0, 80);
}

void
RadioCLI::run()
{
  while (true) {
    _radio.handle_status();
    _radio.handle_mot();

    usleep(100000);
    if (input_available()) {
      string command;
      getline(cin, command);
      if (cin.eof() || (command == "quit")) {
        break;
      }
      handle_command(command);
    }
  }
}

int
main(int argc, char* argv[])
{
  if (argc != 2) {
    throw invalid_argument("Missing command line argument, expecting serial device name");
  }

  RadioCLI cli(argv[1]);

  cli.run();
}
