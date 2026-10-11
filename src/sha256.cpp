#include "SHA256.h"

namespace
{
uint32_t rotr(uint32_t x, uint32_t y)
{
    return (x >> y) | (x << (32 - y));
}

uint32_t sigma0(uint32_t x)
{
    return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3);
}

uint32_t sigma1(uint32_t x)
{
    return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10);
}

uint32_t Ch(uint32_t e, uint32_t f, uint32_t g)
{
    return (e & f) ^ (~e & g);
}

uint32_t Maj(uint32_t a, uint32_t b, uint32_t c)
{
    return (a & b) ^ (a & c) ^ (b & c);
}

uint32_t Sigma0(uint32_t e)
{
    return rotr(e, 2) ^ rotr(e, 13) ^ rotr(e, 22);
}

uint32_t Sigma1(uint32_t e)
{
    return rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25);
}
}

void sha256(const uint8_t* data, size_t len, uint8_t hash[32])
{
    size_t padded_len = ((len + 1 + 8 + 63) / 64) * 64;
    uint8_t* padded = new uint8_t[padded_len]();
    if (data && len>0)
    {
        for (size_t i = 0; i < len; i++)
        {
            padded[i] = data[i];
        }
    }
    padded[len] = 0x80;
    uint64_t bit_len = static_cast<uint64_t>(len) * 8;
    for (int i = 0; i < 8; ++i)
    {
        padded[padded_len - 1 - i] = static_cast<uint8_t>(bit_len >> (8 * i));
    }

    size_t N = padded_len / 64;

    uint32_t H[8] = {
        0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
        0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
    };

    constexpr uint32_t K[64] = {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
        0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
        0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
        0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
        0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
        0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
    };

    for (size_t i = 0; i < N; i++)
    {
        uint32_t w[64];
        for (int j = 0; j < 16; j++)
        {
            w[j] =
                (static_cast<uint32_t>(padded[i * 64 + j * 4    ]) << 24) |
                (static_cast<uint32_t>(padded[i * 64 + j * 4 + 1]) << 16) |
                (static_cast<uint32_t>(padded[i * 64 + j * 4 + 2]) <<  8) |
                static_cast<uint32_t>(padded[i * 64 + j * 4 + 3]);
        }
        for (int j = 16; j < 64; j++)
        {
            w[j] = sigma0(w[j - 15]) + w[j - 16] + sigma1(w[j - 2]) + w[j - 7];
        }

        uint32_t a = H[0];
        uint32_t b = H[1];
        uint32_t c = H[2];
        uint32_t d = H[3];
        uint32_t e = H[4];
        uint32_t f = H[5];
        uint32_t g = H[6];
        uint32_t h = H[7];

        for (int j = 0; j < 64; j++)
        {
            uint32_t temp1 = h + Sigma1(e) + Ch(e, f, g) + K[j] + w[j];
            uint32_t temp2 = Sigma0(a) + Maj(a, b, c);
            h = g;
            g = f;
            f = e;
            e = d + temp1;
            d = c;
            c = b;
            b = a;
            a = temp1 + temp2;
        }

        H[0] += a;
        H[1] += b;
        H[2] += c;
        H[3] += d;
        H[4] += e;
        H[5] += f;
        H[6] += g;
        H[7] += h;
    }

    for (int i = 0; i < 8; i++)
    {
        hash[i * 4 + 0] = static_cast<uint8_t>(H[i] >> 24);
        hash[i * 4 + 1] = static_cast<uint8_t>(H[i] >> 16);
        hash[i * 4 + 2] = static_cast<uint8_t>(H[i] >> 8);
        hash[i * 4 + 3] = static_cast<uint8_t>(H[i]);
    }

    delete[] padded;
}
