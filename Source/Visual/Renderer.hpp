
#ifndef VISUAL_RENDER_HPP
#define VISUAL_RENDER_HPP
#pragma once
#include "../pch.hpp"

struct Instance
{
    ::DirectX::XMFLOAT2 TRANSLATION{};
    float SIZE{};
    float PERIOD{};
    ::UINT COLOR{};
    float STARTTIME{};
};

struct ViewPortSizeBuffer
{
    DirectX::XMFLOAT2 ViewPortSize{};
};

struct FrameBuffer
{
    explicit FrameBuffer(long long t)
        : Time{t / 20., t, t / 1000, t % 1000} {/* Log<File>::Write(Time.x,Time.y,Time.z,Time.w);*/};

    ::DirectX::XMFLOAT4 Time{};
};

struct Vertex
{
    ::DirectX::XMFLOAT2 POSITION{};
};

class Renderer
{
    friend class DeviceResource;
    friend struct Instance;

    const ::Microsoft::WRL::ComPtr<ID3D11DeviceContext> &GetContext() const { return m_pContext; };
    const ::Microsoft::WRL::ComPtr<ID3D11RenderTargetView> &GetRenderTargetView() const { return m_pRTV; };

protected:
    Renderer() noexcept;
    void SetPipeLine() const noexcept;
    void UpdateDrawData() noexcept;
    void UpdateViewPortSizeBuffer(float Width, float Height) const noexcept;
    void SetViewPort(float Width, float Height) noexcept;
    void Draw() const noexcept;

    std::unique_ptr<DeviceResource> m_pDeviceResource{};

    static constexpr UINT s_DrawVertexCount{6}; // 6 points to draw a quad
    static constexpr UINT s_DrawInstanceCount{1u};
    D3D11_VIEWPORT m_ViewPort{0.f, 0.f, 0.f, 0.f, 0.f, 1.f};

    Time::Timer Timer{};

    ::Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_pContext{};
    ::Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_pRTV{};
    ::Microsoft::WRL::ComPtr<ID3D11Texture2D> m_pRenderTarget{};
    ::Microsoft::WRL::ComPtr<ID3D11BlendState> m_pBlend{};

    ::Microsoft::WRL::ComPtr<ID3D11VertexShader> m_pVertexShader{};
    ::Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pPixelShader{};
    ::Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_pComputeShader{};

    ::Microsoft::WRL::ComPtr<ID3D11Buffer> m_pViewPortSizeBuffer{}; // constant buffer:Changes On resizing
    ::Microsoft::WRL::ComPtr<ID3D11Buffer> m_pFrameBuffer{};        // constant buffer:changes every update

    ::Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_InstBufferSRV{};
    ::Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> m_InstBufferUAV{};

    ::Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pCircleTexView{};
    ::Microsoft::WRL::ComPtr<ID3D11SamplerState> m_pSampler{};
};

#endif // VISUAL_RENDER_HPP