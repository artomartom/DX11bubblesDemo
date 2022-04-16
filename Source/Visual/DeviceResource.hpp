
#ifndef VISUAL_DEVICE_HPP
#define VISUAL_DEVICE_HPP

#include "Renderer.hpp"

#include "DebugInterface.hpp"

class DeviceResource : public DebugInterface
{

public:
  HRESULT TestDeviceSupport();
  DeviceResource(const HWND &hwnd, const SIZE &TargetSize, _Out_ ::Microsoft::WRL::ComPtr<ID3D11DeviceContext> &pContext, _Out_ HRESULT *hr);
  HRESULT CreateDeviceResources(_Out_ Renderer &Renderer);

  const ::Microsoft::WRL::ComPtr<IDXGISwapChain> &GetSwapChain() const { return m_pSwapChain; };

protected:
private:
  ::Microsoft::WRL::ComPtr<ID3D11Device> m_pDevice{};
  ::Microsoft::WRL::ComPtr<IDXGISwapChain> m_pSwapChain{};

  D3D_FEATURE_LEVEL m_thisFeatureLevel{};
};

#endif