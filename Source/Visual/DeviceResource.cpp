

#pragma once
#include "DeviceResource.hpp"

using ::Microsoft::WRL::ComPtr;
using namespace ::DirectX;

HRESULT GenerateCircleTexture(
    _In_ const ComPtr<ID3D11Device> &d3dDevice,
    _In_ const ComPtr<ID3D11DeviceContext> &Context,
    _Out_ ID3D11Resource **ppTexture,
    _Out_ ID3D11ShaderResourceView **ppTextureView

);

HRESULT DeviceResource::TestDeviceSupport()
{
   HRESULT hr{};
   ComPtr<ID3D11Device> tmp_pDevice{};

   D3D_FEATURE_LEVEL featureLevels[1]{
       D3D_FEATURE_LEVEL_11_0,
   };
   D3D_FEATURE_LEVEL thisFeatureLevel{};
   // first we create temporary Device to ...
   H_CHECK(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, featureLevels, _countof(featureLevels), D3D11_SDK_VERSION, &tmp_pDevice, &thisFeatureLevel, nullptr),
           L"D3D11CreateDevice failed");

   //... check feature level
   // TODO

   UINT NumQualityLevels{};
   //... and multi-sampling support
   H_CHECK(hr = tmp_pDevice->CheckMultisampleQualityLevels(
               (DXGI_FORMAT)CircleTexFormat, // Circle Texture format used when decoding and loading texture  from resource
               Settings::SampleCount,        // sample count
               &NumQualityLevels),
           L"CheckMultisampleQualityLevels");

   if (hr < 0 || NumQualityLevels == 0)
   {
      hr = ERROR_DEVICE_FEATURE_NOT_SUPPORTED;
      Error<File>::Write(L"Multi-sampling: ERROR_DEVICE_FEATURE_NOT_SUPPORTED");
      // TODO Feature support exception
      return ERROR_DEVICE_FEATURE_NOT_SUPPORTED;
   };
   return S_OK;
};

DeviceResource::DeviceResource(const HWND &hwnd, const SIZE &TargetSize, _Out_ ComPtr<ID3D11DeviceContext> &pContext, _Out_ HRESULT *hr)
{
   HRESULT localhr{};
   if (hr == nullptr)
      hr = &localhr;

   // Create Actual Device
   unsigned int flags{};
   DBG_ONLY(flags |= D3D11_CREATE_DEVICE_DEBUG);
   DXGI_SWAP_CHAIN_DESC d_SwapChain{
       static_cast<UINT>(TargetSize.cx), static_cast<UINT>(TargetSize.cy),
       60, 1, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED,
       DXGI_MODE_SCALING_UNSPECIFIED, 1, 0, DXGI_USAGE_RENDER_TARGET_OUTPUT,
       1, hwnd, true, DXGI_SWAP_EFFECT_DISCARD, 0};

   //   d_SwapChain.SampleDesc = DXGI_SAMPLE_DESC{4, 0};

   H_CHECK(*hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, flags, 0, 0,
                                               D3D11_SDK_VERSION, &d_SwapChain, &m_pSwapChain, &m_pDevice, &m_thisFeatureLevel, &pContext),
           L"D3D11CreateDeviceAndSwapChain failed");
};

HRESULT DeviceResource::CreateDeviceResources(_Out_ Renderer &Renderer)
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

   if (H_FAIL(hr = GenerateCircleTexture(m_pDevice.Get(), Renderer.m_pContext.Get(), nullptr, &Renderer.m_pCircleTexView)))
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
          {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},

          {"TRANSLATION", 0, DXGI_FORMAT_R32G32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
          {"SIZE", 0, DXGI_FORMAT_R32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
          {"PERIOD", 0, DXGI_FORMAT_R32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
          {"COLOR", 0, DXGI_FORMAT_R32_UINT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
          {"STARTTIME", 0, DXGI_FORMAT_R32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},

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
#ifdef _DEBUG
#include "../Shader/Debug/Pixel.hpp"
#else
#include "../Shader/Release/Pixel.hpp"
#endif
      if (H_FAIL(hr = m_pDevice->CreatePixelShader(PixelByteCode, sizeof(PixelByteCode), nullptr, &Renderer.m_pPixelShader)))
         return hr;
   };

   D3D11_BUFFER_DESC d_VertexBuffer{};
   D3D11_SUBRESOURCE_DATA d_VertexData{};

   /**
    *    Create Vertex Buffer
    */
   {

      Vertex Vertices[Renderer.s_DrawVertexCount]{
          {{-1.f, +1.0f}},
          {{+1.f, +1.0f}},
          {{+1.f, -1.0f}},
          {{-1.f, +1.0f}},
          {{+1.f, -1.0f}},
          {{-1.f, -1.0f}},
      };

      d_VertexBuffer.ByteWidth = sizeof(Vertices);
      d_VertexBuffer.Usage = D3D11_USAGE_IMMUTABLE;
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

      d_VertexBuffer.ByteWidth = sizeof(Instance) * Renderer.s_DrawInstanceCount;
      d_VertexBuffer.Usage = D3D11_USAGE_DEFAULT;
      d_VertexBuffer.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
      d_VertexBuffer.StructureByteStride = sizeof(Instance);

      d_VertexData.pSysMem = &Renderer.m_Instancies[0];

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
   /**
    *     Create Constant Buffers
    */
   {

      // ViewPortSizeBuffer   /*    */ m_pViewPortSizeBuffer
      // FrameBuffer   /*           */ m_pFrameBuffer
      // ColorsBuffer   /*          */ m_pColorsBuffer

      D3D11_BUFFER_DESC d_ConstBuffer{};

      d_ConstBuffer.Usage = D3D11_USAGE_DEFAULT;
      d_ConstBuffer.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
      d_ConstBuffer.CPUAccessFlags = 0;
      d_ConstBuffer.ByteWidth = (sizeof(ViewPortSizeBuffer) + 15) / 16 * 16;
      if (H_FAIL(hr = m_pDevice->CreateBuffer(&d_ConstBuffer, nullptr, &Renderer.m_pViewPortSizeBuffer)))
         return hr;

      d_ConstBuffer.Usage = D3D11_USAGE_DEFAULT;
      d_ConstBuffer.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
      d_ConstBuffer.CPUAccessFlags = 0;
      d_ConstBuffer.ByteWidth = (sizeof(FrameBuffer) + 15) / 16 * 16;
      if (H_FAIL(hr = m_pDevice->CreateBuffer(&d_ConstBuffer, nullptr, &Renderer.m_pFrameBuffer)))
         return hr;

      d_ConstBuffer.Usage = D3D11_USAGE_IMMUTABLE;
      d_ConstBuffer.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
      d_ConstBuffer.CPUAccessFlags = 0;
      d_ConstBuffer.ByteWidth = (sizeof(ColorsBuffer) + 15) / 16 * 16;

      ColorsBuffer d_defaultColorsBuffer{};
      D3D11_SUBRESOURCE_DATA d_ConstBufferData{&d_defaultColorsBuffer, 0, 0};

      if (H_FAIL(hr = m_pDevice->CreateBuffer(&d_ConstBuffer, &d_ConstBufferData, &Renderer.m_pColorsBuffer)))
         return hr;
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
   };

   CoUninitialize();
   return S_OK;
};

template <typename T>
struct Vector2
{
   T x, y;
};

HRESULT GenerateCircleTexture(
    _In_ const ComPtr<ID3D11Device> &d3dDevice,
    _In_ const ComPtr<ID3D11DeviceContext> &Context,
    _Out_ ID3D11Resource **ppTexture,
    _Out_ ID3D11ShaderResourceView **ppTextureView)
{
   HRESULT hr{};
   bool autogen{false};

   Vector2<UINT> imageSize{800, 800};

   DXGI_FORMAT DXFormat{DXGI_FORMAT_R8_UNORM};

   UINT bpp{8};

   UINT Stride = (imageSize.x * bpp + 7) / 8;
   UINT BufferSize = imageSize.y * Stride;
   std::unique_ptr<BYTE[]> Buffer{std::make_unique<BYTE[]>(BufferSize)};
   Vector2<UINT> Center{imageSize.x / 2, imageSize.y / 2};

   auto GetPos{
       [&](UINT Index) -> Vector2<float>
       {
          return Vector2<float>{float(Index / imageSize.x) / imageSize.x * 2.f - 1.f, float(Index % imageSize.x) / imageSize.y * 2.f - 1.f};
       }};

   for (UINT index{}; index != BufferSize; index++)
   {
      auto Pos{GetPos(index)};

      if (std::sqrtf(Pos.x * Pos.x + Pos.y * Pos.y) > 1.f)
         Buffer[index] = static_cast<BYTE>(0x00);
      else
         Buffer[index] = static_cast<BYTE>(0xFF);
   };

   // Create texture
   D3D11_TEXTURE2D_DESC desc{};
   desc.Width = imageSize.x;
   desc.Height = imageSize.y;
   desc.MipLevels = (autogen) ? 0 : 1;
   desc.ArraySize = 1;
   desc.Format = DXFormat;
   desc.SampleDesc.Count = 1;
   desc.SampleDesc.Quality = 0;
   desc.Usage = D3D11_USAGE_DEFAULT;
   desc.BindFlags = (autogen) ? (D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET) : (D3D11_BIND_SHADER_RESOURCE);
   desc.CPUAccessFlags = 0;
   desc.MiscFlags = (autogen) ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0;

   D3D11_SUBRESOURCE_DATA initData;
   initData.pSysMem = Buffer.get();
   initData.SysMemPitch = static_cast<UINT>(Stride);
   initData.SysMemSlicePitch = static_cast<UINT>(BufferSize);

   ComPtr<ID3D11Texture2D> tex{};
   if (H_FAIL(hr = d3dDevice->CreateTexture2D(&desc, (autogen) ? nullptr : &initData, &tex)))
      return hr;

   if (tex.Get() != nullptr)
   {
      if (ppTextureView != nullptr)
      {
         D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc{};
         SRVDesc.Format = DXFormat;
         SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
         SRVDesc.Texture2D.MipLevels = (autogen) ? -1 : 1;

         if (H_FAIL(hr = d3dDevice->CreateShaderResourceView(tex.Get(), &SRVDesc, ppTextureView)))
            return hr;

         if (autogen)
         {

            Context->UpdateSubresource(tex.Get(), 0, nullptr, Buffer.get(), static_cast<UINT>(Stride), static_cast<UINT>(BufferSize));
            Context->GenerateMips(*ppTextureView);
         }
      }

      if (ppTexture != nullptr)
      {
         *ppTexture = tex.Detach();
      }
   }
   return S_OK;
};
