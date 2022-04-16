#ifndef EXCEPTION_INL
#define EXCEPTION_INL

namespace Exception
{
    inline void TranslateExceptionCode(DWORD Code)
    {
        std::wstringstream output;
        output
            << L"Exception " << L'\n'
            << L"          "
            << L"Application Exited with Code: "
            << L"0x" << std::hex << Code << L'\n'
            << L"          ";

        HMODULE HNTStatus{GetModuleHandleW(L"NTDLL.DLL")};

        wchar_t *pMessageBuffer{nullptr};

        if (HNTStatus != 0)
        {

            if (::FormatMessageW(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER |
                        FORMAT_MESSAGE_FROM_SYSTEM |
                        FORMAT_MESSAGE_FROM_HMODULE,
                    HNTStatus,
                    Code,
                    MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
                    (LPWSTR)&pMessageBuffer,
                    0,
                    nullptr))
            {

                output << pMessageBuffer;
            }
            else
            {
                output << L"(FORMATTING FAILED)\n";
            }
        }
        else
        {
            output << L"(TRANSLATION FAILED)\n";
        };

        CONSOLE_ONLY(::Writer::Error<Writer::Console>::Write(output.str()));
        GUI_ONLY(::Writer::Error<Writer::File>::Write(output.str()));
    };

    // Get exception record and delegate  exception to handler
    inline int __stdcall Filter(EXCEPTION_POINTERS *Pointers, EXCEPTION_RECORD &Record)
    {
        Record = *(Pointers->ExceptionRecord);
        return EXCEPTION_EXECUTE_HANDLER;
    };

    template <typename PF, typename... Args>
    auto Invoke(PF pfunction, Args... args)
    {
        EXCEPTION_RECORD Record{};
        __try
        {
            typename std::is_function<PF>::value_type res = std::invoke(pfunction, args...);
            if (res != 0)
                H_ERR(res, L"Application invokation Exited with result");
            return res;
        }
        __except (Filter(GetExceptionInformation(), Record))
        {
            TranslateExceptionCode(Record.ExceptionCode);
            return typename std::is_function<PF>::value_type{};
        };
    };

    constexpr std::wstring_view ToFunc(std::wstring_view FunctionName)
    {

        for (size_t index = 0; index != FunctionName.size(); index++)
        {
            if (FunctionName[index] == L'(')
                for (size_t reverse_index = index - 1; reverse_index != 0; --reverse_index)
                {
                    if (
                        (FunctionName[reverse_index] < L'0') ||
                        (FunctionName[reverse_index] > L'9' && FunctionName[reverse_index] < L'A') ||
                        (FunctionName[reverse_index] > L'Z' && FunctionName[reverse_index] < L'a') ||
                        (FunctionName[reverse_index] > L'z') ||
                        FunctionName[reverse_index] == L' ')
                    {
                        return FunctionName.substr(reverse_index + 1, index - reverse_index - 1);
                    }
                };
        }
        return L"";
    };

    constexpr std::wstring_view ToFile(std::wstring_view FileName)
    {
        return FileName.substr(FileName.find_last_of(L'\\') + 1, FileName.size());
    }

    inline HRESULT FormatError(std::wstring_view File, uint32_t uLine, std::wstring_view Func, HRESULT ErrorCode, std::wstring_view Note)
    {

        RLS_ONLY(assert(0));

        std::wstringstream output;

        wchar_t *MsgBuf{nullptr};
        output
            << L"\n"
            << L"Error    [File] " << File << L"\n"
            << L"         [Func] " << Func << L"\n"
            << L"         [Line] " << uLine << L"\n"
            << L"         [Code] " << ErrorCode << L"\n"
            << L"         [Hex ] 0x" << std::hex << ErrorCode << L"\n"
            << L"->";

        if (::FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                             nullptr,
                             ErrorCode,
                             MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
                             (LPWSTR)&MsgBuf,
                             0,
                             nullptr))
        {
            output << MsgBuf;
        }
        else
        {
            output << L"[TRANSLATION FAILED]";
        }

        output
            << Note;

        CONSOLE_ONLY(::Writer::Error<Writer::Console>::Write(output.str()));
        GUI_ONLY(::Writer::Error<Writer::File>::Write(output.str()));

        ::LocalFree(MsgBuf);
        return ErrorCode;
    };
};
#endif