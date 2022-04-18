
#ifndef VISUAL_RENDER_HPP
#define VISUAL_RENDER_HPP

#include "../pch.hpp"
class Renderer;
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
    //static constexpr DirectX::XMFLOAT4 RTVClearColor {0xCF / 256.f, 0x9F / 256.f, 0xAE / 256.f, 0.99f};
    //static constexpr DirectX::XMFLOAT4 RTVClearColor {0xA2 / 256.f, 0x4E / 256.f, 0x97 / 256.f, 0.99f};
    static constexpr DirectX::XMFLOAT4 RTVClearColor {0x12 / 256.f, 0x0e / 256.f, 0x11 / 256.f, 0.99f};

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

struct Instance
{

    Instance() = default;
    explicit Instance(float time)
        : Instance() { STARTTIME = time; };
    DirectX::XMFLOAT2 TRANSLATION{RandUV()};
    float SIZE{RandSize()};
    float PERIOD{Period()};
    UINT COLOR{RandColor()};
    float STARTTIME{static_cast<float>(std::rand() % 20'000)};
    inline void Reset(const Renderer &Renderer, float startTime);

    static inline decltype(SIZE) RandSize() noexcept;
    static float RandPos() noexcept { return {((std::rand() % 1000) / 500.f) - 1.f}; };
    static decltype(TRANSLATION) RandUV() noexcept { return {RandPos(), RandPos()}; };
    static decltype(COLOR) RandColor() noexcept { return std::rand() % (sizeof(ColorsBuffer) / sizeof(ColorsBuffer::A)); };
    static inline decltype(PERIOD) Period() noexcept;
    inline static UINT Index{};

    static void SeedStdRand()
    {
        auto time{Timer::GetLocalTime()};
        std::srand(time.wSecond);
    };
};

class Renderer
{
    friend class DeviceResource;

    const ::Microsoft::WRL::ComPtr<ID3D11DeviceContext> &GetContext() const { return m_pContext; };
    const ::Microsoft::WRL::ComPtr<ID3D11RenderTargetView> &GetRenderTargetView() const { return m_pRTV; };

protected:
    Renderer()
    {
        Instance::SeedStdRand();
        Instance::Index = 0;
    };
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
        m_pContext->OMSetBlendState(m_pBlend.Get(), nullptr, 0xFFFF'FFFF);

        m_pContext->VSSetShader(m_pVertexShader.Get(), nullptr, 0);
        m_pContext->VSSetConstantBuffers(0, _countof(constBuffers), constBuffers);
        m_pContext->PSSetShader(m_pPixelShader.Get(), nullptr, 0);
        m_pContext->PSSetSamplers(0u, 1u, m_pSampler.GetAddressOf());
        m_pContext->PSSetShaderResources(0u, 1u, m_pCircleTexView.GetAddressOf());
        m_pContext->PSSetConstantBuffers(0, _countof(constBuffers), constBuffers);

        m_pContext->OMSetRenderTargets(1u, m_pRTV.GetAddressOf(), nullptr);
    };

    void UpdateFrameBuffer() noexcept
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

    void UpdateViewPortSizeBuffer(float Width, float Height) const noexcept
    {

        ViewPortSizeBuffer ViewPortSize{{Width, Height}};
        m_pContext->UpdateSubresource(
            m_pViewPortSizeBuffer.Get(),
            0, nullptr, &ViewPortSize, 0, 0);
    };

    void SetViewPort(float Width, float Height) noexcept
    {

        m_ViewPort.Width = Width;
        m_ViewPort.Height = Height;
        Renderer::m_pContext->RSSetViewports(1, &m_ViewPort);
    };

    void Draw() const noexcept
    {
        m_pContext->ClearRenderTargetView(Renderer::m_pRTV.Get(), &ColorsBuffer::RTVClearColor.x);

        m_pContext->RSSetViewports(1, &m_ViewPort);
        m_pContext->OMSetRenderTargets(1u, Renderer::m_pRTV.GetAddressOf(), nullptr);
        m_pContext->DrawInstanced(s_DrawVertexCount, s_DrawInstanceCount, 0u, 0u);
    };

    friend struct Instance;
    static constexpr UINT s_DrawVertexCount{6};
    static constexpr UINT s_DrawInstanceCount{105u};
    D3D11_VIEWPORT m_ViewPort{0.f, 0.f, 0.f, 0.f, 0.f, 1.f};

    Timer::CTimer Timer{};

    std::array<Instance, s_DrawInstanceCount> m_Instancies{};
    ::Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_pContext{};
    ::Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_pRTV{};
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

inline decltype(Instance::PERIOD) Instance::Period() noexcept
{
    int step{40'000 / Renderer::s_DrawInstanceCount};

    decltype(Instance::PERIOD) res{(std::rand() % step) + (Instance::Index * step) + 3500.f};

    return res;
};
inline decltype(Instance::SIZE) Instance::RandSize() noexcept
{
    return {((std::rand() % 600 * (Index / Renderer::s_DrawInstanceCount) + 400) / 1300.f)};
};

inline void Instance::Reset(const Renderer &Renderer, float startTime)
{
    *this = Instance{startTime};

    UINT offset{sizeof(Instance) * Index};
    D3D11_BOX box{offset, 0u, 0u, offset + sizeof(Instance), 1u, 1u};
    Renderer.m_pContext->UpdateSubresource(
        Renderer.m_pInstanceVertexBuffer.Get(),
        0, &box, &Renderer.m_Instancies[Index], 0, 0);

    Index = (Index + 1) % Renderer::s_DrawInstanceCount;
};
#endif