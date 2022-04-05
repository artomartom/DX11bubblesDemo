#ifndef TIMER_INL
#define TIMER_INL

namespace Timer
{

  inline SYSTEMTIME GetLocalTime() noexcept
  {
    SYSTEMTIME tmp{};
    ::GetLocalTime(&tmp);
    return tmp;
  };
  inline long long QueryPerformanceFrequency() noexcept
  {
    LARGE_INTEGER tmp;
    ::QueryPerformanceFrequency(&tmp);
    return tmp.QuadPart;
  };

  class CTimer
  {
  public:
    explicit CTimer()
        : m_Frequency{Timer::QueryPerformanceFrequency()},
          m_GlblTime{},
          Intrnl_Time{}
    {
      Count();
      Intrnl_Time = {0, 0}; //  discard  time pass  from OS's start, so timer counts his life time
    };

    virtual ~CTimer()
    {
      Count();
    };

    void TimeStamp() const noexcept {};
    void Count() noexcept
    {
      long long ElapsedTicks{};
      ::QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER *>(&ElapsedTicks));
      ElapsedTicks = ElapsedTicks - m_GlblTime;
      Intrnl_Time.m_Delta += (ElapsedTicks / static_cast<double>(m_Frequency));
      Intrnl_Time.m_SinceStart += (ElapsedTicks) / m_Frequency;
      m_GlblTime += ElapsedTicks;
    };

    void Reset() noexcept
    {
      CTimer tmp{};
      swap(tmp, *this);
    };

    CTimer(const CTimer &other)
        : m_Frequency{other.m_Frequency},
          m_GlblTime{other.m_GlblTime},
          Intrnl_Time{other.Intrnl_Time} {};
    CTimer operator=(const CTimer &other) { return CTimer{other}; };
    bool operator>(const CTimer &other) { return Intrnl_Time.m_SinceStart > other.Intrnl_Time.m_SinceStart; };
    bool operator<(const CTimer &other) { return Intrnl_Time.m_SinceStart < other.Intrnl_Time.m_SinceStart; };
    bool operator==(const CTimer &other) { return Intrnl_Time.m_SinceStart == other.Intrnl_Time.m_SinceStart; };
    bool operator!=(const CTimer &other) { return Intrnl_Time.m_SinceStart != other.Intrnl_Time.m_SinceStart; };

    CTimer(CTimer &&) = default;
    CTimer &operator=(CTimer &&) = default;

    // get time  since last call to either
    template <typename T>
    T CountDelta() noexcept
    {
      Count();
      return static_cast<T>(Intrnl_Time.m_Delta);
    };

    // get time since object creation
    template <typename T>
    T Count() noexcept
    {
      Count();
      return static_cast<T>(Intrnl_Time.m_SinceStart);
    };

  protected:
  private:
    const long long m_Frequency{};

    long long m_GlblTime{}; // since OS's start + since object creation, aka what  ::QueryPerformanceFrequency returned in last Count()

    struct
    {
      long long m_SinceStart{}; // since object creation(1 =1 milisecond)
      double m_Delta{};         // Delta(1.000000=1 second)
    } Intrnl_Time;

    friend void swap(CTimer &Left, CTimer &Right) noexcept
    {
      CTimer Tmp{std::move(Left)};
      Left = std::move(Right);
      Right = std::move(Tmp);
    };
  };

}
#endif