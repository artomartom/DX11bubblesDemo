

#ifndef LOG_INL
#define LOG_INL

inline void fOut(std::wstring_view text)
{
    // static HFile file{HFile::IvalidHandle()};
    HFile file{HFile::CreateFile(L"Log.txt")};

    int tries = 2;
    while (tries)
    {

        if (file.IsValid())
        {
            const std::wstring_view write_text_than_newline[2]{text, std::wstring_view{L"\n"}};
            for (auto &each : write_text_than_newline)
                WriteFile(file.Get(), each.data(), each.size() * 2, 0, 0);
            return;
        }
        else
        {
            file.Attach(HFile::CreateFile(L"Log.txt"));
            --tries;
        };
    };
};

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
    SYSTEMTIME time{};
    ::GetLocalTime(&time);
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

        std::wstringstream output{Accumulate(prefix<MT>(), GetSysTime(), args...)};

        if constexpr (O == eOut::Console)
        {

            cOut(output.view());
        }
        else if constexpr (O == eOut::File)
        {
            fOut(output.view());
        };
    };
};

#endif