
#ifndef WRITER_HPP
#define WRITER_HPP

namespace Writer
{
    enum eOut : uint32_t
    {
        None = 0,
        Console = 1,
        File = (Console + 1),
    };
    enum class eType : uint32_t
    {
        None = 0,
        Log = 1,
        Warning = (Log + 1),
        Error = (Warning + 1),

    };

    inline void fOut(std::wstring_view text);

    inline void cOut(std::wstring_view text);

    template <typename... Args>
    constexpr std::wstringstream Accumulate(Args const &...args);

    inline std::wstring GetSysTime();

    template <eType T>
    constexpr std::wstring_view prefix();

    template <eType T, eOut O>
    class Message;

    template <eOut OT>
    using Log = Message<eType::Log, OT>;

    template <eOut OT>
    using Warning = Message<eType::Warning, OT>;

    template <eOut OT>
    using Error = Message<eType::Error, OT>;
};
#endif