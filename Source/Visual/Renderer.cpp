#pragma once
#include "Renderer.hpp"

Renderer::Renderer() noexcept
{
    Instance::SeedStdRand();
    Instance::Index = 0;
};

void Renderer::SetPipeLine() const noexcept
{
    UINT strides[]{sizeof(Vertex), sizeof(Instance)};
    UINT offsets[]{0, 0};

    ID3D11Buffer *buffers[2]{nullptr,                        // vertex buffer
                             m_pInstanceVertexBuffer.Get()}; // instance buffer
    ID3D11Buffer *constBuffers[3]{
        m_pViewPortSizeBuffer.Get(),
        m_pFrameBuffer.Get(),
    };

    m_pContext->IASetInputLayout(m_pInputLayout.Get());
    m_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_pContext->IASetVertexBuffers(0u, _countof(buffers), buffers, strides, offsets);
    m_pContext->OMSetBlendState(m_pBlend.Get(), nullptr, 0xFFFF'FFFF);

    m_pContext->VSSetShader(m_pVertexShader.Get(), nullptr, 0);
    m_pContext->VSSetConstantBuffers(0, _countof(constBuffers), constBuffers);
    m_pContext->PSSetShader(m_pPixelShader.Get(), nullptr, 0);
    m_pContext->PSSetSamplers(0u, 1u, m_pSampler.GetAddressOf());
    m_pContext->PSSetShaderResources(0u, 1u, m_pCircleTexView.GetAddressOf());
    m_pContext->PSSetConstantBuffers(0, _countof(constBuffers), constBuffers);
};

void Renderer::UpdateFrameBuffer() noexcept
{

    FrameBuffer constantBuffer{Timer.Count<long>()};

    m_pContext->UpdateSubresource(
        m_pFrameBuffer.Get(),
        0, nullptr, &constantBuffer, 0, 0);

    if ((m_Instancies[Instance::Index].STARTTIME + m_Instancies[Instance::Index].PERIOD) < constantBuffer.Time.y)
    {
        m_Instancies[Instance::Index].Reset(*this, constantBuffer.Time.y);
    };
};

void Renderer::UpdateViewPortSizeBuffer(float Width, float Height) const noexcept
{

    ViewPortSizeBuffer ViewPortSize{{Width, Height}};
    m_pContext->UpdateSubresource(
        m_pViewPortSizeBuffer.Get(),
        0, nullptr, &ViewPortSize, 0, 0);
};

void Renderer::Draw() const noexcept
{
    static constexpr DirectX::XMFLOAT4 RTVClearColor{0x12 / 256.f, 0x0e / 256.f, 0x11 / 256.f, 0.99f};

    m_pContext->ClearRenderTargetView(Renderer::m_pRTV.Get(), &RTVClearColor.x);

    m_pContext->RSSetViewports(1, &m_ViewPort);
    m_pContext->OMSetRenderTargets(1u, Renderer::m_pRTV.GetAddressOf(), nullptr);
    m_pContext->DrawInstanced(s_DrawVertexCount, s_DrawInstanceCount, 0u, 0u);
};