
#ifndef WRAPPER_INL
#define WRAPPER_INL

namespace Wrapper
{
    class HFile
    {
    public:
        enum Disposition : DWORD
        {
            Create_Always = CREATE_ALWAYS,
            Create_New = CREATE_NEW,
            Open_Always = OPEN_ALWAYS,
            Open_Existing = OPEN_EXISTING,
            Truncate_Existing = TRUNCATE_EXISTING,
        };
        enum Access : DWORD
        {
            Generic_All = GENERIC_ALL,
            Generic_Execute = GENERIC_EXECUTE,
            Generic_Read = GENERIC_READ,
            Generic_Write = GENERIC_WRITE,
            File_Append_Data = FILE_APPEND_DATA,

        };
        HFile(HANDLE file) : m_file{file} {};
        HFile(const HFile &) = delete;
        HFile &operator=(const HFile &) = delete;
        HFile(HFile &&) = delete;
        HFile &operator=(HFile &&) = delete;
        HANDLE Get() const noexcept { return m_file; };
        bool IsValid() const noexcept { return m_file != 0; };

        void Attach(HANDLE file) noexcept
        {
            if (m_file != file)
            {
                HFile tmp{file};
                swap(tmp, *this);
            };
        };

        virtual ~HFile()
        {
            if (IsValid())
                ::CloseHandle(m_file);
        };

        static HANDLE IvalidHandle() noexcept { return 0; };
        static HANDLE CreateFile(const wchar_t *filename, Access access = File_Append_Data, Disposition disposition = Open_Always) noexcept
        {
            return CreateFile2(filename, access, 0,
                               disposition, 0);
        };

        DWORD Write(const wchar_t *text) const noexcept
        {
            return Write(std::wstring_view{text});
        };

        DWORD Write(std::wstring_view text) const noexcept
        {
            DWORD numbytes{};
            ::WriteFile(m_file, text.data(),
                        text.size() * 2,
                        &numbytes, nullptr);
            return numbytes;
        };

    private:
        HANDLE m_file{};
        friend void swap(HFile &left, HFile &right) noexcept { std::swap(left.m_file, right.m_file); };
    };

};

#endif