#ifndef TIMER_INL
#define TIMER_INL

namespace Timer
{

  using immediate = std::integral_constant<bool, true>;
  using deferred = std::integral_constant<bool, false>;

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
    explicit CTimer(immediate = immediate{})
        : m_Frequency{Timer::QueryPerformanceFrequency()},
          m_GlblTime{},
          Intrnl_Time{}
    {
      Count();
      Intrnl_Time = {0, 0}; //  discard  time pass  from OS's start, so timer counts his life time
    };

    explicit CTimer(deferred)
        : CTimer()
    {
      Intrnl_Time.m_Delta = 0;
      Intrnl_Time.m_SinceStart = 0;
      m_GlblTime = 0;
    };

    virtual ~CTimer() = default;

    void TimeStamp() const noexcept {};

    void Count() noexcept
    {
      long long ElapsedTicks{};
      ::QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER *>(&ElapsedTicks));
      ElapsedTicks = ElapsedTicks - m_GlblTime;
      Intrnl_Time.m_Delta = (ElapsedTicks / static_cast<double>(m_Frequency));
      Intrnl_Time.m_SinceStart += (ElapsedTicks * 1000) / m_Frequency;
      m_GlblTime += ElapsedTicks;
    };

    void Pause() noexcept
    {
      m_PausedAt = Intrnl_Time.m_SinceStart;
    };
    void Unpause() noexcept
    {
      Intrnl_Time.m_SinceStart -= Intrnl_Time.m_SinceStart - m_PausedAt;
      m_PausedAt = 0;
    };

    /**
     * Unpause if pause, and vice-versa
     */
    void Switch() noexcept
    {
      if (m_PausedAt)
        Unpause();
      else
        Pause();
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

    template <typename T>
    T GetDelta() noexcept { return static_cast<T>(Intrnl_Time.m_Delta); };

    // get time  since last call to either
    template <typename T>
    T CountDelta() noexcept
    {
      Count();
      return static_cast<T>(Intrnl_Time.m_Delta);
    };

    // gets time since object creation,  if  paused gets time when was pause
    template <typename T>
    T Count() noexcept
    {
      Count();
      return m_PausedAt ? static_cast<T>(m_PausedAt) : Intrnl_Time.m_SinceStart;
    };

  protected:
  private:
    const long long m_Frequency{};

    long long m_GlblTime{}; // since OS's start + since object creation, aka what  ::QueryPerformanceFrequency returned in last Count()
    long long m_PausedAt{};

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