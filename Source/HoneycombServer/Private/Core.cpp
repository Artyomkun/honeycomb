#include "Core.hpp"
#include <array>

namespace Honeycomb::Core
{
    namespace
    {
        constexpr u32 rol32(u32 v, u32 n) noexcept {
            return (v << n) | (v >> (32 - n));
        }
    }

    std::array<u8, 20> sha1(std::span<const u8> data)
    {
        u32 h0 = 0x67452301, h1 = 0xEFCDAB89, h2 = 0x98BADCFE,
            h3 = 0x10325476, h4 = 0xC3D2E1F0;

        size_t msglen = data.size();
        size_t padded_len = ((msglen + 8) / 64 + 1) * 64;
        Vec<u8> msg(padded_len, 0);
        std::copy(data.begin(), data.end(), msg.begin());
        msg[msglen] = 0x80;

        u64 bitlen = static_cast<u64>(msglen) * 8;
        for (int i = 0; i < 8; ++i)
        {
            msg[padded_len - 1 - i] = static_cast<u8>((bitlen >> (i * 8)) & 0xFF);
        }

        u32 w[80];
        for (size_t chunk = 0; chunk < padded_len; chunk += 64)
        {
            for (int i = 0; i < 16; ++i)
            {
                size_t off = chunk + i * 4;
                w[i] = (msg[off] << 24) | (msg[off + 1] << 16) | (msg[off + 2] << 8) | msg[off + 3];
            }
            for (int i = 16; i < 80; ++i)
            {
                w[i] = rol32(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
            }

            u32 a = h0, b = h1, c = h2, d = h3, e = h4;
            for (int i = 0; i < 80; ++i)
            {
                u32 f, k;
                if (i < 20)      { f = (b & c) | ((~b) & d); k = 0x5A827999; }
                else if (i < 40) { f = b ^ c ^ d;            k = 0x6ED9EBA1; }
                else if (i < 60) { f = (b & c) | (b & d) | (c & d); k = 0x8F1BBCDC; }
                else             { f = b ^ c ^ d;            k = 0xCA62C1D6; }

                u32 temp = rol32(a, 5) + f + e + k + w[i];
                e = d; d = c; c = rol32(b, 30); b = a; a = temp;
            }

            h0 += a; h1 += b; h2 += c; h3 += d; h4 += e;
        }

        std::array<u8, 20> digest{};
        digest[0]  = (h0 >> 24) & 0xFF; digest[1]  = (h0 >> 16) & 0xFF;
        digest[2]  = (h0 >> 8) & 0xFF;  digest[3]  = h0 & 0xFF;
        digest[4]  = (h1 >> 24) & 0xFF; digest[5]  = (h1 >> 16) & 0xFF;
        digest[6]  = (h1 >> 8) & 0xFF;  digest[7]  = h1 & 0xFF;
        digest[8]  = (h2 >> 24) & 0xFF; digest[9]  = (h2 >> 16) & 0xFF;
        digest[10] = (h2 >> 8) & 0xFF;  digest[11] = h2 & 0xFF;
        digest[12] = (h3 >> 24) & 0xFF; digest[13] = (h3 >> 16) & 0xFF;
        digest[14] = (h3 >> 8) & 0xFF;  digest[15] = h3 & 0xFF;
        digest[16] = (h4 >> 24) & 0xFF; digest[17] = (h4 >> 16) & 0xFF;
        digest[18] = (h4 >> 8) & 0xFF;  digest[19] = h4 & 0xFF;

        return digest;
    }
}
