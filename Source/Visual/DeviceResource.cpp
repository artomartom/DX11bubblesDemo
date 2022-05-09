

#pragma once
#include "DeviceResource.hpp"

using ::Microsoft::WRL::ComPtr;
using namespace ::DirectX;

HRESULT DeviceResource::TestDeviceSupport()
{
   return S_OK; // TODO
   HRESULT hr{};
   ComPtr<ID3D11Device> tmp_pDevice{};

   D3D_FEATURE_LEVEL thisFeatureLevel{};
   // first we create temporary Device to ...
   if (H_FAIL(hr = D3D11CreateDevice(
                  nullptr, // default adapter
                  D3D_DRIVER_TYPE_HARDWARE,
                  nullptr, 0,
                  nullptr, // use default array {    D3D_FEATURE_LEVEL_11_0,  ...,    D3D_FEATURE_LEVEL_9_1}
                  0,
                  D3D11_SDK_VERSION,
                  &tmp_pDevice,
                  &thisFeatureLevel,
                  nullptr)))
      return hr;

   /** ... check feature level
    * 2 UAVs (to texture2d, structured buffer) - FL 11.0 support 8
    * shader model 5
    */

   if (thisFeatureLevel < D3D_FEATURE_LEVEL_11_0)
   {
      return D3D12_ERROR_DRIVER_VERSION_MISMATCH;
   }

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

HRESULT DeviceResource::CreateSizeDependentDeviceResources(_In_ const HWND &windowHandle, _Inout_ Renderer &Renderer)
{
   HRESULT hr{};

   if (!windowHandle)
   {
      return ERROR_INVALID_HANDLE_STATE;
   };

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

HRESULT DeviceResource::CreateStructuredBuffer(
    _In_ UINT elementCount, _In_ UINT elementSize,
    _In_opt_ VOID *pData,
    _COM_Outptr_opt_ ID3D11Buffer **ppBuffer,
    _COM_Outptr_opt_ ID3D11UnorderedAccessView **ppUAV,
    _COM_Outptr_opt_ ID3D11ShaderResourceView **ppSRV)
{

   if (!elementCount || !elementSize)
      return ERROR_INVALID_PARAMETER;
   HRESULT hr{};
   D3D11_BUFFER_DESC d_Buffer{};

   ComPtr<ID3D11Buffer> pBuffer{};
   D3D11_SUBRESOURCE_DATA d_BufferData{pData, 0, 0};
   d_Buffer.ByteWidth = elementSize * elementCount;
   d_Buffer.Usage = D3D11_USAGE_DEFAULT;
   d_Buffer.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
   d_Buffer.StructureByteStride = elementSize;
   d_Buffer.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

   if (H_FAIL(hr = m_pDevice->CreateBuffer(&d_Buffer, (pData != nullptr) ? &d_BufferData : nullptr, &pBuffer)))
      return hr;

   SETDBGNAME_COM(pBuffer);

   if (ppUAV != nullptr)
   {
      D3D11_UNORDERED_ACCESS_VIEW_DESC descUAV{};
      descUAV.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
      descUAV.Format = DXGI_FORMAT_UNKNOWN;

      descUAV.Buffer.FirstElement = 0;
      descUAV.Buffer.NumElements = elementCount;
      descUAV.Buffer.Flags = 0;
      if (H_FAIL(hr = m_pDevice->CreateUnorderedAccessView(pBuffer.Get(), &descUAV, ppUAV)))
         return hr;
      SETDBGNAME(*ppUAV);
   }
   if (ppSRV != nullptr)
   {

      D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc{};
      SRVDesc.Format = DXGI_FORMAT_UNKNOWN;
      SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
      SRVDesc.Buffer.ElementOffset = 0;
      SRVDesc.Buffer.ElementWidth = elementSize;
      SRVDesc.Buffer.FirstElement = 0;
      SRVDesc.Buffer.NumElements = elementCount;

      if (H_FAIL(hr = m_pDevice->CreateShaderResourceView(pBuffer.Get(), &SRVDesc, ppSRV)))
         return hr;
      SETDBGNAME(*ppSRV);
   }

   if (ppBuffer != nullptr)
   {
      *ppBuffer = pBuffer.Detach();
   };

   return hr;
};

HRESULT DeviceResource::CreateDeviceResources(_Out_ Renderer &Renderer)
{
   HRESULT hr{};
   hr = CoInitialize(0);
   if (!H_CHECK(hr, L"CoInitialize failed") && hr != S_FALSE)
   {
      return hr;
   };

   if (H_FAIL(hr = ComputeCircleTexture(Renderer.m_pContext, {1, 280, 1}, {280, 280},
                                        nullptr, &Renderer.m_pCircleTexView)))
      return hr;

   /**
    *    Create Compute Shader
    */
   {
#ifdef _DEBUG
#include "../Shader/Debug/CSmain.hpp"
#else
#include "../Shader/Release/CSmain.hpp"
#endif

      if (H_FAIL(hr = m_pDevice->CreateComputeShader(CSByteCode, sizeof(CSByteCode), nullptr, &Renderer.m_pComputeShader)))
         return hr;
      SETDBGNAME_COM(Renderer.m_pComputeShader);
   };

   /**
    *    Create Vertex Shader
    */
   {

#ifdef _DEBUG
#include "../Shader/Debug/VSmain.hpp"
#else
#include "../Shader/Release/VSmain.hpp"
#endif

      if (H_FAIL(hr = m_pDevice->CreateVertexShader(
                     VSByteCode, sizeof(VSByteCode), nullptr, &Renderer.m_pVertexShader)))
         return hr;
      SETDBGNAME_COM(Renderer.m_pVertexShader);
   };

   /**
    *    Pixel Shader
    */
   {
#ifdef _DEBUG
#include "../Shader/Debug/PSmain.hpp"
#else
#include "../Shader/Release/PSmain.hpp"
#endif
      if (H_FAIL(hr = m_pDevice->CreatePixelShader(PSByteCode, sizeof(PSByteCode), nullptr, &Renderer.m_pPixelShader)))
         return hr;
      SETDBGNAME_COM(Renderer.m_pPixelShader);
   };

   /**
    *  Create instance structured buffer containing data for each circle. it's going to be initialized on GPU,
    *  so we don't use any data when creating the buffer. We also don't store point to the buffer itself, cause it
    *  is going to be bound as a structured buffer to vertex shader stage and compute shader, and once both SRV and
    *  UAV this buffer are created pointer to ID3D11Buffer can be released.
    */
   {

      if (H_FAIL(hr = CreateStructuredBuffer(
                     Renderer.s_DrawInstanceCount,
                     sizeof(InstanceData),
                     nullptr,                   // no initialization data, buffer is populated in compute shader
                     nullptr,                   // don't need ID3D11buffer pointer
                     &Renderer.m_InstBufferUAV, // Create UAV of structured Instance Buffer for compute shader
                     &Renderer.m_InstBufferSRV  // Create SRV of structured Instance Buffer for vertex shader
                     )))
         return hr;
   }
   /**
    * Create strucured buffer for data, private to Compute shader
    */
   {

      ComputeData Data[Renderer.s_DrawInstanceCount]{};

      std::srand(Time::GetLocalTime().wMilliseconds);

      for (auto &each : Data)
      {
         // float angle{XMConvertToRadians((std::rand() % 60) + 100.f)}; // 100 ... 160 degrees
         float angle{XMConvertToRadians(23.f)};
         XMFLOAT2 velocity{cos(angle), sin(angle)};

         XMFLOAT2 pos{.0, .0};

         each = {velocity, pos};
      };

      if (H_FAIL(hr = CreateStructuredBuffer(
                     Renderer.s_DrawInstanceCount,
                     sizeof(ComputeData),
                     Data,
                     nullptr, // don't need ID3D11buffer pointer
                     &Renderer.m_ComputeBufferUAV,
                     nullptr)))
         return hr;
   }
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
    _In_ const XMUINT3 &&threadGroupCount,
    _In_ const XMUINT2 &&imageSize,
    _Out_ ID3D11Resource **ppTexture,
    _Out_ ID3D11ShaderResourceView **ppTextureView)
{

   if (!pContext)
      return E_NOINTERFACE;

   if (imageSize.x == 0u || imageSize.y == 0u || threadGroupCount.x == 0u || threadGroupCount.y == 0u || threadGroupCount.z == 0u)
      return ERROR_INVALID_PARAMETER;

   if (m_pDevice->GetFeatureLevel() < D3D_FEATURE_LEVEL_11_0)
   {
      return ERROR_DEVICE_FEATURE_NOT_SUPPORTED;
   };

   using pixelT = DirectX::XMFLOAT4;
   static constexpr struct
   {
      DXGI_FORMAT dx{DXGI_FORMAT_R32G32B32A32_FLOAT};
      UINT bpp{sizeof(pixelT) * 8};
   } circleFormat{};

   HRESULT hr{};
   ComPtr<ID3D11Texture2D> pTexture{};
   ComPtr<ID3D11UnorderedAccessView> pTextureUAV{};
   /**
    * Create shader from byte code in header
    */
   ComPtr<ID3D11ComputeShader> pComputeShader{};
   {

#ifdef _DEBUG
#include "../Shader/Debug/CSCircle.hpp"
#else
#include "../Shader/Release/CSCircle.hpp"
#endif
      if (H_FAIL(hr = m_pDevice->CreateComputeShader(CSCircleByteCode, sizeof(CSCircleByteCode), nullptr, &pComputeShader)))
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
      DBG_ONLY(pTexture->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof("circleTexture") - 1, "circleTexture"));
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
