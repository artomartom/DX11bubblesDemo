

#ifndef VISUAL_DEBUG_HPP
#define VISUAL_DEBUG_HPP

// d3dDebug->ReportLiveDeviceObjects( D3D11_RLDO_SUMMARY | D3D11_RLDO_DETAIL );
//  SetDebugObjectName( pObject, "texture.jpg" );

class DebugInterface
{
public:
#if defined(_DEBUG) || defined(PROFILE)
    DebugInterface()
        : m_hDxgiDebugDLL{::LoadLibraryExW(L"dxgidebug.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32)} {};
    HRESULT Init(const ::Microsoft::WRL::ComPtr<ID3D11Device> &pDevice) // Device is initialized at this point
    {
        HRESULT hr{};
        // function pointer typedef of to retrieve debugging interface from dll
        typedef HRESULT(__stdcall * DXGIGetDebugInterface)(REFIID, void **);

        if (!m_hDxgiDebugDLL)
            return H_ERR(::GetLastError(), L"While Loading dxgidebug.dll");

        // populate function pointers
        W32(const auto pDXGIGetDebugInterface = reinterpret_cast<DXGIGetDebugInterface>(
                reinterpret_cast<void *>(::GetProcAddress(m_hDxgiDebugDLL, "DXGIGetDebugInterface"))));

        if (H_FAIL(hr = pDXGIGetDebugInterface(__uuidof(IDXGIDebug), &m_pDebug)))
            return hr;

        if (H_FAIL(hr = pDXGIGetDebugInterface(__uuidof(IDXGIInfoQueue), &m_pInfoQueue)))
            return hr;

        if (H_FAIL(hr = pDevice->QueryInterface(__uuidof(ID3D11Debug), &m_pSDKLayerDebug)))
            return hr;

        DXGI_INFO_QUEUE_MESSAGE_ID InitialDenyList[]{294, 3146080};

        for (auto &each : InitialDenyList)
            PushDeniedMessage(each);

        return S_OK;
    };

    void PushDeniedMessage(DXGI_INFO_QUEUE_MESSAGE_ID &ID)
    {
        HRESULT hr{};
        static DXGI_INFO_QUEUE_FILTER filter{};
        filter.DenyList.NumIDs = 1;
        filter.DenyList.pIDList = &ID;
        H_FAIL(hr = m_pInfoQueue->AddStorageFilterEntries(DXGI_DEBUG_ALL, &filter));
    };

    void PullDebugMessage()
    {
        static std::vector<char> pMessageBuffer(1024, '\0');

        while (m_MessagePointer < m_pInfoQueue->GetNumStoredMessagesAllowedByRetrievalFilters(DXGI_DEBUG_ALL))
        {

            unsigned long long messageLength{};
            m_pInfoQueue->GetMessage(DXGI_DEBUG_ALL, m_MessagePointer, NULL, &messageLength);
            pMessageBuffer.reserve(messageLength);

            DXGI_INFO_QUEUE_MESSAGE *pMessage{reinterpret_cast<DXGI_INFO_QUEUE_MESSAGE *>(pMessageBuffer.data())};
            m_pInfoQueue->GetMessage(DXGI_DEBUG_ALL, m_MessagePointer, pMessage, &messageLength);
            m_MessagePointer++;

            void (*pSender)(const char *const &);
            switch (pMessage->Severity)
            {
            case DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION:
                pSender = &Log<File>::Write;
                break;
            case DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR:
                pSender = &Error<File>::Write;
                break;
            case DXGI_INFO_QUEUE_MESSAGE_SEVERITY_WARNING:
                pSender = &Warning<File>::Write;
                break;
            case DXGI_INFO_QUEUE_MESSAGE_SEVERITY_INFO:
                pSender = &Log<File>::Write;
                break;
            case DXGI_INFO_QUEUE_MESSAGE_SEVERITY_MESSAGE:
                pSender = &Log<File>::Write;
                break;
            }

            std::invoke(pSender, pMessage->pDescription);
            PushDeniedMessage(pMessage->ID);
        };
    };

    void Report()
    {
        m_pSDKLayerDebug->ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY | D3D11_RLDO_DETAIL);
        m_pDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
    };

    virtual ~DebugInterface()
    {

        if (m_pInfoQueue != nullptr)
            DBG_ONLY(PullDebugMessage());
        //   if (m_pSDKLayerDebug != nullptr)
        //  m_pSDKLayerDebug->ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY | D3D11_RLDO_DETAIL);
        //   if (m_pDebug != nullptr)
        // m_pDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
        if (m_hDxgiDebugDLL != 0)
            ::FreeLibrary(m_hDxgiDebugDLL);
    };

private:
    ::Microsoft::WRL::ComPtr<ID3D11Debug> m_pSDKLayerDebug{};
    ::Microsoft::WRL::ComPtr<IDXGIDebug> m_pDebug{};
    ::Microsoft::WRL::ComPtr<IDXGIInfoQueue> m_pInfoQueue{};
    UINT64 m_MessagePointer{};
    HMODULE m_hDxgiDebugDLL{nullptr};

#endif
};

#endif