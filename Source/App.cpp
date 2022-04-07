#pragma once

#include "pch.hpp"

#include "Visual/DeviceResource.hpp"

using ::Microsoft::WRL::ComPtr;

class CApp : public CoreApp, public CRenderer
{

public:
  static int App(HINSTANCE hinst)
  {
    peekRun(Window::CCoreWindow<CApp>{hinst, {500, 500, 900, 900}});
    MessageBeep(5);
    return 0;
  };

  void OnCreate(_In_ const ::Window::CreationArgs &args) noexcept
  {

    SIZE RTSize{RECTWIDTH(args.m_Rect), RECTHEIGHT(args.m_Rect)};
    m_pDeviceResource = std::make_unique<CDeviceResource>(m_Handle, RTSize, CRenderer::m_pContext);
    H_FAIL(m_pDeviceResource->CreateDeviceResources(*this));
    SetViewPort(static_cast<float>(RTSize.cx), static_cast<float>(RTSize.cx));
    CRenderer::Set();
  };

  void OnPaint() noexcept
  {
    CRenderer::Draw();

    H_FAIL(m_pDeviceResource->GetSwapChain()->Present(1u, 0u));
  };

  void OnClose() noexcept { m_pDeviceResource.release(); };

private:
  void SetViewPort(float Width, float Height) const noexcept
  {
    D3D11_VIEWPORT ViewPortDesc{};
    ViewPortDesc.Width = Width;
    ViewPortDesc.Height = Height;
    ViewPortDesc.MinDepth = 0;
    ViewPortDesc.MaxDepth = 1;
    ViewPortDesc.TopLeftX = 0;
    ViewPortDesc.TopLeftY = 0;
    CRenderer::m_pContext->RSSetViewports(1, &ViewPortDesc);
  };
  std::unique_ptr<CDeviceResource> m_pDeviceResource{};

  template <class TCoreWindow>
  friend int __stdcall peekRun(const TCoreWindow &window)
  {
    ::MSG messages{};

    while (messages.message != WM_QUIT)
    {
      PeekMessageW(&messages, 0, 0, 0, PM_REMOVE);
      if (messages.hwnd != window.m_Handle)
        continue;

      TranslateMessage(&messages);
      DispatchMessageW(&messages);
      // window.OnPaint();
    };
    return 0;
  };
};

EXTERN_C IMAGE_DOS_HEADER __ImageBase;
int wWinMain(HINSTANCE hinst, HINSTANCE, LPWSTR, int) { return Invoke(&CApp::App, hinst); };
int main() { return Invoke(&CApp::App, (HINSTANCE)&__ImageBase); };