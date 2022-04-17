#pragma once

#include "pch.hpp"

#include "Visual/DeviceResource.hpp"

#define CASE(message, action) \
  case message:               \
    action;                   \
    return S_OK

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
  ::HRESULT OnWindowActivate(_In_ const ::Window::ActivateArgs &args) noexcept
  {

    if (CoreApp::m_IsVisible != args.m_IsMinimized)
    {
      CoreApp::m_IsVisible = args.m_IsMinimized;
      m_ShouldDraw = !CoreApp::m_IsVisible;
    };
    return S_OK;
  };

  ::HRESULT OnKeyStroke(_In_ const ::Window::KeyEventArgs &args) noexcept
  {
    switch (args.m_VirtualKey)
    {
      CASE(VK_ESCAPE, { CoreApp::Close(); });
      CASE(VK_SPACE, { Renderer::Timer.Switch(); });
    }
    return S_OK;
  };

  ::HRESULT OnCreate(_In_ const ::Window::CreationArgs &args) noexcept
  {
    HRESULT hr{};

    if (H_FAIL(hr = DeviceResource::TestDeviceSupport()))
      return hr;

    SIZE RTSize{RECTWIDTH(args.m_Rect), RECTHEIGHT(args.m_Rect)};
    m_pDeviceResource = std::make_unique<DeviceResource>(m_Handle, RTSize, Renderer::m_pContext, &hr);
    if (H_FAIL(hr))
      return hr;
    if (H_FAIL(hr = m_pDeviceResource->CreateDeviceResources(*this)))
      return hr;
    Renderer::SetViewPort(static_cast<float>(RTSize.cx), static_cast<float>(RTSize.cy));
    Renderer::UpdateViewPortSizeBuffer(static_cast<float>(RTSize.cx), static_cast<float>(RTSize.cy));
    Renderer::SetPipeLine();
    return S_OK;
  };

  ::HRESULT OnSizeChanged(_In_ const ::Window::SizeChangedArgs &args) noexcept
  {
    Renderer::UpdateViewPortSizeBuffer(static_cast<float>(args.m_New.cx), static_cast<float>(args.m_New.cy));
    return S_OK;
  };

  void Draw() const noexcept
  {
    Renderer::Draw();
    /**
     *   The first argument instructs DXGI to block until VSync, putting the application
     *  to sleep until the next VSync. This ensures we don't waste any cycles rendering
     *  frames that will never be displayed to the screen.
     */
    H_FAIL(m_pDeviceResource->GetSwapChain()->Present(1u, 0u));

    DBG_ONLY(m_pDeviceResource->DebugInterface::Report());
  };

  ::HRESULT OnClose() noexcept
  {
    m_pDeviceResource.release();
    return S_OK;
  };

private:
  std::unique_ptr<DeviceResource> m_pDeviceResource{};
  bool m_ShouldDraw{true};

  template <class TCoreWindow>
  friend int __stdcall peekRun(TCoreWindow &&window)
  {

    ::MSG messages{};
    //   char updateCount[20]{ };
    while (messages.message != WM_QUIT)
    {
      ::PeekMessageW(&messages, 0, 0, 0, PM_REMOVE);
      ::TranslateMessage(&messages);
      ::DispatchMessageW(&messages);
      if (window.m_ShouldDraw)
      {
        window.UpdateFrameBuffer();
        window.App::Draw();
        //::snprintf(updateCount, _countof(updateCount), "Updates/sec %1.0f", 1.f/Renderer::Timer.GetDelta<float>());
        // window.SetHeader(updateCount);
      };
    };
    return 0;
  };
};

EXTERN_C IMAGE_DOS_HEADER __ImageBase;
int wWinMain(HINSTANCE hinst, HINSTANCE, LPWSTR, int) { return Invoke(&App::AppEntry, hinst); };
int main() { return Invoke(&App::AppEntry, (HINSTANCE)&__ImageBase); };