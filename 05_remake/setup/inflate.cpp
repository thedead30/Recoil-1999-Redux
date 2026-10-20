// inflate.cpp - raw DEFLATE decoder (RFC 1951). PLATFORM: the public format; table values are the RFC's.
#include "inflate.h"

namespace setup {
namespace {

struct Bits {
    const std::uint8_t* p;
    std::size_t n, pos = 0;
    std::uint32_t buf = 0;
    int cnt = 0;
    bool bad = false;
    int need(int k)
    {
        while (cnt < k) {
            if (pos >= n) { bad = true; return 0; }
            buf |= std::uint32_t(p[pos++]) << cnt;
            cnt += 8;
        }
        const int v = int(buf & ((1u << k) - 1));
        buf >>= k;
        cnt -= k;
        return v;
    }
};

// canonical Huffman table: count[len], symbols sorted by code
struct Huff {
    short count[16] = {};
    short symbol[320] = {};
};

bool build(Huff& h, const short* lengths, int n)
{
    for (auto& c : h.count) c = 0;
    for (int s = 0; s < n; ++s) h.count[lengths[s]]++;
    if (h.count[0] == n) return true;   // no codes: valid only if never used
    int left = 1;
    for (int len = 1; len < 16; ++len) {
        left <<= 1;
        left -= h.count[len];
        if (left < 0) return false;     // over-subscribed
    }
    short offs[16];
    offs[1] = 0;
    for (int len = 1; len < 15; ++len) offs[len + 1] = short(offs[len] + h.count[len]);
    for (int s = 0; s < n; ++s)
        if (lengths[s]) h.symbol[offs[lengths[s]]++] = short(s);
    return true;
}

int decode(Bits& b, const Huff& h)
{
    int code = 0, first = 0, index = 0;
    for (int len = 1; len < 16; ++len) {
        code |= b.need(1);
        if (b.bad) return -1;
        const int count = h.count[len];
        if (code - count < first) return h.symbol[index + (code - first)];
        index += count;
        first += count;
        first <<= 1;
        code <<= 1;
    }
    return -1;
}

const short kLenBase[29] = {3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258};
const short kLenExtra[29] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0};
const short kDistBase[30] = {1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513, 769, 1025, 1537, 2049, 3073,
                             4097, 6145, 8193, 12289, 16385, 24577};
const short kDistExtra[30] = {0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13};

bool codes(Bits& b, std::vector<std::uint8_t>& out, const Huff& lit, const Huff& dist)
{
    for (;;) {
        int s = decode(b, lit);
        if (s < 0) return false;
        if (s < 256) { out.push_back(std::uint8_t(s)); continue; }
        if (s == 256) return true;
        s -= 257;
        if (s >= 29) return false;
        const int len = kLenBase[s] + b.need(kLenExtra[s]);
        const int ds = decode(b, dist);
        if (ds < 0 || ds >= 30) return false;
        const std::size_t d = std::size_t(kDistBase[ds]) + std::size_t(b.need(kDistExtra[ds]));
        if (b.bad || d > out.size()) return false;
        const std::size_t from = out.size() - d;
        for (int i = 0; i < len; ++i) out.push_back(out[from + i]);
    }
}

}  // namespace

bool Inflate(const std::uint8_t* in, std::size_t in_size, std::vector<std::uint8_t>& out)
{
    Bits b{in, in_size};
    int last;
    do {
        last = b.need(1);
        const int type = b.need(2);
        if (b.bad) return false;
        if (type == 0) {   // stored
            b.buf = 0;
            b.cnt = 0;
            if (b.pos + 4 > b.n) return false;
            const unsigned len = in[b.pos] | in[b.pos + 1] << 8;
            b.pos += 4;
            if (b.pos + len > b.n) return false;
            out.insert(out.end(), in + b.pos, in + b.pos + len);
            b.pos += len;
        } else if (type == 1) {   // fixed codes
            static Huff lit, dist;
            static bool made = false;
            if (!made) {
                short l[288];
                int s = 0;
                for (; s < 144; ++s) l[s] = 8;
                for (; s < 256; ++s) l[s] = 9;
                for (; s < 280; ++s) l[s] = 7;
                for (; s < 288; ++s) l[s] = 8;
                build(lit, l, 288);
                for (s = 0; s < 30; ++s) l[s] = 5;
                build(dist, l, 30);
                made = true;
            }
            if (!codes(b, out, lit, dist)) return false;
        } else if (type == 2) {   // dynamic codes
            static const short order[19] = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
            const int nlen = b.need(5) + 257, ndist = b.need(5) + 1, ncode = b.need(4) + 4;
            if (b.bad || nlen > 286 || ndist > 30) return false;
            short lengths[320] = {};
            for (int i = 0; i < ncode; ++i) lengths[order[i]] = short(b.need(3));
            Huff lencode;
            if (!build(lencode, lengths, 19)) return false;
            int i = 0;
            while (i < nlen + ndist) {
                int sym = decode(b, lencode);
                if (sym < 0) return false;
                if (sym < 16) { lengths[i++] = short(sym); continue; }
                short v = 0;
                int rep;
                if (sym == 16) {
                    if (i == 0) return false;
                    v = lengths[i - 1];
                    rep = 3 + b.need(2);
                } else if (sym == 17) {
                    rep = 3 + b.need(3);
                } else {
                    rep = 11 + b.need(7);
                }
                if (i + rep > nlen + ndist) return false;
                while (rep--) lengths[i++] = v;
            }
            if (lengths[256] == 0) return false;
            Huff lit, dist;
            if (!build(lit, lengths, nlen) || !build(dist, lengths + nlen, ndist)) return false;
            if (!codes(b, out, lit, dist)) return false;
        } else {
            return false;
        }
    } while (!last);
    return !b.bad;
}

}  // namespace setup
