
#ifndef VISUAL_RENDER_HPP
#define VISUAL_RENDER_HPP
#pragma once
#include "../pch.hpp"
#include "Instance.hpp"

class Instance;
struct ViewPortSizeBuffer
{
    DirectX::XMFLOAT2 ViewPortSize{};
};

struct FrameBuffer
{
    explicit FrameBuffer(long long t)
        : Time{t / 20., t, t / 1000, t % 1000} {/* Log<File>::Write(Time.x,Time.y,Time.z,Time.w);*/};

    DirectX::XMFLOAT4 Time{};
};

struct Vertex
{
    DirectX::XMFLOAT2 POSITION{};
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
    void UpdateFrameBuffer() noexcept;
    void UpdateViewPortSizeBuffer(float Width, float Height) const noexcept;
    void SetViewPort(float Width, float Height) noexcept;
    void Draw() const noexcept;

    std::unique_ptr<DeviceResource> m_pDeviceResource{};

    static constexpr UINT s_DrawVertexCount{6}; // 6 points to draw a quad
    static constexpr UINT s_DrawInstanceCount{4u};
    D3D11_VIEWPORT m_ViewPort{0.f, 0.f, 0.f, 0.f, 0.f, 1.f};

    Timer::CTimer Timer{};

    std::array<Instance, s_DrawInstanceCount> m_Instancies{};
    ::Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_pContext{};
    ::Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_pRTV{};
    ::Microsoft::WRL::ComPtr<ID3D11Texture2D> m_pRenderTarget{};
    ::Microsoft::WRL::ComPtr<ID3D11BlendState> m_pBlend{};

    ::Microsoft::WRL::ComPtr<ID3D11VertexShader> m_pVertexShader{};
    ::Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pPixelShader{};

    ::Microsoft::WRL::ComPtr<ID3D11InputLayout> m_pInputLayout{};

    ::Microsoft::WRL::ComPtr<ID3D11Buffer> m_pInstanceVertexBuffer{}; // data for each circle
    ::Microsoft::WRL::ComPtr<ID3D11Buffer> m_pViewPortSizeBuffer{};   // Changes On resizing
    ::Microsoft::WRL::ComPtr<ID3D11Buffer> m_pFrameBuffer{};          // changes every update

    ::Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pCircleTexView{};
    ::Microsoft::WRL::ComPtr<ID3D11SamplerState> m_pSampler{};
};

#endif // VISUAL_RENDER_HPP