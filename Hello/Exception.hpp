#ifndef EXCEPTION_HPP
#define EXCEPTION_HPP

#if (!defined(CONSOLE)) && (defined(GUI))
#define GUI_ONLY(expr) expr
#define CONSOLE_ONLY(expr)
#else
#define GUI_ONLY(expr)
#define CONSOLE_ONLY(expr) expr
#endif // (!defined(CONSOLE)) && (defined(GUI))

#define Beep (MessageBeep(5))

#if defined(_DEBUG) || defined(PROFILE)

#define DBG_ONLY(expr) expr
#define RLS_ONLY(expr)

#define H_ERR(hr, Note) (Exception::FormatError(Exception::ToFile(L"" __FILE__), __LINE__, Exception::ToFunc(L"" #hr), hr, Note))
#define H_CHECK(hr, Note) (Exception::Check(Exception::ToFile(L"" __FILE__), __LINE__, Exception::ToFunc(L"" #hr), hr, Note))
#define H_OK(hr) (!(H_CHECK(hr, L"")))
#define H_FAIL(hr) ((H_CHECK(hr, L"")))
#define W32(expr)      \
    ::SetLastError(0); \
    expr;              \
    H_CHECK(HRESULT_FROM_WIN32(::GetLastError()), L"W32")
#define W32_ERR(Error, Note) H_CHECK(HRESULT_FROM_WIN32(Error), Note)
#else
#define W32_ERR(Error, Note)    // todo :do something 
#define H_CHECK(hr, Note) hr
#define W32(expr)  expr         // check winapi err
#define RLS_ONLY(expr)  expr    //    expr is  part of   release build  only ( )
#define DBG_ONLY(expr) 0        //    expr is not a part of   release build
#define H_ERR(hr, Note) hr    //
#define H_OK(hr) (!(hr))        //  formats hr errors (if any) in debug\profile mode  ( macro does not provide any error checking  to caller, so its his responsibility  to do so  )
#define H_FAIL(hr) ((bool)(hr)) //
#endif                          //  defined(_DEBUG) || defined(PROFILE)

namespace Exception
{
    template <typename PF, typename... Args>
    auto Invoke(PF pfunction, Args... args);

    inline HRESULT FormatError(std::wstring_view File, uint32_t uLine, std::wstring_view Func, HRESULT ErrorCode, std::wstring_view Note);
    inline HRESULT Check(std::wstring_view File, uint32_t uLine, std::wstring_view Func, HRESULT ErrorCode, std::wstring_view Note)
    {
        return (ErrorCode == S_OK) ? S_OK : FormatError(File, uLine, Func, ErrorCode, Note);
    };
    constexpr std::wstring_view ToFunc(std::wstring_view FunctionName);
    constexpr std::wstring_view ToFile(std::wstring_view FileName);

};

#endif