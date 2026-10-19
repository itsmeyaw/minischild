#pragma once
#include <ctime>
class Clock {
 public:
  virtual ~Clock() = default;
  virtual time_t now() = 0;
};
