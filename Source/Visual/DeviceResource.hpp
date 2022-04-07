
#ifndef VISUAL_DEVICE_HPP
#define VISUAL_DEVICE_HPP

#include "Renderer.hpp"

#include "DebugInterface.hpp"

class CDeviceResource : public CDebugInterface
{

public:
  CDeviceResource(const HWND &hwnd, const SIZE &TargetSize, _Out_ ::Microsoft::WRL::ComPtr<ID3D11DeviceContext> &pContext);
  void  DrawTestTriangle(CRenderer &Renderer);
  HRESULT CreateDeviceResources(_Out_ CRenderer &Renderer);

  const ::Microsoft::WRL::ComPtr<IDXGISwapChain> &GetSwapChain() const { return m_pSwapChain; };

protected:
private:
  ::Microsoft::WRL::ComPtr<ID3D11Device> m_pDevice{};
  ::Microsoft::WRL::ComPtr<IDXGISwapChain> m_pSwapChain{};

  D3D_FEATURE_LEVEL m_featureLevel{};
};

#endif