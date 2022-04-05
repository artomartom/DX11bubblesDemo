

#pragma once
#include "DeviceResource.hpp"

using ::Microsoft::WRL::ComPtr;

HRESULT CreateWICTextureFromFile(
    const wchar_t *filename,
    ID3D11Resource **ppTexture,
    ID3D11ShaderResourceView **ppTextureView,
    const ComPtr<ID3D11Device> &d3dDevice,
    const ComPtr<ID3D11DeviceContext> &Context);

CDeviceResource::CDeviceResource(const HWND &hwnd, const SIZE &TargetSize)
{

   unsigned int flags{};

   DBG_ONLY(flags |= D3D11_CREATE_DEVICE_DEBUG);

   DXGI_SWAP_CHAIN_DESC d_SwapChain{
       static_cast<UINT>(TargetSize.cx), static_cast<UINT>(TargetSize.cy),
       60, 1, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED,
       DXGI_MODE_SCALING_UNSPECIFIED, 1, 0, DXGI_USAGE_RENDER_TARGET_OUTPUT,
       1, hwnd, true, DXGI_SWAP_EFFECT_DISCARD, 0};

   H_CHECK(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, flags, 0, 0,
                                         D3D11_SDK_VERSION, &d_SwapChain, &m_pSwapChain, &m_pDevice, &m_featureLevel, &m_pContext),
           L"D3D11CreateDeviceAndSwapChain failed");
};

HRESULT CDeviceResource::CreateDeviceResources(ComPtr<ID3D11VertexShader> &pVertexShader, ComPtr<ID3D11PixelShader> &pPixelShader,
                                               ComPtr<ID3D11InputLayout> &pInputLayout, const D3D11_INPUT_ELEMENT_DESC *pInputElementDescs, UINT NumElements)
{

   HRESULT hr{};
   ComPtr<ID3D11Texture2D> backBuffer{};

   if (H_FAIL(hr = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &backBuffer)))
      return hr;

   if (H_FAIL(hr = m_pDevice->CreateRenderTargetView(backBuffer.Get(), 0, &m_pRenderTargetView)))
      return hr;
   DBG_ONLY({
      if (H_FAIL(hr = CDebugInterface::Init(m_pDevice)))
      return hr; });

   ComPtr<ID3D11Resource> pTexture{};
   ComPtr<ID3D11ShaderResourceView> pTextureView{};

   if (H_FAIL(hr = CreateWICTextureFromFile(L"circle.png", &pTexture, &pTextureView, m_pDevice.Get(), m_pContext.Get())))
      return hr;

#include "../Shader/Release/Vertex.hpp"
#include "../Shader/Release/Pixel.hpp"

   if (H_FAIL(hr = m_pDevice->CreateVertexShader(
                  Vertex, sizeof(Vertex), nullptr, &pVertexShader)))
      return hr;

   if (pInputLayout == nullptr)
   {
      return H_CHECK(ERROR_NOINTERFACE, L"");
   }
   else
   {
      if (pInputElementDescs == nullptr)
         return H_CHECK(E_INVALIDARG, L"");

      if (H_FAIL(hr = m_pDevice->CreateInputLayout(
                     pInputElementDescs,
                     NumElements,
                     Vertex, sizeof(Vertex),
                     &pInputLayout)))
         return hr;
      ;
   }

   if (H_FAIL(hr = m_pDevice->CreatePixelShader(Pixel, sizeof(Pixel), nullptr, &pPixelShader)))
      return hr;

   return S_OK;
};
 
