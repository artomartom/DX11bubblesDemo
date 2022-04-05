
#ifndef VISUAL_DEVICE_HPP
#define VISUAL_DEVICE_HPP

#include "../pch.hpp"
#include "DebugInterface.hpp"

class CDeviceResource : public CDebugInterface
{

public:
  CDeviceResource(const HWND &hwnd, const SIZE &TargetSize);

  HRESULT CreateDeviceResources(::Microsoft::WRL::ComPtr<ID3D11VertexShader> &pVertexShader,
                                ::Microsoft::WRL::ComPtr<ID3D11PixelShader> &pPixelShader,
                                ::Microsoft::WRL::ComPtr<ID3D11InputLayout> &pInputLayout,
                                const D3D11_INPUT_ELEMENT_DESC *pInputElementDescs, UINT NumElements);

  const ::Microsoft::WRL::ComPtr<ID3D11Device> &GetDevice() const { return m_pDevice; };
  const ::Microsoft::WRL::ComPtr<ID3D11DeviceContext> &GetContext() const { return m_pContext; };
  const ::Microsoft::WRL::ComPtr<ID3D11RenderTargetView> &GetRenderTargetView() const { return m_pRenderTargetView; };
  const ::Microsoft::WRL::ComPtr<IDXGISwapChain> &GetSwapChain() const { return m_pSwapChain; };

protected:
private:
  ::Microsoft::WRL::ComPtr<ID3D11Device> m_pDevice{};
  ::Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_pContext{};
  ::Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_pRenderTargetView{};
  ::Microsoft::WRL::ComPtr<IDXGISwapChain> m_pSwapChain{};

  D3D_FEATURE_LEVEL m_featureLevel{};
};

#endif