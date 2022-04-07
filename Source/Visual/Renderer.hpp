
#ifndef VISUAL_RENDER_HPP
#define VISUAL_RENDER_HPP

#include "../pch.hpp"

class CRenderer
{
    friend class CDeviceResource;

    const ::Microsoft::WRL::ComPtr<ID3D11DeviceContext> &GetContext() const { return m_pContext; };
    const ::Microsoft::WRL::ComPtr<ID3D11RenderTargetView> &GetRenderTargetView() const { return m_pRTV; };

protected:
    void Set() const noexcept
    {
        

        UINT strides[2]{sizeof(DirectX::XMFLOAT2) , sizeof(DirectX::XMFLOAT2)};
        UINT offsets[2]{};

        ID3D11Buffer *buffers[2]{m_pVertexBuffer.Get(), m_pInstanceVertexBuffer.Get()};
        m_pContext->OMSetRenderTargets(1u, m_pRTV.GetAddressOf(), nullptr);
        m_pContext->VSSetShader(m_pVertexShader.Get(), nullptr, 0);
        m_pContext->PSSetShader(m_pPixelShader.Get(), nullptr, 0);
        m_pContext->IASetVertexBuffers(0u, 2u, buffers, strides, offsets);
        m_pContext->IASetInputLayout(m_pInputLayout.Get());
        
         m_pContext->PSSetSamplers(0u, 1u, m_pSampler.GetAddressOf());
        m_pContext->PSSetShaderResources(0u, 1u, m_pCircleTexView.GetAddressOf());
        m_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
       
    };
    
    void Draw() const noexcept
    {
        static const float RTVClearColor[4]{0.29f, 0.22f, 0.79f, 0.99f};
        m_pContext->ClearRenderTargetView(CRenderer::m_pRTV.Get(), RTVClearColor);

        m_pContext->DrawInstanced(6u, 1u, 0u, 0u);
        
    };

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