
#ifndef VISUAL_RENDER_HPP
#define VISUAL_RENDER_HPP

#include "../pch.hpp"

struct ViewPortSizeBuffer
{
    DirectX::XMFLOAT2 ViewPortSize{};
};

struct FrameBuffer
{
    explicit FrameBuffer(long long t)
        : Time{t / 20., t, t / 1000, t % 1000} {/* Log<Console>::Write(Time.x,Time.y,Time.z,Time.w);*/};

    DirectX::XMFLOAT4 Time{};
};

/**
 *  (sizeof(ColorsBuffer )%16 ==0  )
 *  used by shader as a float3 [6]
 */
struct ColorsBuffer
{
    // DirectXColors.h constants
    DirectX::XMFLOAT3 LightYellow{1.000000000f, 1.000010000f, 0.878431439f};
    DirectX::XMFLOAT3 OliveDrab{0.419607878f, 0.556862772f, 0.137254909f};
    DirectX::XMFLOAT3 PapayaWhip{1.000000000f, 0.937254965f, 0.835294187f};
    DirectX::XMFLOAT3 SaddleBrown{0.545098066f, 0.270588249f, 0.074509807f};
    DirectX::XMFLOAT3 Tan{0.823529482f, 0.705882370f, 0.549019635f};
    DirectX::XMFLOAT3 Violet{0.933333397f, 0.509803951f, 0.933333397f};
};

struct Vertex
{
    DirectX::XMFLOAT2 POSITION{};
};

struct Instance
{
    DirectX::XMFLOAT2 TRANSLATION{};
    float SIZE{};
    UINT COLOR{};
};

class CRenderer
{
    friend class CDeviceResource;

    const ::Microsoft::WRL::ComPtr<ID3D11DeviceContext> &GetContext() const { return m_pContext; };
    const ::Microsoft::WRL::ComPtr<ID3D11RenderTargetView> &GetRenderTargetView() const { return m_pRTV; };

protected:
    void SetPipeLine() const noexcept
    {
        UINT strides[]{sizeof(Vertex), sizeof(Instance)};
        UINT offsets[]{0, 0};

        ID3D11Buffer *buffers[2]{m_pVertexBuffer.Get(), m_pInstanceVertexBuffer.Get()};
        ID3D11Buffer *constBuffers[3]{
            m_pViewPortSizeBuffer.Get(),
            m_pFrameBuffer.Get(),
            m_pColorsBuffer.Get(),
        };

        m_pContext->IASetInputLayout(m_pInputLayout.Get());
        m_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        m_pContext->IASetVertexBuffers(0u, _countof(buffers), buffers, strides, offsets);

        m_pContext->VSSetShader(m_pVertexShader.Get(), nullptr, 0);
        m_pContext->VSSetConstantBuffers(0, _countof(constBuffers), constBuffers);
        m_pContext->PSSetShader(m_pPixelShader.Get(), nullptr, 0);
        m_pContext->PSSetSamplers(0u, 1u, m_pSampler.GetAddressOf());
        m_pContext->PSSetShaderResources(0u, 1u, m_pCircleTexView.GetAddressOf());
        m_pContext->PSSetConstantBuffers(0, _countof(constBuffers), constBuffers);

        m_pContext->OMSetRenderTargets(1u, m_pRTV.GetAddressOf(), nullptr);
    };

    void UpdateFrameBuffer() const noexcept
    {
        static Timer::CTimer Timer{};

        FrameBuffer constantBuffer{Timer.Count<long>()};

        m_pContext->UpdateSubresource(
            m_pFrameBuffer.Get(),
            0, nullptr, &constantBuffer, 0, 0);
    };

    void UpdateViewPortSizeBuffer(float Width, float Height) const noexcept
    {

        ViewPortSizeBuffer ViewPortSize{{Width, Height}};
        m_pContext->UpdateSubresource(
            m_pViewPortSizeBuffer.Get(),
            0, nullptr, &ViewPortSize, 0, 0);

      
    };

    void SetViewPort(float Width, float Height)
    {

        D3D11_VIEWPORT ViewPortDesc{};
        ViewPortDesc.Width = Width;
        ViewPortDesc.Height = Height;
        ViewPortDesc.MinDepth = 0.f;
        ViewPortDesc.MaxDepth = 1.f;
        ViewPortDesc.TopLeftX = 0.f;
        ViewPortDesc.TopLeftY = 0.f;
        CRenderer::m_pContext->RSSetViewports(1, &ViewPortDesc);
    };

    void Draw() const noexcept
    {
        static const float RTVClearColor[4]{0.f, 0.f, 0.f, 0.99f};
        m_pContext->ClearRenderTargetView(CRenderer::m_pRTV.Get(), RTVClearColor);
        m_pContext->DrawInstanced(m_DrawVertexCount, m_DrawInstanceCount, 0u, 0u);
    };

    UINT m_DrawVertexCount{};
    UINT m_DrawInstanceCount{};
    ::Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_pContext{};
    ::Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_pRTV{};

    ::Microsoft::WRL::ComPtr<ID3D11VertexShader> m_pVertexShader{};
    ::Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pPixelShader{};

    ::Microsoft::WRL::ComPtr<ID3D11InputLayout> m_pInputLayout{};

    ::Microsoft::WRL::ComPtr<ID3D11Buffer> m_pVertexBuffer{};         // 6 point draw a quad , never changes
    ::Microsoft::WRL::ComPtr<ID3D11Buffer> m_pInstanceVertexBuffer{}; // data for each circle
    ::Microsoft::WRL::ComPtr<ID3D11Buffer> m_pViewPortSizeBuffer{};   // Changes On resizing
    ::Microsoft::WRL::ComPtr<ID3D11Buffer> m_pFrameBuffer{};          // changes every update
    ::Microsoft::WRL::ComPtr<ID3D11Buffer> m_pColorsBuffer{};         // Never Changes

    ::Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pCircleTexView{};
    ::Microsoft::WRL::ComPtr<ID3D11SamplerState> m_pSampler{};
};

#endif