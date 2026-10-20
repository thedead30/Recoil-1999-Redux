// Recoil Redux RX-36 - HD videos (user 2026-10-16: "upres the intro video and the other videos").
//
// The game plays its FMVs (zFMV, avi.md) through Video for Windows: AVIStreamOpenFromFileA / AVIStreamReadFormat /
// AVIStreamRead and an installed decompressor (ICLocate / ICSendMessage / ICDecompress / ICClose). Eight of the
// eleven shipped AVIs are Indeo 5 (IV50); Windows 11 has no Indeo 5 decoder (KG-18), so those videos do not play.
// AviPlayer_OpenVideoStream 0x00463ef0 builds its 16-bit frame from the stream format, and AviPlayer_DecodeFrame
// 0x004643a0 reads one sample and calls ICDecompress into it; SeqAvi_ShowFrame 0x004636d0 then stretches the frame to
// the screen (zVideo_BlitImageToBackBufferEx 0x004a7e10). All CONFIRMED-BINARY (ledger).
//
// When redux_video\<name>.mp4 exists beside the game's video\<name>.avi, Redux wraps those imports (the game calls
// them through its import slots, PLATFORM):
//   * AVIStreamOpenFromFileA: remembers which 'vids' stream belongs to a video with an HD replacement;
//   * AVIStreamReadFormat: reports the HD width/height for that stream, so the game allocates an HD frame itself;
//   * AVIStreamRead: notes which frame the game is about to decode (the compressed sample itself is not needed);
//   * ICLocate / ICSendMessage / ICClose: a stand-in decompressor for that stream;
//   * ICDecompress: decodes that frame from the MP4 with Media Foundation (H.264, built into Windows) into the frame,
//     converted to the game's 16-bit format (565 or 555 by the bitfield masks the game passes).
// Sound, timing, looping, skipping and drawing stay the game's own code; the sound still comes from the original AVI
// (PCM, needs no codec). Videos without an MP4 behave exactly as before.
#include <windows.h>
#include <vfw.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>

#include "redux.h"
#include "platform/iat_avifil32.h"
#include "platform/iat_msvfw32.h"

#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <map>
#include <string>

namespace redux {
namespace {

void VLog(const char* fmt, ...)   // stderr is not reliable after start-up: a log file beside the game
{
    static FILE* f = std::fopen("redux_video.log", "w");
    if (!f) return;
    va_list a;
    va_start(a, fmt);
    std::vfprintf(f, fmt, a);
    va_end(a);
    std::fflush(f);
}

using namespace recoil;

struct HdVideo {
    std::string mp4;
    IMFSourceReader* reader = nullptr;
    int w = 0, h = 0;
    double fps = 30.0;
    int next = 0;            // the frame the reader will deliver next
    int want = 0;            // the frame the game asked for (AVIStreamRead)
};

std::map<void*, HdVideo*> g_by_stream;   // PAVISTREAM -> HD video
HdVideo* g_last_format = nullptr;         // the HD video whose format was read last (ICLocate pairs with it)
const DWORD kFakeHicTag = 0x52444856;     // fake decompressor handles are tagged pointers

using OpenFromFileFn = HRESULT(WINAPI*)(PAVISTREAM*, LPCSTR, DWORD, LONG, UINT, CLSID*);
using ReadFormatFn = HRESULT(WINAPI*)(PAVISTREAM, LONG, LPVOID, LONG*);
using StreamReadFn = HRESULT(WINAPI*)(PAVISTREAM, LONG, LONG, LPVOID, LONG, LONG*, LONG*);
using ReleaseFn = ULONG(WINAPI*)(PAVISTREAM);
using LocateFn = HIC(VFWAPI*)(DWORD, DWORD, LPBITMAPINFOHEADER, LPBITMAPINFOHEADER, WORD);
using SendFn = LRESULT(VFWAPI*)(HIC, UINT, DWORD_PTR, DWORD_PTR);
using DecompressFn = DWORD(VFWAPIV*)(HIC, DWORD, LPBITMAPINFOHEADER, LPVOID, LPBITMAPINFOHEADER, LPVOID);
using CloseFn = LRESULT(VFWAPI*)(HIC);

OpenFromFileFn real_open;
ReadFormatFn real_format;
StreamReadFn real_read;
ReleaseFn real_release;
LocateFn real_locate;
SendFn real_send;
DecompressFn real_decompress;
CloseFn real_close;

std::map<HIC, HdVideo*> g_by_hic;

bool OpenReader(HdVideo* v)
{
    if (v->reader) return true;
    static bool com = SUCCEEDED(CoInitializeEx(nullptr, COINIT_MULTITHREADED)) || true;
    static bool mf = SUCCEEDED(MFStartup(MF_VERSION, MFSTARTUP_LITE));
    (void)com;
    if (!mf) VLog("MFStartup failed\n");
    if (!mf) return false;
    IMFAttributes* a = nullptr;
    MFCreateAttributes(&a, 1);
    a->SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, TRUE);   // let MF convert to RGB32
    wchar_t w[MAX_PATH];
    MultiByteToWideChar(CP_ACP, 0, v->mp4.c_str(), -1, w, MAX_PATH);
    HRESULT hr = MFCreateSourceReaderFromURL(w, a, &v->reader);
    a->Release();
    if (FAILED(hr)) { VLog("cannot open %s (0x%08lx)\n", v->mp4.c_str(), hr); return false; }
    v->reader->SetStreamSelection(MF_SOURCE_READER_ALL_STREAMS, FALSE);
    v->reader->SetStreamSelection(MF_SOURCE_READER_FIRST_VIDEO_STREAM, TRUE);
    IMFMediaType* t = nullptr;
    MFCreateMediaType(&t);
    t->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    t->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32);
    hr = v->reader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, nullptr, t);
    t->Release();
    if (FAILED(hr)) { VLog("no RGB32 output for %s\n", v->mp4.c_str()); return false; }
    IMFMediaType* cur = nullptr;
    v->reader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, &cur);
    UINT32 w32 = 0, h32 = 0, num = 30, den = 1;
    MFGetAttributeSize(cur, MF_MT_FRAME_SIZE, &w32, &h32);
    MFGetAttributeRatio(cur, MF_MT_FRAME_RATE, &num, &den);
    cur->Release();
    v->w = static_cast<int>(w32);
    v->h = static_cast<int>(h32);
    v->fps = den ? double(num) / den : 30.0;
    v->next = 0;
    return v->w > 0 && v->h > 0;
}

// probes the MP4's size once (the format call comes before any frame is needed)
bool ProbeSize(HdVideo* v)
{
    return OpenReader(v);
}

// decodes frame `f` into the game's 16-bit top-down frame
bool DecodeFrame(HdVideo* v, int f, std::uint16_t* out, int ow, int oh, bool r565)
{
    if (!OpenReader(v)) return false;
    if (f < v->next || f > v->next + 60) {   // backwards or a long jump: seek (the reader lands on a key frame before)
        PROPVARIANT p;
        PropVariantInit(&p);
        p.vt = VT_I8;
        p.hVal.QuadPart = static_cast<LONGLONG>(f / v->fps * 1e7);
        v->reader->SetCurrentPosition(GUID_NULL, p);
        v->next = -1;   // unknown until the first sample's time stamp
    }
    for (int guard = 0; guard < 400; ++guard) {
        DWORD flags = 0;
        LONGLONG ts = 0;
        IMFSample* s = nullptr;
        if (FAILED(v->reader->ReadSample(MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, nullptr, &flags, &ts, &s))) return false;
        if (flags & MF_SOURCE_READERF_ENDOFSTREAM) { if (s) s->Release(); v->next = 1 << 30; return false; }
        if (!s) continue;
        const int got = static_cast<int>(ts * v->fps / 1e7 + 0.5);
        v->next = got + 1;
        if (got < f) { s->Release(); continue; }
        IMFMediaBuffer* b = nullptr;
        s->ConvertToContiguousBuffer(&b);
        IMF2DBuffer* b2 = nullptr;
        BYTE* scan0 = nullptr;
        LONG pitch = 0;
        BYTE* data = nullptr;
        DWORD len = 0;
        if (b && SUCCEEDED(b->QueryInterface(IID_PPV_ARGS(&b2))) && SUCCEEDED(b2->Lock2D(&scan0, &pitch))) {
        } else if (b && SUCCEEDED(b->Lock(&data, nullptr, &len))) {
            scan0 = data;
            pitch = v->w * 4;
        }
        if (scan0) {
            const int w = ow < v->w ? ow : v->w, h = oh < v->h ? oh : v->h;
            for (int y = 0; y < h; ++y) {
                const std::uint8_t* src = scan0 + static_cast<LONG_PTR>(y) * pitch;
                std::uint16_t* dst = out + static_cast<std::size_t>(y) * ow;
                for (int x = 0; x < w; ++x) {
                    const unsigned bl = src[x * 4], g = src[x * 4 + 1], r = src[x * 4 + 2];
                    dst[x] = static_cast<std::uint16_t>(r565 ? ((r >> 3) << 11) | ((g >> 2) << 5) | (bl >> 3)
                                                             : ((r >> 3) << 10) | ((g >> 3) << 5) | (bl >> 3));
                }
            }
        }
        if (b2) { if (scan0 && !data) b2->Unlock2D(); b2->Release(); }
        if (b) { if (data) b->Unlock(); b->Release(); }
        s->Release();
        return true;
    }
    return false;
}

HRESULT WINAPI HdOpenFromFile(PAVISTREAM* ppavi, LPCSTR file, DWORD type, LONG param, UINT mode, CLSID* handler)
{
    const HRESULT hr = real_open(ppavi, file, type, param, mode, handler);
    VLog("open %s type %08lx hr %08lx\n", file ? file : "?", type, hr);
    if (SUCCEEDED(hr) && type == streamtypeVIDEO && ppavi && *ppavi && file) {
        std::string f = file, base = f.substr(f.find_last_of("\\/") + 1);
        const std::size_t dot = base.find_last_of('.');
        if (dot != std::string::npos) base = base.substr(0, dot);
        const std::string mp4 = "redux_video\\" + base + ".mp4";   // relative to the game folder (working directory)
        if (GetFileAttributesA(mp4.c_str()) != INVALID_FILE_ATTRIBUTES) {
            auto* v = new HdVideo;
            v->mp4 = mp4;
            g_by_stream[*ppavi] = v;
            VLog("%s plays from %s\n", file, mp4.c_str());
        }
    }
    return hr;
}

HRESULT WINAPI HdReadFormat(PAVISTREAM s, LONG pos, LPVOID fmt, LONG* size)
{
    const HRESULT hr = real_format(s, pos, fmt, size);
    auto it = g_by_stream.find(s);
    if (SUCCEEDED(hr) && fmt && it != g_by_stream.end() && size && *size >= static_cast<LONG>(sizeof(BITMAPINFOHEADER)) &&
        ProbeSize(it->second)) {
        auto* bi = static_cast<BITMAPINFOHEADER*>(fmt);
        bi->biWidth = it->second->w;
        bi->biHeight = it->second->h;
        bi->biSizeImage = static_cast<DWORD>(it->second->w * it->second->h * (bi->biBitCount ? bi->biBitCount : 24) / 8);
        g_last_format = it->second;
    }
    return hr;
}

HRESULT WINAPI HdStreamRead(PAVISTREAM s, LONG start, LONG n, LPVOID buf, LONG cb, LONG* bytes, LONG* samples)
{
    auto it = g_by_stream.find(s);
    if (it != g_by_stream.end()) it->second->want = start;
    return real_read(s, start, n, buf, cb, bytes, samples);
}

ULONG WINAPI HdRelease(PAVISTREAM s)
{
    auto it = g_by_stream.find(s);
    if (it != g_by_stream.end()) {
        if (g_last_format == it->second) g_last_format = nullptr;
        for (auto h = g_by_hic.begin(); h != g_by_hic.end();) h = h->second == it->second ? g_by_hic.erase(h) : std::next(h);
        if (it->second->reader) it->second->reader->Release();
        delete it->second;
        g_by_stream.erase(it);
    }
    return real_release(s);
}

HIC VFWAPI HdLocate(DWORD type, DWORD handler, LPBITMAPINFOHEADER in, LPBITMAPINFOHEADER out, WORD mode)
{
    VLog("locate in %ld x %ld last %p\n", in ? in->biWidth : -1, in ? in->biHeight : -1, static_cast<void*>(g_last_format));
    if (g_last_format && in && in->biWidth == g_last_format->w) {
        HIC h = reinterpret_cast<HIC>(static_cast<std::uintptr_t>(kFakeHicTag ^ reinterpret_cast<std::uintptr_t>(g_last_format)));
        g_by_hic[h] = g_last_format;
        g_last_format = nullptr;
        return h;
    }
    return real_locate(type, handler, in, out, mode);
}

LRESULT VFWAPI HdSend(HIC h, UINT msg, DWORD_PTR a, DWORD_PTR b)
{
    if (g_by_hic.count(h)) return ICERR_OK;   // ICM_DECOMPRESS_BEGIN / END
    return real_send(h, msg, a, b);
}

DWORD VFWAPIV HdDecompress(HIC h, DWORD flags, LPBITMAPINFOHEADER in, LPVOID data, LPBITMAPINFOHEADER out, LPVOID bits)
{
    auto it = g_by_hic.find(h);
    if (it == g_by_hic.end()) return real_decompress(h, flags, in, data, out, bits);
    static int n = 0;
    if (n++ < 3) VLog("decompress frame %d out %ld x %ld bpp %d\n", it->second->want, out->biWidth, out->biHeight, out->biBitCount);
    const DWORD* masks = reinterpret_cast<const DWORD*>(out + 1);   // BI_BITFIELDS masks follow the header
    const bool r565 = out->biCompression != BI_BITFIELDS || masks[1] == 0x07E0;
    const int ow = out->biWidth, oh = out->biHeight < 0 ? -out->biHeight : out->biHeight;
    return DecodeFrame(it->second, it->second->want, static_cast<std::uint16_t*>(bits), ow, oh, r565) ? ICERR_OK : ICERR_ERROR;
}

LRESULT VFWAPI HdClose(HIC h)
{
    if (g_by_hic.erase(h)) return ICERR_OK;
    return real_close(h);
}

}  // namespace

bool InitHdVideo()
{
    if (GetFileAttributesA("redux_video") == INVALID_FILE_ATTRIBUTES) return false;
    real_open = reinterpret_cast<OpenFromFileFn>(g_Iat_AVIStreamOpenFromFileA_004cc030);
    real_format = reinterpret_cast<ReadFormatFn>(g_Iat_AVIStreamReadFormat_004cc02c);
    real_read = reinterpret_cast<StreamReadFn>(g_Iat_AVIStreamRead_004cc034);
    real_release = reinterpret_cast<ReleaseFn>(g_Iat_AVIStreamRelease_004cc01c);
    real_locate = reinterpret_cast<LocateFn>(g_Iat_ICLocate_004cc5f0);
    real_send = reinterpret_cast<SendFn>(g_Iat_ICSendMessage_004cc5f4);
    real_decompress = reinterpret_cast<DecompressFn>(g_Iat_ICDecompress_004cc5ec);
    real_close = reinterpret_cast<CloseFn>(g_Iat_ICClose_004cc5f8);
    if (!real_open || !real_format || !real_read || !real_release || !real_locate || !real_send || !real_decompress ||
        !real_close)
        return false;
    g_Iat_AVIStreamOpenFromFileA_004cc030 = reinterpret_cast<void*>(&HdOpenFromFile);
    g_Iat_AVIStreamReadFormat_004cc02c = reinterpret_cast<void*>(&HdReadFormat);
    g_Iat_AVIStreamRead_004cc034 = reinterpret_cast<void*>(&HdStreamRead);
    g_Iat_AVIStreamRelease_004cc01c = reinterpret_cast<void*>(&HdRelease);
    g_Iat_ICLocate_004cc5f0 = reinterpret_cast<void*>(&HdLocate);
    g_Iat_ICSendMessage_004cc5f4 = reinterpret_cast<void*>(&HdSend);
    g_Iat_ICDecompress_004cc5ec = reinterpret_cast<void*>(&HdDecompress);
    g_Iat_ICClose_004cc5f8 = reinterpret_cast<void*>(&HdClose);
    std::fprintf(stderr, "redux: RX-36 HD videos from redux_video\\ (Media Foundation)\n");
    return true;
}

}  // namespace redux
