
#ifndef WRITER_INL
#define WRITER_INL

namespace Writer
{

    inline void cOut(std::wstring_view text)
    {

        std::wcout << text << L'\n';
    };

    template <typename... Args>
    constexpr std::wstringstream Accumulate(Args const &...args)
    {
        std::wstringstream ss{};
        (void)std::initializer_list<int>{(ss << args << L' ', 0)...};
        return ss;
    };

    inline std::wstring GetSysTime()
    {
        auto time{Timer::GetLocalTime()};

        std::wstring out{L"00:00:00|00.00| "};
        WORD t[]{
            time.wHour, time.wMinute, time.wSecond, time.wDay, time.wMonth};
        int i{};
        for (auto &each : t)
        {
            out[i] += each / 10;
            out[i + 1] += each % 10;
            i += 3;
        };
        return out;
    };

    template <eType T>
    constexpr std::wstring_view prefix()
    {
#define TCASE(in, out) \
    case in:           \
        return out
        switch (T)
        {
            TCASE(eType::None, L"None     |");
            TCASE(eType::Log, L"Log      |");
            TCASE(eType::Warning, L"Warning  |");
            TCASE(eType::Error, L"Error    |");
        default:
            return L"          ";
        }
#undef TCASE
    };

    template <eType MT, eOut O>
    struct Message
    {
        template <typename... Args>
        constexpr static void Write(Args const &...args)
        {
            using Wrapper::HFile;

            std::wstringstream output{Accumulate(prefix<MT>(), GetSysTime(), args...)};

            if constexpr (O == eOut::Console)
            {
                GUI_ONLY(static_assert(0));

                cOut(output.view());
            }
            else if constexpr (O == eOut::File)
            {
                HFile file{HFile::CreateFile(L"Log.txt")};
                file.Write(output.view());
                file.Write(L"\n");
            };
        };
    };
};
#endif