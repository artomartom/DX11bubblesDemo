

#pragma once
#include "DeviceResource.hpp"

using ::Microsoft::WRL::ComPtr;
using namespace ::DirectX;

HRESULT CreateCircleTexture(
    _In_ const ComPtr<ID3D11Device> &d3dDevice,
    _In_ const ComPtr<ID3D11DeviceContext> &Context,
    _Out_ ID3D11Resource **ppTexture,
    _Out_ ID3D11ShaderResourceView **ppTextureView);

CDeviceResource::CDeviceResource(const HWND &hwnd, const SIZE &TargetSize, _Out_ ::Microsoft::WRL::ComPtr<ID3D11DeviceContext> &pContext)
{

   unsigned int flags{};

   DBG_ONLY(flags |= D3D11_CREATE_DEVICE_DEBUG);

   DXGI_SWAP_CHAIN_DESC d_SwapChain{
       static_cast<UINT>(TargetSize.cx), static_cast<UINT>(TargetSize.cy),
       60, 1, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED,
       DXGI_MODE_SCALING_UNSPECIFIED, 1, 0, DXGI_USAGE_RENDER_TARGET_OUTPUT,
       1, hwnd, true, DXGI_SWAP_EFFECT_DISCARD, 0};

   H_CHECK(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, flags, 0, 0,
                                         D3D11_SDK_VERSION, &d_SwapChain, &m_pSwapChain, &m_pDevice, &m_featureLevel, &pContext),
           L"D3D11CreateDeviceAndSwapChain failed");
};

HRESULT CDeviceResource::CreateDeviceResources(_Out_ CRenderer &Renderer)
{
   HRESULT hr{};
   hr=CoInitializeEx(0, COINIT_MULTITHREADED);
   if (hr != S_OK && hr != S_FALSE)
   {
      return H_CHECK(hr, L"CoInitialize failed");
   };
   ComPtr<ID3D11Texture2D> backBuffer{};
   if (H_FAIL(hr = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &backBuffer)))
      return hr;
   if (H_FAIL(hr = m_pDevice->CreateRenderTargetView(backBuffer.Get(), 0, &Renderer.m_pRTV)))
      return hr;
   DBG_ONLY({
      if (H_FAIL(hr = CDebugInterface::Init(m_pDevice)))
      return hr; });
  

   if (H_FAIL(hr = CreateCircleTexture(m_pDevice.Get(), Renderer.m_pContext.Get(), nullptr, &Renderer.m_pCircleTexView)))
      return hr;
   /**
    *  Create Rasterizer State
    */
   {
      D3D11_RASTERIZER_DESC d_RasterizerState{};

      d_RasterizerState.FillMode = D3D11_FILL_SOLID;
      d_RasterizerState.CullMode = D3D11_CULL_NONE;
      d_RasterizerState.DepthClipEnable = TRUE;

      if (H_FAIL(hr = m_pDevice->CreateRasterizerState(&d_RasterizerState, &Renderer.m_pRasterizer)))
         return hr;
   };

   /**
    *     Vertex Shader & Vertex Buffers
    */
   {
#include "../Shader/Release/Vertex.hpp"

      D3D11_INPUT_ELEMENT_DESC InputElementDescs[]{
          {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},

          //  {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
      };

      if (H_FAIL(hr = m_pDevice->CreateVertexShader(
                     VertexByteCode, sizeof(VertexByteCode), nullptr, &Renderer.m_pVertexShader)))
         return hr;

      if (H_FAIL(hr = m_pDevice->CreateInputLayout(
                     InputElementDescs,
                     _countof(InputElementDescs),
                     VertexByteCode, sizeof(VertexByteCode),
                     &Renderer.m_pInputLayout)))
         return hr;
      D3D11_BUFFER_DESC d_VertexBuffer{};
      D3D11_SUBRESOURCE_DATA d_VertexData{};
      /**
       *    Create Vertex Buffer
       */
      {
         struct
         {

            DirectX::XMFLOAT2 TEXCOORD{};
         } Vertices[]{
             {{0.f, 0.f}},
             {{.99f, 0.f}},
             {{.99f, .99f}},
             {{0.f, .99f}},
             {{0.f, 0.f}},
             {{.99f, .99f}},
         };

         d_VertexBuffer.ByteWidth = sizeof(Vertices);
         d_VertexBuffer.Usage = D3D11_USAGE_DEFAULT;
         d_VertexBuffer.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
         d_VertexBuffer.StructureByteStride = _countof(Vertices);

         d_VertexData.pSysMem = Vertices;
         d_VertexData.SysMemPitch = sizeof(Vertices);
         d_VertexData.SysMemSlicePitch = sizeof(Vertices[0]);

         if (H_FAIL(hr = m_pDevice->CreateBuffer(&d_VertexBuffer, &d_VertexData, &Renderer.m_pVertexBuffer)))
            return hr;
      };
      /**
       *     Create  Instance  Vertex  Buffer
       */
      {
         struct
         {
            DirectX::XMFLOAT2 POSITION{};
         } Vertices[]{{{0.2f, 0.2f}}};


         d_VertexBuffer.ByteWidth = sizeof(Vertices);
         d_VertexBuffer.Usage = D3D11_USAGE_DEFAULT;
         d_VertexBuffer.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
         d_VertexBuffer.StructureByteStride = _countof(Vertices);
         d_VertexData.pSysMem = Vertices;
         d_VertexData.SysMemPitch = sizeof(Vertices);
         d_VertexData.SysMemSlicePitch = sizeof(Vertices[0]);

               if (H_FAIL(hr = m_pDevice->CreateBuffer(&d_VertexBuffer, &d_VertexData, &Renderer.m_pInstanceVertexBuffer)))
            return hr;
      };
   };

   /**
    *     Sampler
    */
   {

      D3D11_SAMPLER_DESC SamplerDesc{};
      SamplerDesc.Filter = D3D11_FILTER::D3D11_FILTER_MIN_MAG_MIP_LINEAR;
      SamplerDesc.AddressW = SamplerDesc.AddressV = SamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_WRAP;
      SamplerDesc.ComparisonFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_NEVER;
      SamplerDesc.MinLOD = 0;
      SamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

      if (H_FAIL(hr = m_pDevice->CreateSamplerState(&SamplerDesc, &Renderer.m_pSampler)))
         return hr;
   };
   /**
    *    Pixel Shader
    */
   {
#include "../Shader/Release/Pixel.hpp"
      if (H_FAIL(hr = m_pDevice->CreatePixelShader(PixelByteCode, sizeof(PixelByteCode), nullptr, &Renderer.m_pPixelShader)))
         return hr;
   };

   CoUninitialize();
   return S_OK;
};
