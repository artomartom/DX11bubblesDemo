

#pragma once
#include "DeviceResource.hpp"

using ::Microsoft::WRL::ComPtr;
using namespace ::DirectX;

HRESULT DeviceResource::TestDeviceSupport()
{
   return S_OK; // TODO
   // HRESULT hr{};
   // ComPtr<ID3D11Device> tmp_pDevice{};
   //
   // D3D_FEATURE_LEVEL featureLevels[1]{
   //    D3D_FEATURE_LEVEL_11_0,
   //};
   // D3D_FEATURE_LEVEL thisFeatureLevel{};
   //// first we create temporary Device to ...
   // H_CHECK(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, featureLevels, _countof(featureLevels), D3D11_SDK_VERSION, &tmp_pDevice, &thisFeatureLevel, nullptr),
   //         L"D3D11CreateDevice failed");

   //... check feature level
   // TODO

   // if (hr < 0)
   //{
   //    hr = ERROR_DEVICE_FEATURE_NOT_SUPPORTED;
   //    Error<File>::Write(L"ERROR_DEVICE_FEATURE_NOT_SUPPORTED");
   //    // TODO Feature support exception
   //    return ERROR_DEVICE_FEATURE_NOT_SUPPORTED;
   // };
   // return S_OK;
};

DeviceResource::DeviceResource(_Out_ Renderer &Renderer, _Out_ HRESULT *hr)
    : m_numBackBuffers{2}
{
   HRESULT localhr{};
   if (hr == nullptr)
      hr = &localhr;
   //
   //  Create Actual Device

   unsigned int flags{};
   DBG_ONLY(flags |= D3D11_CREATE_DEVICE_DEBUG);

   H_CHECK(*hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, flags, 0, 0, D3D11_SDK_VERSION, &m_pDevice, &m_thisFeatureLevel, &Renderer.m_pContext),
           L"D3D11CreateDevice  failed");

   DBG_ONLY(
       {
          if (D3D11_CREATE_DEVICE_DEBUG && m_pDevice->GetCreationFlags())
             if (H_FAIL(*hr = DebugInterface::Init(m_pDevice)))
                return;
       });

   ComPtr<IDXGIDevice4> pDXGIDevice{};
   ComPtr<IDXGIAdapter> pDXGIAdapter{};

   if (H_FAIL(*hr = m_pDevice->QueryInterface(__uuidof(IDXGIDevice4), (void **)&pDXGIDevice)))
      return;

   if (H_FAIL(*hr = pDXGIDevice->GetAdapter(&pDXGIAdapter)))
      return;

   if (H_FAIL(*hr = pDXGIAdapter->GetParent(__uuidof(IDXGIFactory4), (void **)&m_pDXGIFactory)))
      return;
   SETDBGNAME_COM(m_pDXGIFactory);
};

HRESULT DeviceResource::CreateSizeDependentDeviceResources(const HWND &windowHandle, Renderer &Renderer)
{
   HRESULT hr{};

   Renderer.m_pContext->OMSetRenderTargets(0, nullptr, nullptr);
   Renderer.m_pRTV.Reset();
   Renderer.m_pRenderTarget.Reset();
   Renderer.m_pContext->Flush();

   if (m_pSwapChain)
   {

      H_FAIL(hr = m_pSwapChain->ResizeBuffers(
                 GetNumBackBuffer(),
                 Renderer.m_ViewPort.Width, Renderer.m_ViewPort.Height,
                 DXGI_FORMAT_UNKNOWN, 0u));

      if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
      {
         H_FAIL(HandleDeviceRemoved());
      };
   }
   else
   {

      DXGI_SWAP_CHAIN_DESC1 d_swapChain{};
      d_swapChain.Width = static_cast<UINT>(Renderer.m_ViewPort.Width);
      d_swapChain.Height = static_cast<UINT>(Renderer.m_ViewPort.Height);
      d_swapChain.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
      d_swapChain.Stereo = false;
      d_swapChain.SampleDesc = {1, 0};
      d_swapChain.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
      d_swapChain.BufferCount = m_numBackBuffers;
      d_swapChain.Scaling = DXGI_SCALING::DXGI_SCALING_STRETCH;
      d_swapChain.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
      d_swapChain.AlphaMode = DXGI_ALPHA_MODE ::DXGI_ALPHA_MODE_UNSPECIFIED;
      d_swapChain.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

      DXGI_SWAP_CHAIN_FULLSCREEN_DESC d_fullScreenSwapChain{};
      d_fullScreenSwapChain.RefreshRate = {0u, 1u};
      d_fullScreenSwapChain.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER ::DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
      d_fullScreenSwapChain.Scaling = DXGI_MODE_SCALING ::DXGI_MODE_SCALING_STRETCHED;
      d_fullScreenSwapChain.Windowed = true;
      if (H_FAIL(hr = m_pDXGIFactory->CreateSwapChainForHwnd(m_pDevice.Get(), windowHandle, &d_swapChain, &d_fullScreenSwapChain, nullptr, &m_pSwapChain)))
         return hr;
   };
   if (H_FAIL(hr = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &Renderer.m_pRenderTarget)))
      return hr;
   if (H_FAIL(hr = m_pDevice->CreateRenderTargetView(Renderer.m_pRenderTarget.Get(), 0, &Renderer.m_pRTV)))
      return hr;

   Renderer.SetPipeLine();
   return S_OK;
};

HRESULT DeviceResource::CreateDeviceResources(_Out_ Renderer &Renderer)
{
   HRESULT hr{};
   hr = CoInitialize(0);
   if (hr != S_OK && hr != S_FALSE)
   {
      return H_CHECK(hr, L"CoInitialize failed");
   };

   if (H_FAIL(hr = ComputeCircleTexture(Renderer.m_pContext, nullptr, &Renderer.m_pCircleTexView,
                                        {1, 280, 1}, {280, 280})))
      return hr;

   /**
    *    Create Vertex Shader
    */
   {

#ifdef _DEBUG
#include "../Shader/Debug/Vertex.hpp"
#else
#include "../Shader/Release/Vertex.hpp"
#endif

      const D3D11_INPUT_ELEMENT_DESC InputElementDescs[]{
          {"TRANSLATION", 0, DXGI_FORMAT_R32G32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
          {"SIZE", 0, DXGI_FORMAT_R32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
          {"PERIOD", 0, DXGI_FORMAT_R32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
          {"COLOR", 0, DXGI_FORMAT_R32_UINT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
          {"STARTTIME", 0, DXGI_FORMAT_R32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},

      };

      if (H_FAIL(hr = m_pDevice->CreateVertexShader(
                     VertexByteCode, sizeof(VertexByteCode), nullptr, &Renderer.m_pVertexShader)))
         return hr;
      SETDBGNAME_COM(Renderer.m_pVertexShader);
      if (H_FAIL(hr = m_pDevice->CreateInputLayout(
                     InputElementDescs,
                     _countof(InputElementDescs),
                     VertexByteCode, sizeof(VertexByteCode),
                     &Renderer.m_pInputLayout)))
         return hr;
      SETDBGNAME_COM(Renderer.m_pInputLayout);
   };

   /**
    *    Pixel Shader
    */
   {
#ifdef _DEBUG
#include "../Shader/Debug/Pixel.hpp"
#else
#include "../Shader/Release/Pixel.hpp"
#endif
      if (H_FAIL(hr = m_pDevice->CreatePixelShader(PixelByteCode, sizeof(PixelByteCode), nullptr, &Renderer.m_pPixelShader)))
         return hr;
      SETDBGNAME_COM(Renderer.m_pPixelShader);
   };

   D3D11_BUFFER_DESC d_VertexBuffer{};
   D3D11_SUBRESOURCE_DATA d_VertexData{};

   /**
    *     Create  Instance  Vertex  Buffer
    */
   {

      d_VertexBuffer.ByteWidth = sizeof(Instance) * Renderer.s_DrawInstanceCount;
      d_VertexBuffer.Usage = D3D11_USAGE_DEFAULT;
      d_VertexBuffer.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
      d_VertexBuffer.StructureByteStride = sizeof(Instance);

      d_VertexData.pSysMem = &Renderer.m_Instancies[0];

      if (H_FAIL(hr = m_pDevice->CreateBuffer(&d_VertexBuffer, &d_VertexData, &Renderer.m_pInstanceVertexBuffer)))
         return hr;
      SETDBGNAME_COM(Renderer.m_pInstanceVertexBuffer);
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
      SETDBGNAME_COM(Renderer.m_pSampler);
   };
   /**
    *     Create Constant Buffers
    */
   {

      // ViewPortSizeBuffer   /*    */ m_pViewPortSizeBuffer
      // FrameBuffer   /*           */ m_pFrameBuffer

      D3D11_BUFFER_DESC d_ConstBuffer{};

      d_ConstBuffer.Usage = D3D11_USAGE_DEFAULT;
      d_ConstBuffer.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
      d_ConstBuffer.CPUAccessFlags = 0;
      d_ConstBuffer.ByteWidth = (sizeof(ViewPortSizeBuffer) + 15) / 16 * 16;
      if (H_FAIL(hr = m_pDevice->CreateBuffer(&d_ConstBuffer, nullptr, &Renderer.m_pViewPortSizeBuffer)))
         return hr;
      SETDBGNAME_COM(Renderer.m_pViewPortSizeBuffer);

      d_ConstBuffer.Usage = D3D11_USAGE_DEFAULT;
      d_ConstBuffer.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
      d_ConstBuffer.CPUAccessFlags = 0;
      d_ConstBuffer.ByteWidth = (sizeof(FrameBuffer) + 15) / 16 * 16;
      if (H_FAIL(hr = m_pDevice->CreateBuffer(&d_ConstBuffer, nullptr, &Renderer.m_pFrameBuffer)))
         return hr;

      SETDBGNAME_COM(Renderer.m_pFrameBuffer);
   };

   /**
    *     Create Blend State
    */
   {

      D3D11_BLEND_DESC d_BlendState{};
      D3D11_RENDER_TARGET_BLEND_DESC d_RTBlend{};

      d_RTBlend.BlendEnable = true;

      d_RTBlend.SrcBlend = D3D11_BLEND::D3D11_BLEND_SRC_ALPHA;
      d_RTBlend.DestBlend = D3D11_BLEND::D3D11_BLEND_INV_SRC_ALPHA;
      d_RTBlend.BlendOp = D3D11_BLEND_OP::D3D11_BLEND_OP_ADD;

      d_RTBlend.SrcBlendAlpha = D3D11_BLEND::D3D11_BLEND_ONE;
      d_RTBlend.DestBlendAlpha = D3D11_BLEND::D3D11_BLEND_ONE;
      d_RTBlend.BlendOpAlpha = D3D11_BLEND_OP::D3D11_BLEND_OP_SUBTRACT;

      d_RTBlend.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE::D3D11_COLOR_WRITE_ENABLE_ALL;

      d_BlendState.RenderTarget[0] = d_RTBlend;

      if (H_FAIL(hr = m_pDevice->CreateBlendState(&d_BlendState, &Renderer.m_pBlend)))
         return hr;
      SETDBGNAME_COM(Renderer.m_pBlend);
   };

   CoUninitialize();
   return S_OK;
};

HRESULT DeviceResource::ComputeCircleTexture(
    _In_ const ComPtr<ID3D11DeviceContext> &pContext,

    _Out_ ID3D11Resource **ppTexture,
    _Out_ ID3D11ShaderResourceView **ppTextureView,
    const XMUINT3 &&threadGroupCount,
    const XMUINT2 &&imageSize)

{

   if (!pContext)
      return E_NOINTERFACE;

   /** compute shader requirements:
    * RWTexture2D - FL 11.0
    *
    */
   if (m_pDevice->GetFeatureLevel() < D3D_FEATURE_LEVEL_11_0)
   {
      return ERROR_DEVICE_FEATURE_NOT_SUPPORTED;
   }
   static constexpr struct
   {
      DXGI_FORMAT dx{DXGI_FORMAT_R32_FLOAT};
      UINT bpp{32};
   } circleFormat{};
   using pixelT = float;

   HRESULT hr{};
   ComPtr<ID3D11Texture2D> pTexture{};
   ComPtr<ID3D11UnorderedAccessView> pTextureUAV{};
   /**
    * Create shader from byte code in header
    */
   ComPtr<ID3D11ComputeShader> pComputeShader{};
   {

#ifdef _DEBUG
#include "../Shader/Debug/Compute.hpp"
#else
#include "../Shader/Release/Compute.hpp"
#endif
      if (H_FAIL(hr = m_pDevice->CreateComputeShader(ComputeByteCode, sizeof(ComputeByteCode), nullptr, &pComputeShader)))
         return hr;
   }

   /**
    * Create texture  we write to
    */
   {

      D3D11_TEXTURE2D_DESC desc{};
      desc.Width = imageSize.x;
      desc.Height = imageSize.y;
      desc.MipLevels = 0;
      desc.ArraySize = 1;
      desc.Format = circleFormat.dx;
      desc.SampleDesc.Count = 1;
      desc.SampleDesc.Quality = 0;
      desc.Usage = D3D11_USAGE_DEFAULT;
      desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
      desc.CPUAccessFlags = 0;
      desc.MiscFlags = 0;

      if (H_FAIL(hr = m_pDevice->CreateTexture2D(&desc, nullptr, &pTexture)))
         return hr;
      pTexture->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof("circleTexture") - 1, "circleTexture");
   }
   /**
    * Create Unordered Access View of  texture we write to
    */
   {
      D3D11_UNORDERED_ACCESS_VIEW_DESC descUAV{};
      descUAV.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
      descUAV.Buffer.FirstElement = 0;
      descUAV.Format = circleFormat.dx;

      if (H_FAIL(hr = m_pDevice->CreateUnorderedAccessView(pTexture.Get(), &descUAV, &pTextureUAV)))
         return hr;
   }
   /**
    * Start computation
    */
   {
      pContext->CSSetShader(pComputeShader.Get(), nullptr, 0);
      pContext->CSSetUnorderedAccessViews(0, 1, pTextureUAV.GetAddressOf(), nullptr);
      pContext->Dispatch(threadGroupCount.x, threadGroupCount.y, threadGroupCount.z);
      pContext->ClearState();
   }

   if (ppTexture != nullptr)
   {
      *ppTexture = pTexture.Detach();
   }
   if (ppTextureView != nullptr)
   {
      D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc{};
      SRVDesc.Format = circleFormat.dx;
      SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
      SRVDesc.Texture2D.MipLevels = 1;

      H_FAIL(hr = m_pDevice->CreateShaderResourceView(pTexture.Get(), &SRVDesc, ppTextureView));
   }

   return S_OK;
};
