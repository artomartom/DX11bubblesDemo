#pragma once
#include "Instance.hpp"
#include "Renderer.hpp"

//  UINT Instance::Index{};

decltype(Instance::PERIOD) Instance::Period() noexcept
{
    int step{40'000 / Renderer::s_DrawInstanceCount};

    decltype(Instance::PERIOD) res{(std::rand() % step) + (Instance::Index * step) + 3500.f};

    return res;
};
decltype(Instance::SIZE) Instance::RandSize() noexcept
{
    return {((std::rand() % 600 * (Index / Renderer::s_DrawInstanceCount) + 400) / 1300.f)};
};

void Instance::Reset(const Renderer &Renderer, float startTime)
{
    *this = Instance{startTime};

    UINT offset{sizeof(Instance) * Index};
    D3D11_BOX box{offset, 0u, 0u, offset + sizeof(Instance), 1u, 1u};
    Renderer.m_pContext->UpdateSubresource(
        Renderer.m_pInstanceVertexBuffer.Get(),
        0, &box, &Renderer.m_Instancies[Index], 0, 0);

    Index = (Index + 1) % Renderer::s_DrawInstanceCount;
};

decltype(Instance::COLOR) Instance::RandColor() noexcept { return std::rand() % 6; }; // 6 is element count of  color array in vertex shader