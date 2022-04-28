
#ifndef VISUAL_DEVICE_HPP
#define VISUAL_DEVICE_HPP

#include "Renderer.hpp"

#include "DebugInterface.hpp"

class DeviceResource : public DebugInterface
{

public:
  static HRESULT TestDeviceSupport();

  DeviceResource(_Out_ Renderer &Renderer, _Out_ HRESULT *hr);
  HRESULT CreateDeviceResources(_Out_ Renderer &Renderer);
  HRESULT CreateSizeDependentDeviceResources(const HWND &windowHandle, Renderer &Renderer);
  HRESULT HandleDeviceRemoved() { return ERROR_CALL_NOT_IMPLEMENTED; };
  HRESULT Present() const noexcept { return m_pSwapChain->Present(1u, 0u); };

  HRESULT ComputeCircleTexture(
      _In_ const ::Microsoft::WRL::ComPtr<ID3D11DeviceContext> &pContext,
      _Out_ ID3D11Resource **ppTexture,
      _Out_ ID3D11ShaderResourceView **ppTextureView,
      const DirectX::XMUINT3 &&threadGroupCount,
      const DirectX::XMUINT2 &&imageSize);

  UINT GetNumBackBuffer() const noexcept
  {
    return m_numBackBuffers;
  };
  const ::Microsoft::WRL::ComPtr<IDXGISwapChain1> &GetSwapChain() const
  {
    return m_pSwapChain;
  };

protected:
private:
  ::Microsoft::WRL::ComPtr<ID3D11Device> m_pDevice{};
  ::Microsoft::WRL::ComPtr<IDXGISwapChain1> m_pSwapChain{};
  ::Microsoft::WRL::ComPtr<IDXGIFactory4> m_pDXGIFactory{};

  D3D_FEATURE_LEVEL m_thisFeatureLevel{};

  UINT m_numBackBuffers{};
};

#endif