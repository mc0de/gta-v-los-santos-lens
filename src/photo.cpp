#include "photo.h"
#include "settings.h"
#include "ui.h"
#include <wincodec.h>
#include <unordered_map>
#include <vector>

namespace {

constexpr int kPhotoW = 512, kPhotoH = 288;

// Turns the scripted camera off and destroys it however TakeCarPhoto returns.
struct CamGuard {
    int cam = 0;
    ~CamGuard() {
        if (!cam) return;
        Invoke(N::SET_CAM_ACTIVE, cam, 0);
        Invoke(N::RENDER_SCRIPT_CAMS, 0, 0, 0, 1, 0);
        Invoke(N::DESTROY_CAM, cam, 0);
    }
};

// Captures the game window's client area, cropped to a centred 16:9 area and scaled to
// kPhotoW x kPhotoH, as top-down BGRA pixels. Fails if the game isn't the foreground window.
bool CaptureWindow(std::vector<BYTE>& pixels) {
    HWND wnd = GetForegroundWindow();
    DWORD pid = 0;
    if (!wnd || !GetWindowThreadProcessId(wnd, &pid) || pid != GetCurrentProcessId()) return false;

    RECT rc;
    if (!GetClientRect(wnd, &rc)) return false;
    POINT origin{ 0, 0 };
    ClientToScreen(wnd, &origin);
    int cw = rc.right - rc.left, ch = rc.bottom - rc.top;
    if (cw < 16 || ch < 9) return false;

    int sw = cw, sh = cw * 9 / 16; // largest centred 16:9 area
    if (sh > ch) { sh = ch; sw = ch * 16 / 9; }
    int sx = origin.x + (cw - sw) / 2, sy = origin.y + (ch - sh) / 2;

    HDC screen = GetDC(nullptr);
    HDC mem = CreateCompatibleDC(screen);
    BITMAPINFO bi{};
    bi.bmiHeader.biSize = sizeof bi.bmiHeader;
    bi.bmiHeader.biWidth = kPhotoW;
    bi.bmiHeader.biHeight = -kPhotoH; // top-down
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    bi.bmiHeader.biCompression = BI_RGB;
    void* bits = nullptr;
    HBITMAP dib = CreateDIBSection(screen, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
    bool ok = dib && bits;
    if (ok) {
        HGDIOBJ old = SelectObject(mem, dib);
        SetStretchBltMode(mem, HALFTONE);
        SetBrushOrgEx(mem, 0, 0, nullptr);
        ok = StretchBlt(mem, 0, 0, kPhotoW, kPhotoH, screen, sx, sy, sw, sh, SRCCOPY | CAPTUREBLT) != 0;
        GdiFlush();
        if (ok) {
            const BYTE* p = static_cast<const BYTE*>(bits);
            pixels.assign(p, p + kPhotoW * kPhotoH * 4);
        }
        SelectObject(mem, old);
    }
    if (dib) DeleteObject(dib);
    DeleteDC(mem);
    ReleaseDC(nullptr, screen);
    return ok;
}

// A capture of a DX12 game can come back black; that's treated as a failure.
bool IsBlack(const std::vector<BYTE>& px) {
    unsigned long long sum = 0;
    for (size_t i = 0; i + 3 < px.size(); i += 4) sum += px[i] + px[i + 1] + px[i + 2];
    return sum / (px.size() / 4) < 8;
}

bool WritePng(const std::string& path, std::vector<BYTE>& px) {
    // COM is initialised just for this call and released again, so the game's thread is left
    // as we found it. RPC_E_CHANGED_MODE means it's already initialised another way: use that.
    HRESULT com = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(com) && com != RPC_E_CHANGED_MODE) return false;
    for (size_t i = 3; i < px.size(); i += 4) px[i] = 255; // GDI leaves alpha undefined

    // The path is ANSI (GetModuleFileNameA); widen it with the same code page.
    std::wstring wpath(MultiByteToWideChar(CP_ACP, 0, path.c_str(), -1, nullptr, 0), L'\0');
    if (!wpath.empty()) MultiByteToWideChar(CP_ACP, 0, path.c_str(), -1, &wpath[0], static_cast<int>(wpath.size()));
    IWICImagingFactory* factory = nullptr;
    IWICStream* stream = nullptr;
    IWICBitmapEncoder* encoder = nullptr;
    IWICBitmapFrameEncode* frame = nullptr;
    WICPixelFormatGUID format = GUID_WICPixelFormat32bppBGRA;
    bool ok = SUCCEEDED(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
                                         IID_PPV_ARGS(&factory))) &&
              SUCCEEDED(factory->CreateStream(&stream)) &&
              SUCCEEDED(stream->InitializeFromFilename(wpath.c_str(), GENERIC_WRITE)) &&
              SUCCEEDED(factory->CreateEncoder(GUID_ContainerFormatPng, nullptr, &encoder)) &&
              SUCCEEDED(encoder->Initialize(stream, WICBitmapEncoderNoCache)) &&
              SUCCEEDED(encoder->CreateNewFrame(&frame, nullptr)) &&
              SUCCEEDED(frame->Initialize(nullptr)) &&
              SUCCEEDED(frame->SetSize(kPhotoW, kPhotoH)) &&
              SUCCEEDED(frame->SetPixelFormat(&format)) &&
              IsEqualGUID(format, GUID_WICPixelFormat32bppBGRA) &&
              SUCCEEDED(frame->WritePixels(kPhotoH, kPhotoW * 4, static_cast<UINT>(px.size()), px.data())) &&
              SUCCEEDED(frame->Commit()) &&
              SUCCEEDED(encoder->Commit());
    if (frame) frame->Release();
    if (encoder) encoder->Release();
    if (stream) stream->Release();
    if (factory) factory->Release();
    if (SUCCEEDED(com)) CoUninitialize();
    if (!ok) DeleteFileA(path.c_str());
    return ok;
}

// Texture id per photo file name, or -1 if it failed to load.
std::unordered_map<std::string, int> g_textures;

} // namespace

void PhotoResetTextures() { g_textures.clear(); }

std::string PhotoDir() {
    std::string dir = IniPath();
    size_t slash = dir.find_last_of("\\/");
    dir = (slash == std::string::npos ? std::string() : dir.substr(0, slash + 1)) + "SPESP_garage\\";
    CreateDirectoryA(dir.c_str(), nullptr);
    return dir;
}

bool TakeCarPhoto(Vehicle v, const std::string& file) {
    if (!Invoke<int>(N::DOES_ENTITY_EXIST, v)) return false;

    // Frame the car from its front-left at a distance set by its length.
    Vec3 mn{}, mx{};
    Invoke(N::GET_MODEL_DIMENSIONS, Invoke<Hash>(N::GET_ENTITY_MODEL, v), &mn, &mx);
    float len = mx.y - mn.y;
    if (len < 1.0f) len = 1.0f;
    Vec3 pos = Invoke<Vec3>(N::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS, v, -0.8f * len, 1.0f * len, 0.35f * len);

    CamGuard guard;
    guard.cam = Invoke<int>(N::CREATE_CAM, "DEFAULT_SCRIPTED_CAMERA", 1);
    if (!guard.cam) return false;
    Invoke(N::SET_CAM_COORD, guard.cam, pos.x, pos.y, pos.z);
    Invoke(N::POINT_CAM_AT_ENTITY, guard.cam, v, 0.0f, 0.0f, 0.0f, 1);
    Invoke(N::SET_CAM_FOV, guard.cam, 40.0f);
    Invoke(N::SET_CAM_ACTIVE, guard.cam, 1);
    Invoke(N::RENDER_SCRIPT_CAMS, 1, 0, 0, 1, 0);

    // A few frames so the camera cut and streaming settle; nothing of ours draws meanwhile.
    for (int i = 0; i < 4; ++i) {
        Invoke(N::HIDE_HUD_AND_RADAR_THIS_FRAME);
        scriptWait(0);
    }
    Invoke(N::HIDE_HUD_AND_RADAR_THIS_FRAME);

    std::vector<BYTE> px;
    if (!CaptureWindow(px) || IsBlack(px)) return false;
    return WritePng(PhotoDir() + file, px);
}

bool DrawPhoto(const std::string& file, float x, float y, float w, float h) {
    if (file.empty()) return false;
    auto it = g_textures.find(file);
    if (it == g_textures.end()) {
        std::string path = PhotoDir() + file;
        if (GetFileAttributesA(path.c_str()) == INVALID_FILE_ATTRIBUTES) return false; // retry later
        it = g_textures.emplace(file, createTexture(path.c_str())).first;
    }
    if (it->second < 0) return false;
    // sizeY is measured in screen widths here (seen in-game), so convert from a screen-height fraction.
    const float aspect = AspectRatio();
    drawTexture(it->second, 0, 0, 100, w, h / aspect, 0.5f, 0.5f, x, y, 0.0f, aspect, 1.0f, 1.0f, 1.0f, 1.0f);
    return true;
}
