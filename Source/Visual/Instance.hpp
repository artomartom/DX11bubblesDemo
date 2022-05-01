#ifndef VISUAL_INASTANCE_HPP
#define VISUAL_INASTANCE_HPP
#pragma once

class Renderer;

struct Instance
{
    Instance() = default;

    explicit Instance(float time)
        : Instance() { STARTTIME = time; };

    void Reset(const Renderer &Renderer, float startTime);

    // layout should stictly mirror  vertex shader Instance input layout
    DirectX::XMFLOAT2 TRANSLATION{RandUV()};
    float SIZE{RandSize()};
    float PERIOD{Period()};
    UINT COLOR{RandColor()};
    float STARTTIME{static_cast<float>(std::rand() % 20'000)};

    inline static UINT Index{};

    // static functions form member initialization
    static decltype(SIZE) RandSize() noexcept;
    static float RandPos() noexcept { return {((std::rand() % 1000) / 500.f) - 1.f}; };
    static decltype(TRANSLATION) RandUV() noexcept { return {RandPos(), RandPos()}; };
    static decltype(COLOR) RandColor() noexcept;
    static decltype(PERIOD) Period() noexcept;
    static void SeedStdRand()
    {
        auto time{Timer::GetLocalTime()};
        std::srand(time.wSecond + time.wYear * time.wMinute);
    };
};
#endif // VISUAL_INASTANCE_HPP