
#ifndef VISUAL_DEVICE_HPP
#define VISUAL_DEVICE_HPP

#include "Renderer.hpp"

#include "DebugInterface.hpp"

class DeviceResource : public DebugInterface
{

public:
  static HRESULT TestDeviceSupport();

  HRESULT GenerateCircleTexture(
      _In_ const ::Microsoft::WRL::ComPtr<ID3D11DeviceContext> &Context,
      _Out_ ID3D11Resource **ppTexture,
      _Out_ ID3D11ShaderResourceView **ppTextureView);

  DeviceResource( _Out_ Renderer &Renderer, _Out_ HRESULT *hr);
  HRESULT CreateDeviceResources(_Out_ Renderer &Renderer);
  HRESULT CreateSizeDependentDeviceResources(const HWND &windowHandle,Renderer &Renderer);
  HRESULT HandleDeviceRemoved() { return ERROR_CALL_NOT_IMPLEMENTED; };

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

  inline static UINT s_SampleCount;
  inline static UINT s_QualityLevel;

  D3D_FEATURE_LEVEL m_thisFeatureLevel{};

  UINT m_numBackBuffers{};
};

#endif