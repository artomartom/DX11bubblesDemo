

#pragma once
#include "DeviceResource.hpp"
#include <d3dcompiler.h>
#pragma comment(lib, "D3DCompiler.lib")

using ::Microsoft::WRL::ComPtr;
using namespace ::DirectX;

HRESULT CreateCircleTexture(
    _In_ const ComPtr<ID3D11Device> &d3dDevice,
    _In_ const ComPtr<ID3D11DeviceContext> &Context,
    _Out_ ID3D11Resource **ppTexture,
    _Out_ ID3D11ShaderResourceView **ppTextureView);

void CDeviceResource::DrawTestTriangle(CRenderer &Renderer)
{
   HRESULT hr;

   struct Vertex
   {
      float x;
      float y;
      float r;
      float g;
      float b;
   };

   // create vertex buffer (1 2d triangle at center of screen)
   const Vertex vertices[] =
       {
           {0.0f, 0.5f, 1.0f, 0.0f, 0.0f},
           {0.5f, -0.5f, 0.0f, 1.0f, 0.0f},
           {-0.5f, -0.5f, 0.0f, 0.0f, 1.0f},
       };
   ComPtr<ID3D11Buffer> pVertexBuffer;
   D3D11_BUFFER_DESC bd = {};
   bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
   bd.Usage = D3D11_USAGE_DEFAULT;
   bd.CPUAccessFlags = 0u;
   bd.MiscFlags = 0u;
   bd.ByteWidth = sizeof(vertices);
   bd.StructureByteStride = sizeof(Vertex);
   D3D11_SUBRESOURCE_DATA sd = {};
   sd.pSysMem = vertices;
   H_FAIL(m_pDevice->CreateBuffer(&bd, &sd, &pVertexBuffer));

   // Bind vertex buffer to pipeline
   const UINT stride = sizeof(Vertex);
   const UINT offset = 0u;
   Renderer.m_pContext->IASetVertexBuffers(0u, 1u, pVertexBuffer.GetAddressOf(), &stride, &offset);

   // create pixel shader
   ComPtr<ID3D11PixelShader> pPixelShader;
   ComPtr<ID3DBlob> pBlob;
   H_FAIL(D3DReadFileToBlob(L"F://Dev//Projects//bubblesDemo//Build//x64//Debug//PixelShader.so", &pBlob));
   H_FAIL(m_pDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pPixelShader));

   // bind pixel shader
   Renderer.m_pContext->PSSetShader(pPixelShader.Get(), nullptr, 0u);

   // create vertex shader
   ComPtr<ID3D11VertexShader> pVertexShader;
   H_FAIL(D3DReadFileToBlob(L"F://Dev//Projects//bubblesDemo//Build//x64//Debug//VertexShader.so", &pBlob));
   H_FAIL(m_pDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pVertexShader));

   // bind vertex shader
   Renderer.m_pContext->VSSetShader(pVertexShader.Get(), nullptr, 0u);

   // input (vertex) layout (2d position only)
   ComPtr<ID3D11InputLayout> pInputLayout;
   const D3D11_INPUT_ELEMENT_DESC ied[] =
       {
           {"Position", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
           {"Color", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 8u, D3D11_INPUT_PER_VERTEX_DATA, 0},
       };
   H_FAIL(m_pDevice->CreateInputLayout(
       ied, (UINT)std::size(ied),
       pBlob->GetBufferPointer(),
       pBlob->GetBufferSize(),
       &pInputLayout));

   // bind vertex layout
   Renderer.m_pContext->IASetInputLayout(pInputLayout.Get());

   // bind render target
   Renderer.m_pContext->OMSetRenderTargets(1u, Renderer.m_pRTV.GetAddressOf(), nullptr);

   // Set primitive topology to triangle list (groups of 3 vertices)
   Renderer.m_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

   (Renderer.m_pContext->Draw((UINT)std::size(vertices), 0u));
};

CDeviceResource::CDeviceResource(const HWND &hwnd, const SIZE &TargetSize, _Out_ ComPtr<ID3D11DeviceContext> &pContext)
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
   hr = CoInitialize(0);
   if (hr != S_OK && hr != S_FALSE)
   {
      return H_CHECK(hr, L"CoInitialize failed");
   };
   ComPtr<ID3D11Texture2D> backBuffer{};
   if (H_FAIL(hr = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &backBuffer)))
      return hr;
   if (H_FAIL(hr = m_pDevice->CreateRenderTargetView(backBuffer.Get(), 0, &Renderer.m_pRTV)))
      return hr;

   if (H_FAIL(hr = CreateCircleTexture(m_pDevice.Get(), Renderer.m_pContext.Get(), nullptr, &Renderer.m_pCircleTexView)))
      return hr;
   /**
    *    Create Vertex Shader
    */
   {
#include "../Shader/Debug/Vertex.hpp"

      const D3D11_INPUT_ELEMENT_DESC InputElementDescs[]{
          {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},

          {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1},

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
   };

   /**
    *    Pixel Shader
    */
   {
#include "../Shader/Debug/Pixel.hpp"
      if (H_FAIL(hr = m_pDevice->CreatePixelShader(PixelByteCode, sizeof(PixelByteCode), nullptr, &Renderer.m_pPixelShader)))
         return hr;
   };

   D3D11_BUFFER_DESC d_VertexBuffer{};
   D3D11_SUBRESOURCE_DATA d_VertexData{};

   /**
    *    Create Vertex Buffer
    */
   {
      Vertex Vertices[]{
          {{-0.5f, +0.5f}},
          {{+0.5f, +0.5f}},
          {{+0.5f, -0.5f}},
          {{-0.5f, +0.5f}},
          {{+0.5f, -0.5f}},
          {{-0.5f, -0.5f}},
      };
      Renderer.m_DrawVertexCount = _countof(Vertices);

      d_VertexBuffer.ByteWidth = sizeof(Vertices);
      d_VertexBuffer.Usage = D3D11_USAGE_DEFAULT;
      d_VertexBuffer.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
      d_VertexBuffer.StructureByteStride = sizeof(Vertex);

      d_VertexData.pSysMem = Vertices;

      if (H_FAIL(hr = m_pDevice->CreateBuffer(&d_VertexBuffer, &d_VertexData, &Renderer.m_pVertexBuffer)))
         return hr;
   };
   /**
    *     Create  Instance  Vertex  Buffer
    */
   {
      Instance Instancies[]{{{0.3f, 0.3f}}};

      Renderer.m_DrawInstanceCount = _countof(Instancies);

      d_VertexBuffer.ByteWidth = sizeof(Instancies);
      d_VertexBuffer.Usage = D3D11_USAGE_DEFAULT;
      d_VertexBuffer.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
      d_VertexBuffer.StructureByteStride = sizeof(Instance);

      d_VertexData.pSysMem = Instancies;

      if (H_FAIL(hr = m_pDevice->CreateBuffer(&d_VertexBuffer, &d_VertexData, &Renderer.m_pInstanceVertexBuffer)))
         return hr;
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

   CoUninitialize();
   return S_OK;
};
