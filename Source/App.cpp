#pragma once

#include "pch.hpp"

#include "Visual/DeviceResource.hpp"

#define CASE(message, action) \
  case message:               \
    action;                   \
    break

using ::Microsoft::WRL::ComPtr;

class App : public CoreApp, public Renderer
{

public:
  static int AppEntry(HINSTANCE hinst)
  {
    peekRun(Window::CCoreWindow<App>{hinst, {10, 10, 1700, 1000}});
    MessageBeep(5);
    return 0;
  };
  void OnWindowActivate(_In_ const ::Window::ActivateArgs &args) noexcept
  {

    if (CoreApp::m_IsVisible != args.m_IsMinimized)
    {
      CoreApp::m_IsVisible = args.m_IsMinimized;
      m_ShouldDraw = !CoreApp::m_IsVisible;
    };
  };

  void OnKeyStroke(_In_ const ::Window::KeyEventArgs &args) noexcept
  {
    switch (args.m_VirtualKey)
    {
      CASE(VK_ESCAPE, { CoreApp::Close(); });
      CASE(VK_SPACE, { Renderer::Timer.Switch(); });
    }
  };

  void OnCreate(_In_ const ::Window::CreationArgs &args) noexcept
  {
    HRESULT hr{};

    if (H_OK(hr = DeviceResource::TestDeviceSupport()))
    {

      SIZE RTSize{RECTWIDTH(args.m_Rect), RECTHEIGHT(args.m_Rect)};
      m_pDeviceResource = std::make_unique<DeviceResource>(m_Handle, Renderer::m_pContext, &hr);
      if (H_OK(hr))
      {
        if (H_OK(hr = m_pDeviceResource->CreateDeviceResources(*this)))
        {
          // window's procedure sends OnSizeChanged event  on creation, so what we need to do here is only set new viewport before it happens
          Renderer::SetViewPort(static_cast<float>(RTSize.cx), static_cast<float>(RTSize.cy));
          Renderer::SetPipeLine();
          return;
        };
      };
    };
    DBG_ONLY(
        {
          m_pDeviceResource->PullDebugMessage();
        });
    m_ShouldClose = true;
  };

  void OnSizeChanged(_In_ const ::Window::SizeChangedArgs &args) noexcept
  {
    HRESULT hr{};

    if (m_pDeviceResource->GetSwapChain())
    {
      // H_FAIL(hr = m_pDeviceResource->GetSwapChain()->ResizeBuffers(
      //            m_pDeviceResource->GetNumBackBuffer(),
      //            args.m_New.cx, args.m_New.cy,
      //            DXGI_FORMAT_UNKNOWN, 0u));

      if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
      {
        H_FAIL(m_pDeviceResource->HandleDeviceRemoved());
      };
    }
    else
    {
      m_pDeviceResource->CreateSizeDependentDeviceResources(*this, m_Handle);
      Renderer::UpdateViewPortSizeBuffer(static_cast<float>(args.m_New.cx), static_cast<float>(args.m_New.cy));
    };
  };

  void Draw() noexcept
  {

    if (m_ShouldDraw && !m_ShouldClose)
    {
      UpdateFrameBuffer();
      Renderer::Draw();
      /**
       *   The first argument instructs DXGI to block until VSync, putting the application
       *  to sleep until the next VSync. This ensures we don't waste any cycles rendering
       *  frames that will never be displayed to the screen.
       */
      H_FAIL(m_pDeviceResource->GetSwapChain()->Present(1u, 0u));
    };
    DBG_ONLY(m_pDeviceResource->DebugInterface::Report());
  };

  void OnClose() noexcept
  {
    m_pDeviceResource.release();
  };

private:
  std::unique_ptr<DeviceResource> m_pDeviceResource{};
  bool m_ShouldDraw{true};
  bool m_ShouldClose{false};

  template <class TCoreWindow>
  friend int __stdcall peekRun(TCoreWindow &&window)
  {
    if (window.m_ShouldClose)
    {
      Log<File>::Write(L"App Creation canceled");
      return 1;
    };
    ::MSG messages{};
    //   char updateCount[20]{ };
    while (messages.message != WM_QUIT)
    {
      ::PeekMessageW(&messages, 0, 0, 0, PM_REMOVE);
      ::TranslateMessage(&messages);
      ::DispatchMessageW(&messages);

      window.App::Draw();
      //::snprintf(updateCount, _countof(updateCount), "Updates/sec %1.0f", 1.f/Renderer::Timer.GetDelta<float>());
      // window.SetHeader(updateCount);
    };
    return 0;
  };
};

EXTERN_C IMAGE_DOS_HEADER __ImageBase;
int wWinMain(HINSTANCE hinst, HINSTANCE, LPWSTR, int) { return Invoke(&App::AppEntry, hinst); };
int main() { return Invoke(&App::AppEntry, (HINSTANCE)&__ImageBase); };