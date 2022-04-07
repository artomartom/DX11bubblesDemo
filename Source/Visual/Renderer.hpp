
#ifndef VISUAL_RENDER_HPP
#define VISUAL_RENDER_HPP

#include "../pch.hpp"


struct Vertex
{
    DirectX::XMFLOAT2 POSITION{};
};

struct Instance
{
    DirectX::XMFLOAT2 TRANSLATION{};
    float SIZE{};
    DirectX::XMFLOAT3 COLOR{};
};
class CRenderer
{
    friend class CDeviceResource;

    const ::Microsoft::WRL::ComPtr<ID3D11DeviceContext> &GetContext() const { return m_pContext; };
    const ::Microsoft::WRL::ComPtr<ID3D11RenderTargetView> &GetRenderTargetView() const { return m_pRTV; };

protected:
    void Set() const noexcept
    {
        UINT strides[]{sizeof(Vertex), sizeof(Instance)};
        UINT offsets[]{0, 0};

        ID3D11Buffer *buffers[2]{m_pVertexBuffer.Get(), m_pInstanceVertexBuffer.Get()};

        m_pContext->IASetInputLayout(m_pInputLayout.Get());
        m_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        m_pContext->IASetVertexBuffers(0u, _countof(buffers), buffers, strides, offsets);

        m_pContext->VSSetShader(m_pVertexShader.Get(), nullptr, 0);

        m_pContext->PSSetShader(m_pPixelShader.Get(), nullptr, 0);
        m_pContext->PSSetSamplers(0u, 1u, m_pSampler.GetAddressOf());
        m_pContext->PSSetShaderResources(0u, 1u, m_pCircleTexView.GetAddressOf());

        m_pContext->OMSetRenderTargets(1u, m_pRTV.GetAddressOf(), nullptr);
    };

    void Draw() const noexcept
    {
        static const float RTVClearColor[4]{0.f, 0.f, 0.f, 0.99f};
        m_pContext->ClearRenderTargetView(CRenderer::m_pRTV.Get(), RTVClearColor);
        m_pContext->DrawInstanced(m_DrawVertexCount,m_DrawInstanceCount,0u,0u);
    };

    UINT m_DrawVertexCount{};
    UINT m_DrawInstanceCount{};
    ::Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_pContext{};
    ::Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_pRTV{};

    ::Microsoft::WRL::ComPtr<ID3D11VertexShader> m_pVertexShader{};
    ::Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pPixelShader{};

    ::Microsoft::WRL::ComPtr<ID3D11InputLayout> m_pInputLayout{};
    ::Microsoft::WRL::ComPtr<ID3D11Buffer> m_pVertexBuffer{};
    ::Microsoft::WRL::ComPtr<ID3D11Buffer> m_pInstanceVertexBuffer{};

    ::Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pCircleTexView{};
    ::Microsoft::WRL::ComPtr<ID3D11SamplerState> m_pSampler{};
};

#endif