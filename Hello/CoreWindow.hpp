
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

  struct AppEventArgs
  {
    AppEventArgs(::UINT message, ::WPARAM wParam, ::LPARAM lParam)
        : message{message}, wParam{wParam}, lParam{lParam} {};
    ::UINT message{};
    ::WPARAM wParam{};
    ::LPARAM lParam{};
  };
  struct CommandArgs
  {
    CommandArgs(::WPARAM wParam, ::LPARAM lParam)
        : wParam{wParam}, lParam{lParam} {};
    ::WPARAM wParam{};
    ::LPARAM lParam{};
  };

  struct KeyEventArgs
  {
    KeyEventArgs(::WPARAM wParam)
        : m_VirtualKey{wParam} {};
    ::WPARAM m_VirtualKey{}; // Key Code   ...
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
  class CCoreWindow : public TImpl
  {

  public:
    CCoreWindow() = delete;
    CCoreWindow(const CCoreWindow &) = delete;
    CCoreWindow(CCoreWindow &&) = delete;
    CCoreWindow &operator=(const CCoreWindow &) = delete;
    CCoreWindow &operator=(CCoreWindow &&) = delete;
    ~CCoreWindow()
    {
      TImpl::OnClose();
      ::DestroyWindow(TImpl::m_Handle);
    };

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
      ::CreateWindowExW(
          hwin_style_ex,
          wincl.lpszClassName, 0,
          hwin_style,
          MainRect.left, MainRect.top,
          RECTWIDTH(MainRect),
          RECTHEIGHT(MainRect),
          HWND_DESKTOP, nullptr, nullptr, &CreateArgs);
    };

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
                   s_pthis->m_Handle = hWnd;
                   if (H_FAIL(s_pthis->TImpl::OnCreate(*args)))
                   {
                     return -1;
                   };
                   ::ShowWindow(hWnd, SW_NORMAL);
                 });
        PROCCASE(WM_ACTIVATE, false, { s_pthis->TImpl::OnWindowActivate({wParam}); });
        PROCCASE(WM_CLOSE, false, { ::PostQuitMessage(0); });
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
        PROCCASE(WM_ENTERSIZEMOVE, false, { ::Sleep(30); });
        PROCCASE(WM_EXITSIZEMOVE, false, { ::Sleep(30); });

        /*user input*/
        PROCCASE(WM_COMMAND, false, { s_pthis->TImpl::OnCommand(CommandArgs(wParam, lParam)); });
        PROCCASE(WM_MOUSEMOVE, false, s_pthis->OnCursorMove());
        PROCCASE(WM_KEYDOWN, false, { (!(lParam & (1 << 31))) ? s_pthis->OnKeyStroke({wParam}) : s_pthis->OnKeyHold({wParam}); });
      };

      if (message > WM_APP)
      {
        s_pthis->TImpl::OnAppEvent(AppEventArgs(message, wParam, lParam));
        return 0;
      }
      else
      {
        return ::DefWindowProcW(hWnd, message, wParam, lParam);
      };

#undef PROCCASE
    };
  };

  template <class TCoreWindow>
  int __stdcall Run(const TCoreWindow &window)
  {
    ::MSG m_messages{};

    while (::GetMessageW(&m_messages, window.m_Handle, 0, 0))
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
  /**
   * Event proc message handlers
   *
   */
  ::HRESULT OnCreate(_In_ const ::Window::CreationArgs &args) noexcept { return S_OK; };
  ::HRESULT OnPaint() noexcept { return S_OK; };
  ::HRESULT OnClose() noexcept { return S_OK; };
  ::HRESULT OnKeyHold(_In_ const ::Window::KeyEventArgs &args) noexcept { return S_OK; };
  ::HRESULT OnKeyStroke(_In_ const ::Window::KeyEventArgs &args) noexcept { return S_OK; };
  ::HRESULT OnCursorMove() noexcept { return S_OK; };
  ::HRESULT OnSizing(_Out_ RECT *pRect) noexcept { return S_OK; };
  ::HRESULT OnCommand(_In_ const ::Window::CommandArgs &args) noexcept { return S_OK; };
  ::HRESULT OnSizeChanged(_In_ const ::Window::SizeChangedArgs &args) noexcept { return S_OK; };
  ::HRESULT OnWindowActivate(_In_ const ::Window::ActivateArgs &args) noexcept { return S_OK; };
  ::HRESULT OnAppEvent(_In_ const ::Window::AppEventArgs &args) noexcept { return S_OK; };

  void Close() noexcept { ::SendMessageW(m_Handle, WM_CLOSE, 0, 0); };

  bool IsValid()noexcept{ return m_Handle !=0; };
  void SetHeader(const char *text)const noexcept { ::SetWindowTextA(m_Handle, text); };

protected:
  HWND m_Handle{nullptr};
  bool m_IsVisible{};
};

#endif
 