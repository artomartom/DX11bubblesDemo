

#include <wincodec.h>
#include <memory>
#include <dxgiformat.h>
#include <assert.h>
#include "../pch.hpp"

using Microsoft::WRL::ComPtr;

std::wstring GetLocation()
{
    /**GetCurrentDirectoryW :
     * return number of characters that are written to the buffer,
     * not including the terminating null character.
     * If the buffer is not large enough,
     * the return value specifies the required size of the buffer, in characters,
     * including the null-terminating character.
     * To determine the required buffer size,we set params to 0.
     */

    W32(auto BufferSize{::GetCurrentDirectoryW(0, 0)});

    if (BufferSize < _countof(L"F:/n"))
    {
        std::tuple<const wchar_t *> t{L"Fatal:Unable to determine current directory"};
        throw t;
    };

    std::wstring Location{};
    Location.reserve(BufferSize);
    ::SetLastError(0);
    W32(auto res{::GetCurrentDirectoryW(Location.capacity(), &Location[0])});

    if (res != (BufferSize - 1) || ::GetLastError())
    {

        std::tuple<const wchar_t *> t{L"Fatal:Unable to determine current directory"};
        throw t;
    }
    else
    {
        return Location;
    };
};

std::vector<byte> ReadData(
    const wchar_t *filename)
{
    static std::wstring CurrentDir{GetLocation()};

    CREATEFILE2_EXTENDED_PARAMETERS extendedParams = {0};
    extendedParams.dwSize = sizeof(CREATEFILE2_EXTENDED_PARAMETERS);
    extendedParams.dwFileAttributes = FILE_ATTRIBUTE_NORMAL;
    extendedParams.dwFileFlags = FILE_FLAG_SEQUENTIAL_SCAN;
    extendedParams.dwSecurityQosFlags = SECURITY_ANONYMOUS;
    extendedParams.lpSecurityAttributes = nullptr;
    extendedParams.hTemplateFile = nullptr;

    W32(HANDLE file{
        ::CreateFile2(
            (CurrentDir + filename).c_str(),
            GENERIC_READ,
            FILE_SHARE_READ,
            OPEN_EXISTING,
            &extendedParams)});

    if (file == INVALID_HANDLE_VALUE)
        H_ERR(CO_E_FAILEDTOCREATEFILE, L"");

    FILE_STANDARD_INFO fileInfo = {0};
    W32(::GetFileInformationByHandleEx(
        file,
        FileStandardInfo,
        &fileInfo,
        sizeof(fileInfo)));

    std::vector<byte> fileData(fileInfo.EndOfFile.LowPart);

    W32(::ReadFile(
        file,
        fileData.data(),
        (DWORD)fileData.size(),
        nullptr,
        nullptr));

    ::CloseHandle(file);
    return fileData;
}

static IWICImagingFactory **pWIC{};

static IWICImagingFactory *_GetWIC()
{
    IWICImagingFactory *s_Factory{};
    
    

    H_FAIL(CoCreateInstance(
        CLSID_WICImagingFactory,
        nullptr,
        CLSCTX_INPROC_SERVER,
        __uuidof(IWICImagingFactory),
        (LPVOID *)&s_Factory));

    return s_Factory;
}

struct WICTranslate
{
    GUID wic;
    DXGI_FORMAT format;
};
struct WICConvert
{
    GUID source;
    GUID target;
};

static WICConvert g_WICConvert[] =
    {
        // Note target GUID in this conversion table must be one of those directly supported formats (above).

        {GUID_WICPixelFormatBlackWhite, GUID_WICPixelFormat8bppGray}, // DXGI_FORMAT_R8_UNORM

        {GUID_WICPixelFormat1bppIndexed, GUID_WICPixelFormat32bppRGBA}, // DXGI_FORMAT_R8G8B8A8_UNORM
        {GUID_WICPixelFormat2bppIndexed, GUID_WICPixelFormat32bppRGBA}, // DXGI_FORMAT_R8G8B8A8_UNORM
        {GUID_WICPixelFormat4bppIndexed, GUID_WICPixelFormat32bppRGBA}, // DXGI_FORMAT_R8G8B8A8_UNORM
        {GUID_WICPixelFormat8bppIndexed, GUID_WICPixelFormat32bppRGBA}, // DXGI_FORMAT_R8G8B8A8_UNORM

        {GUID_WICPixelFormat2bppGray, GUID_WICPixelFormat8bppGray}, // DXGI_FORMAT_R8_UNORM
        {GUID_WICPixelFormat4bppGray, GUID_WICPixelFormat8bppGray}, // DXGI_FORMAT_R8_UNORM

        {GUID_WICPixelFormat16bppGrayFixedPoint, GUID_WICPixelFormat16bppGrayHalf},  // DXGI_FORMAT_R16_FLOAT
        {GUID_WICPixelFormat32bppGrayFixedPoint, GUID_WICPixelFormat32bppGrayFloat}, // DXGI_FORMAT_R32_FLOAT

#ifdef DXGI_1_2_FORMATS

        {GUID_WICPixelFormat16bppBGR555, GUID_WICPixelFormat16bppBGRA5551}, // DXGI_FORMAT_B5G5R5A1_UNORM

#else

        {GUID_WICPixelFormat16bppBGR555, GUID_WICPixelFormat32bppRGBA},   // DXGI_FORMAT_R8G8B8A8_UNORM
        {GUID_WICPixelFormat16bppBGRA5551, GUID_WICPixelFormat32bppRGBA}, // DXGI_FORMAT_R8G8B8A8_UNORM
        {GUID_WICPixelFormat16bppBGR565, GUID_WICPixelFormat32bppRGBA},   // DXGI_FORMAT_R8G8B8A8_UNORM

#endif // DXGI_1_2_FORMATS

        {GUID_WICPixelFormat32bppBGR101010, GUID_WICPixelFormat32bppRGBA1010102}, // DXGI_FORMAT_R10G10B10A2_UNORM

        {GUID_WICPixelFormat24bppBGR, GUID_WICPixelFormat32bppRGBA},   // DXGI_FORMAT_R8G8B8A8_UNORM
        {GUID_WICPixelFormat24bppRGB, GUID_WICPixelFormat32bppRGBA},   // DXGI_FORMAT_R8G8B8A8_UNORM
        {GUID_WICPixelFormat32bppPBGRA, GUID_WICPixelFormat32bppRGBA}, // DXGI_FORMAT_R8G8B8A8_UNORM
        {GUID_WICPixelFormat32bppPRGBA, GUID_WICPixelFormat32bppRGBA}, // DXGI_FORMAT_R8G8B8A8_UNORM

        {GUID_WICPixelFormat48bppRGB, GUID_WICPixelFormat64bppRGBA},   // DXGI_FORMAT_R16G16B16A16_UNORM
        {GUID_WICPixelFormat48bppBGR, GUID_WICPixelFormat64bppRGBA},   // DXGI_FORMAT_R16G16B16A16_UNORM
        {GUID_WICPixelFormat64bppBGRA, GUID_WICPixelFormat64bppRGBA},  // DXGI_FORMAT_R16G16B16A16_UNORM
        {GUID_WICPixelFormat64bppPRGBA, GUID_WICPixelFormat64bppRGBA}, // DXGI_FORMAT_R16G16B16A16_UNORM
        {GUID_WICPixelFormat64bppPBGRA, GUID_WICPixelFormat64bppRGBA}, // DXGI_FORMAT_R16G16B16A16_UNORM

        {GUID_WICPixelFormat48bppRGBFixedPoint, GUID_WICPixelFormat64bppRGBAHalf},  // DXGI_FORMAT_R16G16B16A16_FLOAT
        {GUID_WICPixelFormat48bppBGRFixedPoint, GUID_WICPixelFormat64bppRGBAHalf},  // DXGI_FORMAT_R16G16B16A16_FLOAT
        {GUID_WICPixelFormat64bppRGBAFixedPoint, GUID_WICPixelFormat64bppRGBAHalf}, // DXGI_FORMAT_R16G16B16A16_FLOAT
        {GUID_WICPixelFormat64bppBGRAFixedPoint, GUID_WICPixelFormat64bppRGBAHalf}, // DXGI_FORMAT_R16G16B16A16_FLOAT
        {GUID_WICPixelFormat64bppRGBFixedPoint, GUID_WICPixelFormat64bppRGBAHalf},  // DXGI_FORMAT_R16G16B16A16_FLOAT
        {GUID_WICPixelFormat64bppRGBHalf, GUID_WICPixelFormat64bppRGBAHalf},        // DXGI_FORMAT_R16G16B16A16_FLOAT
        {GUID_WICPixelFormat48bppRGBHalf, GUID_WICPixelFormat64bppRGBAHalf},        // DXGI_FORMAT_R16G16B16A16_FLOAT

        {GUID_WICPixelFormat96bppRGBFixedPoint, GUID_WICPixelFormat128bppRGBAFloat},   // DXGI_FORMAT_R32G32B32A32_FLOAT
        {GUID_WICPixelFormat128bppPRGBAFloat, GUID_WICPixelFormat128bppRGBAFloat},     // DXGI_FORMAT_R32G32B32A32_FLOAT
        {GUID_WICPixelFormat128bppRGBFloat, GUID_WICPixelFormat128bppRGBAFloat},       // DXGI_FORMAT_R32G32B32A32_FLOAT
        {GUID_WICPixelFormat128bppRGBAFixedPoint, GUID_WICPixelFormat128bppRGBAFloat}, // DXGI_FORMAT_R32G32B32A32_FLOAT
        {GUID_WICPixelFormat128bppRGBFixedPoint, GUID_WICPixelFormat128bppRGBAFloat},  // DXGI_FORMAT_R32G32B32A32_FLOAT

        {GUID_WICPixelFormat32bppCMYK, GUID_WICPixelFormat32bppRGBA},      // DXGI_FORMAT_R8G8B8A8_UNORM
        {GUID_WICPixelFormat64bppCMYK, GUID_WICPixelFormat64bppRGBA},      // DXGI_FORMAT_R16G16B16A16_UNORM
        {GUID_WICPixelFormat40bppCMYKAlpha, GUID_WICPixelFormat64bppRGBA}, // DXGI_FORMAT_R16G16B16A16_UNORM
        {GUID_WICPixelFormat80bppCMYKAlpha, GUID_WICPixelFormat64bppRGBA}, // DXGI_FORMAT_R16G16B16A16_UNORM

#if (_WIN32_WINNT >= 0x0602 /*_WIN32_WINNT_WIN8*/)
        {GUID_WICPixelFormat32bppRGB, GUID_WICPixelFormat32bppRGBA},           // DXGI_FORMAT_R8G8B8A8_UNORM
        {GUID_WICPixelFormat64bppRGB, GUID_WICPixelFormat64bppRGBA},           // DXGI_FORMAT_R16G16B16A16_UNORM
        {GUID_WICPixelFormat64bppPRGBAHalf, GUID_WICPixelFormat64bppRGBAHalf}, // DXGI_FORMAT_R16G16B16A16_FLOAT
#endif

        // We don't support n-channel formats
};

static WICTranslate g_WICFormats[] =
    {
        {GUID_WICPixelFormat128bppRGBAFloat, DXGI_FORMAT_R32G32B32A32_FLOAT},

        {GUID_WICPixelFormat64bppRGBAHalf, DXGI_FORMAT_R16G16B16A16_FLOAT},
        {GUID_WICPixelFormat64bppRGBA, DXGI_FORMAT_R16G16B16A16_UNORM},

        {GUID_WICPixelFormat32bppRGBA, DXGI_FORMAT_R8G8B8A8_UNORM},
        {GUID_WICPixelFormat32bppBGRA, DXGI_FORMAT_B8G8R8A8_UNORM}, // DXGI 1.1
        {GUID_WICPixelFormat32bppBGR, DXGI_FORMAT_B8G8R8X8_UNORM},  // DXGI 1.1

        {GUID_WICPixelFormat32bppRGBA1010102XR, DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM}, // DXGI 1.1
        {GUID_WICPixelFormat32bppRGBA1010102, DXGI_FORMAT_R10G10B10A2_UNORM},
        {GUID_WICPixelFormat32bppRGBE, DXGI_FORMAT_R9G9B9E5_SHAREDEXP},

#ifdef DXGI_1_2_FORMATS

        {GUID_WICPixelFormat16bppBGRA5551, DXGI_FORMAT_B5G5R5A1_UNORM},
        {GUID_WICPixelFormat16bppBGR565, DXGI_FORMAT_B5G6R5_UNORM},

#endif // DXGI_1_2_FORMATS

        {GUID_WICPixelFormat32bppGrayFloat, DXGI_FORMAT_R32_FLOAT},
        {GUID_WICPixelFormat16bppGrayHalf, DXGI_FORMAT_R16_FLOAT},
        {GUID_WICPixelFormat16bppGray, DXGI_FORMAT_R16_UNORM},
        {GUID_WICPixelFormat8bppGray, DXGI_FORMAT_R8_UNORM},

        {GUID_WICPixelFormat8bppAlpha, DXGI_FORMAT_A8_UNORM},

#if (_WIN32_WINNT >= 0x0602 /*_WIN32_WINNT_WIN8*/)
        {GUID_WICPixelFormat96bppRGBFloat, DXGI_FORMAT_R32G32B32_FLOAT},
#endif
};

static DXGI_FORMAT _WICToDXGI(const GUID &guid)
{
    for (size_t i = 0; i < _countof(g_WICFormats); ++i)
    {
        if (memcmp(&g_WICFormats[i].wic, &guid, sizeof(GUID)) == 0)
            return g_WICFormats[i].format;
    }

    return DXGI_FORMAT_UNKNOWN;
}

static size_t _WICBitsPerPixel(REFGUID targetGuid)
{

    if (!pWIC)
        return 0;

    ComPtr<IWICComponentInfo> cinfo;
    H_FAIL((*pWIC)->CreateComponentInfo(targetGuid, &cinfo));

    WICComponentType type;
    H_FAIL(cinfo->GetComponentType(&type));

    if (type != WICPixelFormat)
        return 0;

    ComPtr<IWICPixelFormatInfo> pfinfo;
    H_FAIL(cinfo->QueryInterface(__uuidof(IWICPixelFormatInfo), static_cast<void **>(&pfinfo)));

    UINT bpp;
    H_FAIL(pfinfo->GetBitsPerPixel(&bpp));

    return bpp;
}

static void CreateTextureFromWIC(ID3D11Device *d3dDevice,
                                 ID3D11DeviceContext *d3dContext,
                                 IWICBitmapFrameDecode *frame,
                                 ID3D11Resource **texture,
                                 ID3D11ShaderResourceView **textureView)
{
    UINT width, height;

    H_FAIL(frame->GetSize(&width, &height));
    ULONGLONG maxsize{};
    assert(width > 0 && height > 0);

    // This is a bit conservative because the hardware could support larger textures than
    // the Feature Level defined minimums, but doing it this way is much easier and more
    // performant for WIC than the 'fail and retry' model used by DDSTextureLoader

    switch (d3dDevice->GetFeatureLevel())
    {
    case D3D_FEATURE_LEVEL_9_1:
    case D3D_FEATURE_LEVEL_9_2:
        maxsize = 2048 /*D3D_FL9_1_REQ_TEXTURE2D_U_OR_V_DIMENSION*/;
        break;

    case D3D_FEATURE_LEVEL_9_3:
        maxsize = 4096 /*D3D_FL9_3_REQ_TEXTURE2D_U_OR_V_DIMENSION*/;
        break;

    case D3D_FEATURE_LEVEL_10_0:
    case D3D_FEATURE_LEVEL_10_1:
        maxsize = 8192 /*D3D10_REQ_TEXTURE2D_U_OR_V_DIMENSION*/;
        break;

    default:
        maxsize = D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION;
        break;
    }

    assert(maxsize > 0);

    UINT twidth, theight;
    if (width > maxsize || height > maxsize)
    {
        float ar = static_cast<float>(height) / static_cast<float>(width);
        if (width > height)
        {
            twidth = static_cast<UINT>(maxsize);
            theight = static_cast<UINT>(static_cast<float>(maxsize) * ar);
        }
        else
        {
            theight = static_cast<UINT>(maxsize);
            twidth = static_cast<UINT>(static_cast<float>(maxsize) / ar);
        }
        assert(twidth <= maxsize && theight <= maxsize);
    }
    else
    {
        twidth = width;
        theight = height;
    }

    // Determine format
    WICPixelFormatGUID pixelFormat;
    H_FAIL(frame->GetPixelFormat(&pixelFormat));

    WICPixelFormatGUID convertGUID;
    memcpy(&convertGUID, &pixelFormat, sizeof(WICPixelFormatGUID));

    size_t bpp = 0;

    DXGI_FORMAT format = _WICToDXGI(pixelFormat);
    if (format == DXGI_FORMAT_UNKNOWN)
    {
        for (size_t i = 0; i < _countof(g_WICConvert); ++i)
        {
            if (memcmp(&g_WICConvert[i].source, &pixelFormat, sizeof(WICPixelFormatGUID)) == 0)
            {
                memcpy(&convertGUID, &g_WICConvert[i].target, sizeof(WICPixelFormatGUID));

                format = _WICToDXGI(g_WICConvert[i].target);
                assert(format != DXGI_FORMAT_UNKNOWN);
                bpp = _WICBitsPerPixel(convertGUID);
                break;
            }
        }

        if (format == DXGI_FORMAT_UNKNOWN)
            H_ERR(ERROR_NOT_SUPPORTED, L"");
    }
    else
    {
        bpp = _WICBitsPerPixel(pixelFormat);
    }

    if (!bpp)
        H_ERR(E_FAIL, L"");

    // Verify our target format is supported by the current device
    // (handles WDDM 1.0 or WDDM 1.1 device driver cases as well as DirectX 11.0 Runtime without 16bpp format support)
    UINT support = 0;
    H_FAIL(d3dDevice->CheckFormatSupport(format, &support));
    if (!(support & D3D11_FORMAT_SUPPORT_TEXTURE2D))
    {
        // Fallback to RGBA 32-bit format which is supported by all devices
        memcpy(&convertGUID, &GUID_WICPixelFormat32bppRGBA, sizeof(WICPixelFormatGUID));
        format = DXGI_FORMAT_R8G8B8A8_UNORM;
        bpp = 32;
    }

    // Allocate temporary memory for image
    size_t rowPitch = (twidth * bpp + 7) / 8;
    size_t imageSize = rowPitch * theight;

    std::unique_ptr<uint8_t[]> temp(new uint8_t[imageSize]);

    // Load image data
    if (memcmp(&convertGUID, &pixelFormat, sizeof(GUID)) == 0 && twidth == width && theight == height)
    {
        // No format conversion or resize needed
        H_FAIL(frame->CopyPixels(0, static_cast<UINT>(rowPitch), static_cast<UINT>(imageSize), temp.get()));
    }
    else if (twidth != width || theight != height)
    {
        // Resize
        if (!(*pWIC))
            H_ERR(E_NOINTERFACE, L"");

        ComPtr<IWICBitmapScaler> scaler;
        H_FAIL((*pWIC)->CreateBitmapScaler(&scaler));

        H_FAIL(scaler->Initialize(frame, twidth, theight, WICBitmapInterpolationModeFant));

        WICPixelFormatGUID pfScaler;
        H_FAIL(scaler->GetPixelFormat(&pfScaler));

        if (memcmp(&convertGUID, &pfScaler, sizeof(GUID)) == 0)
        {
            // No format conversion needed
            scaler->CopyPixels(0, static_cast<UINT>(rowPitch), static_cast<UINT>(imageSize), temp.get());
        }
        else
        {
            ComPtr<IWICFormatConverter> FC;
            H_FAIL((*pWIC)->CreateFormatConverter(&FC));
            H_FAIL(FC->Initialize(scaler.Get(), convertGUID, WICBitmapDitherTypeErrorDiffusion, 0, 0, WICBitmapPaletteTypeCustom));
            H_FAIL(FC->CopyPixels(0, static_cast<UINT>(rowPitch), static_cast<UINT>(imageSize), temp.get()));
        }
    }
    else
    {
        // Format conversion but no resize
        if (!(*pWIC))
            H_ERR(E_NOINTERFACE, L"");

        ComPtr<IWICFormatConverter> FC;
        H_FAIL((*pWIC)->CreateFormatConverter(&FC));
        H_FAIL(FC->Initialize(frame, convertGUID, WICBitmapDitherTypeErrorDiffusion, 0, 0, WICBitmapPaletteTypeCustom));
        H_FAIL(FC->CopyPixels(0, static_cast<UINT>(rowPitch), static_cast<UINT>(imageSize), temp.get()));
    }

    // See if format is supported for auto-gen mipmaps (varies by feature level)
    bool autogen = false;
    if (d3dContext != 0 && textureView != 0) // Must have context and shader-view to auto generate mipmaps
    {
        UINT fmtSupport = 0;
        H_FAIL(d3dDevice->CheckFormatSupport(format, &fmtSupport));
        if ((fmtSupport & D3D11_FORMAT_SUPPORT_MIP_AUTOGEN))
        {
            //  autogen = true; //no mipmap
        }
    }

    // Create texture
    D3D11_TEXTURE2D_DESC desc;
    desc.Width = twidth;
    desc.Height = theight;
    desc.MipLevels = (autogen) ? 0 : 1;
    desc.ArraySize = 1;
    desc.Format = format;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = (autogen) ? (D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET) : (D3D11_BIND_SHADER_RESOURCE);
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = (autogen) ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0;

    D3D11_SUBRESOURCE_DATA initData;
    initData.pSysMem = temp.get();
    initData.SysMemPitch = static_cast<UINT>(rowPitch);
    initData.SysMemSlicePitch = static_cast<UINT>(imageSize);

    ID3D11Texture2D *tex{};
    H_FAIL(d3dDevice->CreateTexture2D(&desc, (autogen) ? nullptr : &initData, &tex));
    if (tex != nullptr)
    {
        if (textureView != nullptr)
        {
            D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc{};
            SRVDesc.Format = format;
            SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            SRVDesc.Texture2D.MipLevels = (autogen) ? -1 : 1;

            H_FAIL(d3dDevice->CreateShaderResourceView(tex, &SRVDesc, textureView));

            if (autogen)
            {
                assert(d3dContext != 0);
                d3dContext->UpdateSubresource(tex, 0, nullptr, temp.get(), static_cast<UINT>(rowPitch), static_cast<UINT>(imageSize));
                d3dContext->GenerateMips(*textureView);
            }
        }

        if (texture != nullptr)
        {
            *texture = tex;
        }
    }
}

HRESULT CreateWICTextureFromFile(
    const wchar_t *filename,
    ID3D11Resource **ppTexture,
    ID3D11ShaderResourceView **ppTextureView,
    const ComPtr<ID3D11Device> &d3dDevice,
    const ComPtr<ID3D11DeviceContext> &Context )
{
    auto res{CoInitializeEx(0, COINIT_MULTITHREADED)};
    if (res != S_OK && res != S_FALSE)
      return H_CHECK(res, L"CoInitialize failed");

    HRESULT hr{};
    if (!d3dDevice || !(filename))
    {
        return H_CHECK(E_INVALIDARG, L"");
    }

    ComPtr<IWICImagingFactory> wicFactory{_GetWIC()};

    if (!wicFactory)
        return H_CHECK(E_NOINTERFACE, L"");

    ComPtr<ID3D11Resource> pTexture{};
    ComPtr<ID3D11ShaderResourceView> pTextureView{};
    const auto filenameAbs{GetLocation() + filename};

    ComPtr<IWICBitmapDecoder> decoder{};
    if (H_FAIL(hr = wicFactory->CreateDecoderFromFilename(filenameAbs.c_str(), 0, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &decoder)))
        return hr;

    ComPtr<IWICBitmapFrameDecode> frame{};
    if (H_FAIL(hr = decoder->GetFrame(0, &frame)))
        return hr;
    CreateTextureFromWIC(d3dDevice.Get(), Context.Get(), frame.Get(), &pTexture, &pTextureView );

    if (ppTexture != nullptr)
    {
        *ppTexture = pTexture.Detach();
    }
    if (ppTextureView != nullptr)
    {
        *ppTextureView = pTextureView.Detach();
    }
    CoUninitialize();
    return S_OK;
}