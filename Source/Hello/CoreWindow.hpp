
#ifndef WINDOW_HPP
#define WINDOW_HPP

#define RECTWIDTH(x) (x.right - x.left)
#define RECTHEIGHT(x) (x.bottom - x.top)

namespace Window
{

#pragma region EventArgs
  struct CreationArgs
  {

    /*args*/
    void *pCoreWindow{};
    ::HINSTANCE m_hInst{};
    ::RECT m_Rect{};
  };

  struct SizeChangedArgs
  {

    SizeChangedArgs(::WPARAM wParam, ::LPARAM lParam)
        : m_Type{static_cast<e_Type>(wParam)}, m_New{LOWORD(lParam), HIWORD(lParam)} {};
    /*args*/
    enum e_Type : ::WPARAM
    {
      MaxHide = SIZE_MAXHIDE,     // Message is sent to all pop-up windows when some other window is maximized.
      Maxinized = SIZE_MAXIMIZED, // The window has been maximized.
      MaxShow = SIZE_MAXSHOW,     // Message is sent to all pop-up windows when some other window has been restored to its former size.
      Minimized = SIZE_MINIMIZED, // The window has been minimized.
      Restored = SIZE_RESTORED,   // The window has been resized, but neither the SIZE_MINIMIZED nor SIZE_MAXIMIZED value applies.

    } m_Type{};
    ::SIZE m_New{};
  };

  struct ActivateArgs
  {
    ActivateArgs(WPARAM wParam)
        : m_Activate{LOWORD(wParam)},
          m_IsMinimized{static_cast<bool>(HIWORD(wParam))} {};
    /*args*/
    enum e_Activate : ::WPARAM
    {
      Activated = WA_ACTIVE,           // Activated by some method other than a mouse click (  by a call to the SetActiveWindow function or by use of the keyboard interface to select the window).
      ClickActivated = WA_CLICKACTIVE, // Activated by a mouse click.
      Deactivated = WA_INACTIVE,       // Deactivated.
    } m_Activate{};
    bool m_IsMinimized{};
  };
#pragma endregion // EventArgs

  template <class TImpl>
  class CCoreWindow : private TImpl
  {

  public:
    CCoreWindow() = delete;
    CCoreWindow(const CCoreWindow &) = delete;
    CCoreWindow(CCoreWindow &&) = delete;
    CCoreWindow &operator=(const CCoreWindow &) = delete;
    CCoreWindow &operator=(CCoreWindow &&) = delete;
    ~CCoreWindow() { ::DestroyWindow(TImpl::m_hWnd); };

    CCoreWindow(HINSTANCE hinst, const RECT rect)
    {

      WNDCLASSEXW wincl{
          sizeof(WNDCLASSEXW),
          CS_GLOBALCLASS | CS_PARENTDC,
          CoreProcedure,
          0, 0, 0, 0, ::LoadCursorA(0, IDC_ARROW), 0, 0,
          L"TheWindowName", 0};
      ::RegisterClassExW(&wincl);

      constexpr DWORD hwin_style = WS_OVERLAPPEDWINDOW | WS_CAPTION | WS_MINIMIZE;
      constexpr DWORD hwin_style_ex = WS_EX_OVERLAPPEDWINDOW;

      RECT MainRect{rect};
      ::AdjustWindowRectEx(&MainRect, hwin_style, 0, hwin_style_ex);

      CreationArgs CreateArgs{this, hinst, rect};
      TImpl::m_hWnd = ::CreateWindowExW(
          hwin_style_ex,
          wincl.lpszClassName, 0,
          hwin_style,
          MainRect.left, MainRect.top,
          RECTWIDTH(MainRect),
          RECTHEIGHT(MainRect),
          HWND_DESKTOP, nullptr, nullptr, &CreateArgs);
    };

  private: // member (restrict  to only HWND)
  private: // non-member functions
    template <class TCoreWindow>
    friend int __stdcall Run(const TCoreWindow &window);

    inline static ::LRESULT __stdcall CoreProcedure(::HWND hWnd, ::UINT message, ::WPARAM wParam, ::LPARAM lParam)
    {
#define PROCCASE(message, boolreturn, action) \
  case message:                               \
    action;                                   \
    return boolreturn

      static CCoreWindow *s_pthis{};

      switch (message)
      {
        /*window status */
        PROCCASE(WM_CREATE, false,
                 {
                   CreationArgs *args = reinterpret_cast<CreationArgs *>((reinterpret_cast<::CREATESTRUCT *>(lParam))->lpCreateParams);
                   s_pthis = reinterpret_cast<CCoreWindow *>(args->pCoreWindow);
                   s_pthis->TImpl::OnCreate(*args);
                   ::ShowWindow(hWnd, SW_NORMAL);
                 });
        PROCCASE(WM_ACTIVATE, false, { s_pthis->TImpl::OnWindowActivate({wParam}); });
        PROCCASE(WM_CLOSE, false, {s_pthis->TImpl::OnClose(); /*::DestroyWindow(hWnd);*/::PostQuitMessage(0);  s_pthis =nullptr; });
        /*visual*/
        PROCCASE(WM_ERASEBKGND, true, {});
        PROCCASE(WM_PAINT, false,
                 {
                   ::BeginPaint(hWnd, nullptr);
                   s_pthis->TImpl::OnPaint();
                   ::EndPaint(hWnd, nullptr);
                 };);
        /*window size*/
        PROCCASE(WM_SIZE, false, s_pthis->TImpl::OnSizeChanged({wParam, lParam}));
        PROCCASE(WM_SIZING, true, s_pthis->TImpl::OnSizing(reinterpret_cast<RECT *>(lParam)));

        /*Devices::Input*/
        PROCCASE(WM_MOUSEMOVE, false, s_pthis->OnCursorMove());
        PROCCASE(WM_KEYDOWN, false, { (!(lParam & (1 << 31))) ? s_pthis->OnKeyStroke() : s_pthis->OnKeyHold(); });

      default:
        return ::DefWindowProcW(hWnd, message, wParam, lParam);
      }
#undef PROCCASE
    };
  };

  template <class TCoreWindow>
  int __stdcall Run(const TCoreWindow &window)
  {
    ::MSG m_messages{};

    while (::GetMessageW(&m_messages, 0, 0, 0))
    {

      ::TranslateMessage(&m_messages);
      ::DispatchMessageW(&m_messages);
    };

    return 0;
  };
};

class CoreApp
{
public:
  void OnCreate(const ::Window::CreationArgs &args) noexcept {};
  void OnPaint() noexcept {};
  void OnClose() noexcept {};
  void OnKeyHold() noexcept {};
  void OnKeyStroke() noexcept {};
  void OnCursorMove() noexcept {};
  void OnSizeChanged(_In_ const ::Window::SizeChangedArgs &args) noexcept {};
  void OnSizing(_Out_ RECT *pRect) noexcept {};
  void OnWindowActivate(const ::Window::ActivateArgs &args) noexcept {};

protected:
  HWND m_hWnd{nullptr};
};

#endif