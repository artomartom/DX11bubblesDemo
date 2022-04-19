
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

/**
 *  (sizeof(ColorsBuffer )%16 ==0  )
 *  used by shader as a float3 [6]
 */
struct ColorsBuffer
{
    // static constexpr DirectX::XMFLOAT4 RTVClearColor {0xCF / 256.f, 0x9F / 256.f, 0xAE / 256.f, 0.99f};
    // static constexpr DirectX::XMFLOAT4 RTVClearColor {0xA2 / 256.f, 0x4E / 256.f, 0x97 / 256.f, 0.99f};
    static constexpr DirectX::XMFLOAT4 RTVClearColor{0x12 / 256.f, 0x0e / 256.f, 0x11 / 256.f, 0.99f};

    DirectX::XMFLOAT4 A{0xFB / 256.f, 0xB5 / 256.f, 0xE0 / 256.f, 1.f};
    DirectX::XMFLOAT4 B{0xB9 / 256.f, 0x80 / 256.f, 0xCE / 256.f, 1.f};
    DirectX::XMFLOAT4 C{0x49 / 256.f, 0x0A / 256.f, 0xBA / 256.f, 1.f};
    DirectX::XMFLOAT4 D{0x4B / 256.f, 0x37 / 256.f, 0x8E / 256.f, 1.f};
    DirectX::XMFLOAT4 E{0x2B / 256.f, 0x6D / 256.f, 0xE2 / 256.f, 1.f};
    DirectX::XMFLOAT4 F{0x1C / 256.f, 0x29 / 256.f, 0xB8 / 256.f, 1.f};
};

struct Vertex
{
    DirectX::XMFLOAT2 POSITION{};
};

class Renderer
{
    friend class DeviceResource;

    const ::Microsoft::WRL::ComPtr<ID3D11DeviceContext> &GetContext() const { return m_pContext; };
    const ::Microsoft::WRL::ComPtr<ID3D11RenderTargetView> &GetRenderTargetView() const { return m_pRTV; };

protected:
    Renderer() noexcept;
    void SetPipeLine() const noexcept;
    void UpdateFrameBuffer() noexcept;
    void UpdateViewPortSizeBuffer(float Width, float Height) const noexcept;
    void SetViewPort(float Width, float Height) noexcept;
    void Draw() const noexcept;

    friend struct Instance;
    static constexpr UINT s_DrawVertexCount{6};
    static constexpr UINT s_DrawInstanceCount{105u};
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

    ::Microsoft::WRL::ComPtr<ID3D11Buffer> m_pVertexBuffer{};         // 6 point draw a quad , never changes
    ::Microsoft::WRL::ComPtr<ID3D11Buffer> m_pInstanceVertexBuffer{}; // data for each circle
    ::Microsoft::WRL::ComPtr<ID3D11Buffer> m_pViewPortSizeBuffer{};   // Changes On resizing
    ::Microsoft::WRL::ComPtr<ID3D11Buffer> m_pFrameBuffer{};          // changes every update
    ::Microsoft::WRL::ComPtr<ID3D11Buffer> m_pColorsBuffer{};         // Never Changes

    ::Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pCircleTexView{};
    ::Microsoft::WRL::ComPtr<ID3D11SamplerState> m_pSampler{};
};

#endif // VISUAL_RENDER_HPP