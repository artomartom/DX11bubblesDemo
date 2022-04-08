#pragma once

#include "pch.hpp"

#include "Visual/DeviceResource.hpp"

using ::Microsoft::WRL::ComPtr;

class CApp : public CoreApp, public CRenderer
{

public:
  static int App(HINSTANCE hinst)
  {
    peekRun(Window::CCoreWindow<CApp>{hinst, {500, 500, 1100, 900}});
    MessageBeep(5);
    return 0;
  };

  void OnCreate(_In_ const ::Window::CreationArgs &args) noexcept
  {

    SIZE RTSize{RECTWIDTH(args.m_Rect), RECTHEIGHT(args.m_Rect)};
    m_pDeviceResource = std::make_unique<CDeviceResource>(m_Handle, RTSize, CRenderer::m_pContext);
    H_FAIL(m_pDeviceResource->CreateDeviceResources(*this));
    CRenderer::SetViewPort(static_cast<float>(RTSize.cx), static_cast<float>(RTSize.cy));
    CRenderer::UpdateViewPortSizeBuffer(static_cast<float>(RTSize.cx), static_cast<float>(RTSize.cy));
    CRenderer::SetPipeLine();
  };

  void OnSizeChanged(_In_ const ::Window::SizeChangedArgs &args) noexcept
  {
         CRenderer::UpdateViewPortSizeBuffer(static_cast<float>(args.m_New.cx), static_cast<float>(args.m_New.cy));
  };

   

  void Draw() const noexcept
  {
    CRenderer::Draw();

    H_FAIL(m_pDeviceResource->GetSwapChain()->Present(1u, 0u));
  };

  void OnClose() noexcept { m_pDeviceResource.release(); };

private:
  std::unique_ptr<CDeviceResource> m_pDeviceResource{};

  template <class TCoreWindow>
  friend int __stdcall peekRun(const TCoreWindow &window)
  {
    ::MSG messages{};

    while (messages.message != WM_QUIT)
    {
      ::PeekMessageW(&messages, 0, 0, 0, PM_REMOVE);
      ::TranslateMessage(&messages);
      ::DispatchMessageW(&messages);
      window.UpdateFrameBuffer();
      window.CApp::Draw();
    };
    return 0;
  };
};

EXTERN_C IMAGE_DOS_HEADER __ImageBase;
int wWinMain(HINSTANCE hinst, HINSTANCE, LPWSTR, int) { return Invoke(&CApp::App, hinst); };
int main() { return Invoke(&CApp::App, (HINSTANCE)&__ImageBase); };