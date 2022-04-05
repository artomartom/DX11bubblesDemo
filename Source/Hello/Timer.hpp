#ifndef TIMER_HPP
#define TIMER_HPP

namespace Timer
{

  SYSTEMTIME GetLocalTime() noexcept;
  long long QueryPerformanceFrequency() noexcept;

  class CTimer;

}
#endif