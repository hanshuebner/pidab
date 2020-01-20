
#pragma once

#include <iostream>

class nullstream : public std::ostream {
public:
  nullstream() : std::ostream(nullptr) {}
  nullstream(const nullstream &) : std::ostream(nullptr) {}
};

template <class T>
const nullstream &operator<<(nullstream &&os, const T &value) { 
  return os;
}
