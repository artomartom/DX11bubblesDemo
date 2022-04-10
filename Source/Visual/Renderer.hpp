
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
    DirectX::XMFLOAT3 Violet{0.933333397f, 0.509803951f, 0.933333397f};
    DirectX::XMFLOAT3 OliveDrab{0.419607878f, 0.556862772f, 0.137254909f};
    DirectX::XMFLOAT3 DarkSalmon{0.913725555f, 0.588235319f, 0.478431404f};
    DirectX::XMFLOAT3 PapayaWhip{1.000000000f, 0.937254965f, 0.835294187f};
    DirectX::XMFLOAT3 SaddleBrown{0.545098066f, 0.270588249f, 0.074509807f};
};

struct Vertex
{
    DirectX::XMFLOAT2 POSITION{};
};
struct Instance
{
    Instance() = default;
    Instance(float time)
        : Instance() { STARTTIME = time; };

    DirectX::XMFLOAT2 TRANSLATION{RandUV()};
    float SIZE{RandPos() * 1.2f};
    float PERIOD{Period()};
    UINT COLOR{RandColor()};
    float STARTTIME{};

    static float RandPos() noexcept { return {((std::rand() % 1000) / 500.f) - 1.f}; };
    static decltype(TRANSLATION) RandUV() noexcept { return {RandPos(), RandPos()}; };
    static decltype(COLOR) RandColor() noexcept { return std::rand() % (sizeof(ColorsBuffer) / sizeof(DirectX::XMFLOAT3)); };
    static decltype(PERIOD) Period() noexcept
    {
        int step{500};
        return (std::rand() % step) + (Index++ * step) + 1500.f;
    };

    inline static UINT Index{};

    static void SeedStdRand()
    {
        auto time{Timer::GetLocalTime()};

        std::srand(time.wSecond);
    };
};

class CRenderer
{
    friend class CDeviceResource;

    const ::Microsoft::WRL::ComPtr<ID3D11DeviceContext> &GetContext() const { return m_pContext; };
    const ::Microsoft::WRL::ComPtr<ID3D11RenderTargetView> &GetRenderTargetView() const { return m_pRTV; };

protected:
    CRenderer() { Instance::SeedStdRand(); };
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

    void UpdateFrameBuffer() const noexcept
    {
        static Timer::CTimer Timer{};

        FrameBuffer constantBuffer{Timer.Count<long>()};
        m_pContext->UpdateSubresource(
            m_pFrameBuffer.Get(),
            0, nullptr, &constantBuffer, 0, 0);

        Log<Console>::Write(Instance::Index);

        if ((m_pInstancies[Instance::Index].STARTTIME + m_pInstancies[Instance::Index].PERIOD) < constantBuffer.Time.y)
        {

            m_pInstancies[Instance::Index] = Instance{};
            /**
             *  default  ctr for Instance increments Index internally, so i should do a out-of-bounds check before
             *  using  accessor
             */
            Instance::Index %= m_DrawInstanceCount;
            Log<Console>::Write(m_pInstancies[Instance::Index - 1          ].STARTTIME);

            D3D11_BOX box{Instance::Index, 0u, 0u, Instance::Index + sizeof(Instance), 1u, 1u};

            m_pContext->UpdateSubresource(
                m_pInstanceVertexBuffer.Get(),
                0, &box, &m_pInstancies[Instance::Index], 0, 0);
        };
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

    std::unique_ptr<Instance[]> m_pInstancies{std::make_unique<Instance[]>(m_DrawInstanceCount)};
    static constexpr UINT m_DrawVertexCount{6};
    static constexpr UINT m_DrawInstanceCount{30u};
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

#endif