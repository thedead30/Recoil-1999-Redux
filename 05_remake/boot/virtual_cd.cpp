// virtual_cd.cpp - virtual CD-audio drive for the game's music (launcher platform layer; PLATFORM deviation, KG-24).
//
// The original plays its music as Red Book CD audio: zsnd_cd.cpp (SoundCD_OpenDevice 0x004a20d0, SoundCD_Play 0x004a2600,
// SoundCD_PlayTrack 0x004a2750, SoundCD_Stop 0x004a26f0, SoundCD_Shutdown 0x004a24d0, volume 0x004a27f0 / 0x004a2880) talks
// to the "cdaudio" MCI device through the WINMM import slots mciSendCommandA 0x004cc6e8 and auxGetNumDevs / auxGetDevCapsA /
// auxGetVolume / auxSetVolume (0x004cc6dc / 0x004cc6d4 / 0x004cc6e0 / 0x004cc6e4). Without the disc in a drive there is no
// music. When the game folder has music\trackNN.wav (the CD's audio tracks, extracted from the player's own disc image by
// tools/recoil_source.py --music), these slots answer as a drive holding the Recoil disc would - track 1 = data, tracks 2..N =
// audio - and play the WAV files through waveOut. The game code is unchanged: it still picks, starts, loops (MM_MCINOTIFY
// with MCI_NOTIFY_SUCCESSFUL -> SoundCD_OnMciNotify 0x004a26b0) and sets the CD volume exactly as the original.
// Commands answered: MCI_OPEN ("cdaudio"), MCI_SET (time format), MCI_STATUS (media present, ready, mode, number of tracks,
// length, position, track type), MCI_SEEK, MCI_PLAY (from/to tracks, notify), MCI_STOP, MCI_PAUSE, MCI_CLOSE.
#include <windows.h>
#include <mmsystem.h>
#include <atomic>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <string>
#include <thread>

#include "platform/iat_winmm.h"

namespace {
using MciFn = MCIERROR(WINAPI*)(MCIDEVICEID, UINT, DWORD_PTR, DWORD_PTR);
using AuxNumFn = UINT(WINAPI*)();
using AuxCapsFn = MMRESULT(WINAPI*)(UINT_PTR, LPAUXCAPSA, UINT);
using AuxGetVolFn = MMRESULT(WINAPI*)(UINT, LPDWORD);
using AuxSetVolFn = MMRESULT(WINAPI*)(UINT, DWORD);
MciFn g_mci; AuxNumFn g_aux_num; AuxCapsFn g_aux_caps; AuxGetVolFn g_aux_get; AuxSetVolFn g_aux_set;

constexpr MCIDEVICEID kDevice = 0x7ce1;   // PLATFORM: the virtual device's id (any id the real MCI is not using)
constexpr int kWaveBuffers = 4;           // PLATFORM: streaming buffers
constexpr DWORD kWaveBufferBytes = 1 << 16;
int g_last_track = 0;                     // highest music\trackNN.wav present; track 1 is the data track
int g_current = 1;                        // MCI_SEEK position (track)
DWORD g_volume = 0xFFFFFFFFu;             // aux volume, low word left / high word right (full scale)
std::string g_dir;
std::mutex g_lock;
HWAVEOUT g_wave = nullptr;                // the playing waveOut handle (under g_lock)

struct Player { std::thread th; std::atomic<bool> stop{false}; std::atomic<bool> done{false}; };
Player* g_player = nullptr;

void vlog(const char* fmt, ...)   // RECOIL_VCD_LOG=1: one flushed line per command in vcd.log (game folder)
{
    static int on = -1;
    if (on < 0) on = std::getenv("RECOIL_VCD_LOG") != nullptr;
    if (!on) return;
    if (FILE* f = std::fopen((g_dir + "\\vcd.log").c_str(), "a")) {
        va_list a; va_start(a, fmt); std::vfprintf(f, fmt, a); va_end(a); std::fclose(f);
    }
}

std::string track_file(int t)
{
    char b[32]; std::snprintf(b, sizeof b, "\\music\\track%02d.wav", t);
    return g_dir + b;
}

// the PCM data of a WAV file: format check (44.1 kHz, 16-bit, stereo = CD audio) and the data chunk's place
bool open_wav(FILE* f, long* data_at, DWORD* data_len, WAVEFORMATEX* wf)
{
    char riff[12];
    if (std::fread(riff, 1, 12, f) != 12 || std::memcmp(riff, "RIFF", 4) || std::memcmp(riff + 8, "WAVE", 4)) return false;
    bool have_fmt = false;
    for (;;) {
        char id[4]; DWORD len;
        if (std::fread(id, 1, 4, f) != 4 || std::fread(&len, 4, 1, f) != 1) return false;
        if (!std::memcmp(id, "fmt ", 4)) {
            WAVEFORMATEX w{}; std::fread(&w, 1, len < sizeof w ? len : sizeof w, f);
            if (len > sizeof w) std::fseek(f, static_cast<long>(len - sizeof w), SEEK_CUR);
            w.cbSize = 0; *wf = w; have_fmt = w.wFormatTag == WAVE_FORMAT_PCM;
        } else if (!std::memcmp(id, "data", 4)) {
            *data_at = std::ftell(f); *data_len = len; return have_fmt;
        } else {
            std::fseek(f, static_cast<long>(len + (len & 1)), SEEK_CUR);
        }
    }
}

void play_thread(Player* pl, int from, int to, HWND notify)
{
    for (int t = from; t < to && !pl->stop; ++t) {
        FILE* f = std::fopen(track_file(t).c_str(), "rb");
        if (!f) continue;
        long at; DWORD len; WAVEFORMATEX wf;
        if (!open_wav(f, &at, &len, &wf)) { std::fclose(f); continue; }
        HANDLE ev = CreateEventA(nullptr, FALSE, FALSE, nullptr);
        HWAVEOUT wo = nullptr;
        if (waveOutOpen(&wo, WAVE_MAPPER, &wf, reinterpret_cast<DWORD_PTR>(ev), 0, CALLBACK_EVENT) != MMSYSERR_NOERROR) {
            std::fclose(f); CloseHandle(ev); continue;
        }
        { std::lock_guard<std::mutex> g(g_lock); g_wave = wo; waveOutSetVolume(wo, g_volume); }
        static char bufs[kWaveBuffers][kWaveBufferBytes];
        WAVEHDR hdr[kWaveBuffers]{};
        std::fseek(f, at, SEEK_SET);
        DWORD left = len; int queued = 0;
        for (int k = 0; k < kWaveBuffers; ++k) { hdr[k].lpData = bufs[k]; hdr[k].dwFlags = WHDR_DONE; }
        while (!pl->stop) {
            bool any = false;
            for (int k = 0; k < kWaveBuffers; ++k) {
                if (!(hdr[k].dwFlags & WHDR_DONE)) { any = true; continue; }
                if (hdr[k].dwFlags & WHDR_PREPARED) { waveOutUnprepareHeader(wo, &hdr[k], sizeof hdr[k]); --queued; }
                if (left == 0) continue;
                DWORD n = left < kWaveBufferBytes ? left : kWaveBufferBytes;
                n = static_cast<DWORD>(std::fread(bufs[k], 1, n, f)); left = n ? left - n : 0;
                if (!n) continue;
                hdr[k].dwBufferLength = n; hdr[k].dwFlags = 0;
                waveOutPrepareHeader(wo, &hdr[k], sizeof hdr[k]); waveOutWrite(wo, &hdr[k], sizeof hdr[k]);
                ++queued; any = true;
            }
            if (!any && left == 0) break;
            WaitForSingleObject(ev, 200);
        }
        if (pl->stop) waveOutReset(wo);
        for (int k = 0; k < kWaveBuffers; ++k)
            if (hdr[k].dwFlags & WHDR_PREPARED) {
                while (!(hdr[k].dwFlags & WHDR_DONE)) Sleep(5);
                waveOutUnprepareHeader(wo, &hdr[k], sizeof hdr[k]);
            }
        { std::lock_guard<std::mutex> g(g_lock); g_wave = nullptr; }
        waveOutClose(wo); CloseHandle(ev); std::fclose(f);
    }
    pl->done = true;
    vlog("tracks %d..%d %s\n", from, to - 1, pl->stop ? "stopped" : "finished - notify sent");
    if (!pl->stop && notify) PostMessageA(notify, MM_MCINOTIFY, MCI_NOTIFY_SUCCESSFUL, kDevice);   // the track(s) finished
}

void stop_play()
{
    Player* p = g_player; g_player = nullptr;
    if (!p) return;
    p->stop = true;
    if (p->th.joinable()) p->th.join();
    delete p;
}

DWORD track_frames(int t)   // length of a track in CD frames (75 per second)
{
    FILE* f = std::fopen(track_file(t).c_str(), "rb");
    if (!f) return 0;
    long at; DWORD len; WAVEFORMATEX wf; DWORD r = 0;
    if (open_wav(f, &at, &len, &wf) && wf.nAvgBytesPerSec) r = static_cast<DWORD>(static_cast<unsigned long long>(len) * 75 / wf.nAvgBytesPerSec);
    std::fclose(f); return r;
}

MCIERROR WINAPI VirtualMci(MCIDEVICEID id, UINT msg, DWORD_PTR flags, DWORD_PTR param)
{
    if (msg == MCI_OPEN) {
        auto* o = reinterpret_cast<MCI_OPEN_PARMSA*>(param);
        if (o && (flags & MCI_OPEN_TYPE) && !(flags & MCI_OPEN_TYPE_ID) && o->lpstrDeviceType && !_stricmp(o->lpstrDeviceType, "cdaudio")) {
            o->wDeviceID = kDevice; vlog("open cdaudio (%d tracks)\n", g_last_track); return 0;
        }
        return g_mci(id, msg, flags, param);
    }
    if (id != kDevice) return g_mci(id, msg, flags, param);
    switch (msg) {
    case MCI_SET: return 0;   // time format: answers are given in TMSF/MSF either way
    case MCI_STATUS: {
        auto* s = reinterpret_cast<MCI_STATUS_PARMS*>(param);
        switch (s->dwItem) {
        case MCI_STATUS_MEDIA_PRESENT: case MCI_STATUS_READY: s->dwReturn = TRUE; break;
        case MCI_STATUS_NUMBER_OF_TRACKS: s->dwReturn = static_cast<DWORD>(g_last_track); break;
        case MCI_STATUS_MODE: s->dwReturn = (g_player && !g_player->done) ? MCI_MODE_PLAY : MCI_MODE_STOP; break;
        case MCI_STATUS_POSITION: s->dwReturn = MCI_MAKE_TMSF(g_current, 0, 0, 0); break;
        case MCI_STATUS_LENGTH: {
            DWORD fr = 0;
            if (flags & MCI_TRACK) fr = track_frames(static_cast<int>(s->dwTrack));
            else for (int t = 2; t <= g_last_track; ++t) fr += track_frames(t);
            s->dwReturn = MCI_MAKE_MSF(fr / 75 / 60, fr / 75 % 60, fr % 75);
            break;
        }
        case MCI_CDA_STATUS_TYPE_TRACK: s->dwReturn = s->dwTrack <= 1 ? MCI_CDA_TRACK_OTHER : MCI_CDA_TRACK_AUDIO; break;
        default: s->dwReturn = 0; break;
        }
        return 0;
    }
    case MCI_SEEK: {
        auto* s = reinterpret_cast<MCI_SEEK_PARMS*>(param);
        stop_play();
        if (flags & MCI_TO) g_current = MCI_TMSF_TRACK(s->dwTo);
        else if (flags & MCI_SEEK_TO_START) g_current = 1;
        else if (flags & MCI_SEEK_TO_END) g_current = g_last_track;
        return 0;
    }
    case MCI_PLAY: {
        auto* p = reinterpret_cast<MCI_PLAY_PARMS*>(param);
        int from = (flags & MCI_FROM) ? MCI_TMSF_TRACK(p->dwFrom) : g_current;
        int to = (flags & MCI_TO) ? MCI_TMSF_TRACK(p->dwTo) : g_last_track + 1;   // TO = start of that track: exclusive
        if (from < 2) from = 2;                                                   // track 1 is the data track
        HWND notify = (flags & MCI_NOTIFY) ? reinterpret_cast<HWND>(p->dwCallback) : nullptr;
        stop_play();
        g_current = from;
        vlog("play tracks %d..%d%s\n", from, to - 1, notify ? " (notify)" : "");
        auto* pl = new Player; g_player = pl;
        pl->th = std::thread(play_thread, pl, from, to, notify);
        return 0;
    }
    case MCI_STOP: case MCI_PAUSE: case MCI_CLOSE: vlog("stop/close (msg %04x)\n", msg); stop_play(); return 0;
    default: return 0;
    }
}

// aux devices: the virtual CD-audio line is device 0 (the game binds the first AUXCAPS_CDAUDIO line); real ones follow
UINT WINAPI VirtualAuxNum() { return g_aux_num() + 1; }
MMRESULT WINAPI VirtualAuxCaps(UINT_PTR id, LPAUXCAPSA c, UINT cb)
{
    if (id != 0) return g_aux_caps(id - 1, c, cb);
    if (!c || cb < sizeof(AUXCAPSA)) return MMSYSERR_INVALPARAM;
    std::memset(c, 0, sizeof *c);
    std::strcpy(c->szPname, "Recoil virtual CD audio");
    c->wTechnology = AUXCAPS_CDAUDIO; c->dwSupport = AUXCAPS_VOLUME | AUXCAPS_LRVOLUME;
    return MMSYSERR_NOERROR;
}
MMRESULT WINAPI VirtualAuxGet(UINT id, LPDWORD v)
{
    if (id != 0) return g_aux_get(id - 1, v);
    *v = g_volume; return MMSYSERR_NOERROR;
}
MMRESULT WINAPI VirtualAuxSet(UINT id, DWORD v)
{
    if (id != 0) return g_aux_set(id - 1, v);
    std::lock_guard<std::mutex> g(g_lock);
    g_volume = v;
    if (g_wave) waveOutSetVolume(g_wave, v);
    return MMSYSERR_NOERROR;
}
}  // namespace

// Called by the launcher in the game folder before the game starts. Returns the number of music tracks found (0 = the
// real MCI stays in place).
int InstallVirtualCd(const char* game_dir)
{
    g_dir = game_dir;
    for (int t = 2; t <= 99; ++t) {
        if (GetFileAttributesA(track_file(t).c_str()) == INVALID_FILE_ATTRIBUTES) break;
        g_last_track = t;
    }
    if (g_last_track < 2) return 0;
    using recoil::g_Iat_mciSendCommandA_004cc6e8;
    g_mci = reinterpret_cast<MciFn>(recoil::g_Iat_mciSendCommandA_004cc6e8);
    g_aux_num = reinterpret_cast<AuxNumFn>(recoil::g_Iat_auxGetNumDevs_004cc6dc);
    g_aux_caps = reinterpret_cast<AuxCapsFn>(recoil::g_Iat_auxGetDevCapsA_004cc6d4);
    g_aux_get = reinterpret_cast<AuxGetVolFn>(recoil::g_Iat_auxGetVolume_004cc6e0);
    g_aux_set = reinterpret_cast<AuxSetVolFn>(recoil::g_Iat_auxSetVolume_004cc6e4);
    recoil::g_Iat_mciSendCommandA_004cc6e8 = reinterpret_cast<void*>(&VirtualMci);
    recoil::g_Iat_auxGetNumDevs_004cc6dc = reinterpret_cast<void*>(&VirtualAuxNum);
    recoil::g_Iat_auxGetDevCapsA_004cc6d4 = reinterpret_cast<void*>(&VirtualAuxCaps);
    recoil::g_Iat_auxGetVolume_004cc6e0 = reinterpret_cast<void*>(&VirtualAuxGet);
    recoil::g_Iat_auxSetVolume_004cc6e4 = reinterpret_cast<void*>(&VirtualAuxSet);
    return g_last_track - 1;
}
