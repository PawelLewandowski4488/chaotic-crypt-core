#include "crypto.h"
#include "map.h"
#include "SHA256.h"

#include <algorithm>
#include <cmath>
#include <random>
#include <stdexcept>
#include <vector>

namespace
{
uint64_t extract_nibbles(const uint8_t* H, size_t startNibble, size_t nibbleCount) {
    uint64_t result = 0;
    for (size_t i = 0; i < nibbleCount; ++i) {
        size_t currentNibble = startNibble + i;
        size_t byteIndex = currentNibble / 2;

        uint8_t nibble;
        if (currentNibble % 2 == 0) {
            nibble = (H[byteIndex] >> 4) & 0x0F;
        } else {
            nibble = H[byteIndex] & 0x0F;
        }

        result = (result << 4) | nibble;
    }
    return result;
}

uint8_t PRNG(double x)
{
    return std::fmod(1e10*x, 256);
}

std::array<double, 8> generatekey()
{
    std::random_device rd;

    std::array<double, 8> key = {};


    for (double& x : key) {
        std::uint64_t r = (std::uint64_t(rd()) << 32) ^ rd();

        x = double(r >> 11) / 9007199254740992.0;
    }

    return key;
}
}

std::array<double, 10> encrypt(uint8_t* pixels, size_t size, uint32_t width, uint32_t height, PixelFormat format)
{
    const auto channels = static_cast<uint8_t>(format);
    if (pixels == nullptr || width == 0 || height == 0 || (channels != 3 && channels != 4) || uint64_t(width) * height * channels != size)
    {
        throw std::invalid_argument("Invalid image");
    }

    uint8_t H[32];

    sha256(pixels, size, H);

    uint64_t h1 = extract_nibbles(H,  0, 13);
    uint64_t h2 = extract_nibbles(H, 13, 13);
    uint64_t h3 = extract_nibbles(H, 26, 13);
    uint64_t h4 = extract_nibbles(H, 39, 13);
    uint64_t h5 = extract_nibbles(H, 52, 12);

    std::array<double, 10> key = {};

    key[0] = static_cast<double>(h1+h2+h5)/1e16;
    key[5] = static_cast<double>(h3+h4+h5)/1e16;

    std::random_device rd;

    for (double& x : key) {
        std::uint64_t r = (std::uint64_t(rd()) << 32) ^ rd();

        x = double(r >> 11) / 9007199254740992.0;
    }

    Map mapx(key[0], key[1], key[2], 1, key[3], key[4], 1, 1);
    Map mapy(key[5], key[6], key[7], 1, key[8], key[9], 1, 1);

    for (int i = 0; i<48; i++)
    {
        mapx.next();
        mapy.next();
    }

    double x49 = mapx.next();
    double x50 = mapx.next();
    double y49 = mapy.next();
    double y50 = mapy.next();

    mapx.x = std::fmod(x50 + y49, 1);
    mapy.x = std::fmod(x49 + y50, 1);

    double tab[8];

    for (int i = 0; i<8; i++)
    {
        tab[i] = mapx.next();
    }

    std::vector<int> ind(8);
    for (int i = 0; i < 8; i++) {
        ind[i] = i;
    }

    std::sort(ind.begin(), ind.end(), [&tab](int i1, int i2){return tab[i1] < tab[i2];});

    size_t M = height;
    size_t N = static_cast<size_t>(width) * channels;

    size_t cols = 8*N;

    std::vector<uint8_t> A_planes(M*cols, 0);

    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            uint8_t pixel_byte = pixels[i * N + j];
            for (int k = 0; k < 8; k++) {
                int source_bit = ind[k];
                uint8_t bit_val = (pixel_byte >> source_bit) & 1;
                int target_col = k * N + j;
                A_planes[i * cols + target_col] = bit_val;
            }
        }
    }

    std::vector<int> r(M);
    for (int i = 0; i < M; i++) {
        double xi = mapx.next();
        r[i] = static_cast<int>(std::floor(8 * N * xi));
    }

    double xi_shift = mapx.next();
    int r_shift = static_cast<int>(std::floor(M * xi_shift));

    r_shift = (r_shift % M + M) % M;

    if (r_shift > 0) {
        std::rotate(r.rbegin(), r.rbegin() + r_shift, r.rend());
    }

    for (int i = 0; i < M; i++) {
        int shift_val = r[i];

        shift_val = (shift_val % cols + cols) % cols;

        if (shift_val > 0) {
            auto row_start = A_planes.begin() + i * cols;
            auto row_end = row_start + cols;
            std::rotate(row_start, row_end - shift_val, row_end);
        }
    }

    std::vector<int> c(cols);
    for (int i = 0; i < cols; i++) {
        double xi = mapx.next();
        c[i] = static_cast<int>(std::floor(M * xi));
    }

    double xi_shift_c = mapx.next();
    int c_shift = static_cast<int>(std::floor(cols * xi_shift_c));

    c_shift = (c_shift % cols + cols) % cols;

    if (c_shift > 0) {
        std::rotate(c.rbegin(), c.rbegin() + c_shift, c.rend());
    }

    std::vector<uint8_t> temp_col(M);

    for (int j = 0; j < cols; j++) {
        int shift_val = c[j];
        shift_val = (shift_val % M + M) % M;

        if (shift_val > 0) {
            for (int i = 0; i < M; i++) {
                temp_col[i] = A_planes[i * cols + j];
            }
            std::rotate(temp_col.rbegin(), temp_col.rbegin() + shift_val, temp_col.rend());
            for (int i = 0; i < M; i++) {
                A_planes[i * cols + j] = temp_col[i];
            }
        }
    }

    std::vector<double> tab_y(8);
    for (int i = 0; i < 8; i++) {
        tab_y[i] = mapy.next();
    }

    std::vector<int> pln(8);
    for (int i = 0; i < 8; i++) {
        pln[i] = i;
    }

    std::sort(pln.begin(), pln.end(), [&tab_y](int i1, int i2) {
        return tab_y[i1] < tab_y[i2];
    });

    std::vector<uint8_t> A_shuffled(M * N, 0);

    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            uint8_t pixel_val = 0;
            for (int k = 0; k < 8; k++) {
                int block_index = pln[k];
                uint8_t bit_val = A_planes[i * cols + block_index * N + j];
                pixel_val |= (bit_val << k);
            }
            A_shuffled[i * N + j] = pixel_val;
        }
    }

    std::vector<uint8_t> stream(M * N);
    for (int i = 0; i < M * N; i++) {
        double yi = mapy.next();
        stream[i] = PRNG(yi);
    }

    double yi_shift = mapy.next();
    int s_shift = static_cast<int>(std::floor(M * N * yi_shift));
    s_shift = (s_shift % (M * N) + (M * N)) % (M * N);

    if (s_shift > 0) {
        std::rotate(stream.rbegin(), stream.rbegin() + s_shift, stream.rend());
    }

    for (int i = 0; i < M * N; i++) {
        pixels[i] = stream[i] ^ A_shuffled[i];
    }

    return key;
}

void decrypt(uint8_t* pixels, size_t size, uint32_t width, uint32_t height, PixelFormat format, std::array<double, 10> key)
{
    const auto channels = static_cast<uint8_t>(format);
    if (pixels == nullptr || width == 0 || height == 0 || (channels != 3 && channels != 4) || uint64_t(width) * height * channels != size)
    {
        throw std::invalid_argument("Invalid image");
    }

    Map mapx(key[0], key[1], key[2], 1, key[3], key[4], 1, 1);
    Map mapy(key[5], key[6], key[7], 1, key[8], key[9], 1, 1);

    for (int i = 0; i<48; i++)
    {
        mapx.next();
        mapy.next();
    }

    double x49 = mapx.next();
    double x50 = mapx.next();
    double y49 = mapy.next();
    double y50 = mapy.next();

    mapx.x = std::fmod(x50 + y49, 1);
    mapy.x = std::fmod(x49 + y50, 1);

    size_t M = height;
    size_t N = static_cast<size_t>(width) * channels;

    size_t cols = 8*N;

    const size_t x_size = 8 + M + 1 + cols + 1;
    const size_t y_size = 8 + M * N + 1;

    std::vector<double> xs;
    std::vector<double> ys;

    xs.reserve(x_size);
    ys.reserve(y_size);

    for (size_t i = 0; i < x_size; i++)
    {
        xs.push_back(mapx.next());
    }

    for (size_t i = 0; i < y_size; i++)
    {
        ys.push_back(mapy.next());
    }

    std::vector<uint8_t> stream(M * N);
    for (int i = 0; i < M * N; i++) {
        stream[i] = PRNG(ys[i+8]);
    }
    double yi_shift = ys[8 + M * N];

    size_t s_shift = static_cast<size_t>(std::floor(static_cast<double>(M * N) * yi_shift)) % (M * N);

    if (s_shift > 0) {
        std::rotate(stream.rbegin(), stream.rbegin() + s_shift, stream.rend());
    }
    std::vector<uint8_t> A_unshuffled(M * N);

    for (size_t i = 0; i < M * N; i++)
    {
        A_unshuffled[i] = stream[i] ^ pixels[i];
    }

    std::vector<int> pln(8);

    for (int i = 0; i < 8; i++)
    {
        pln[i] = i;
    }

    std::sort(
        pln.begin(),
        pln.end(),
        [&ys](int i1, int i2) {
            return ys[i1] < ys[i2];
        }
        );

    // ==========================================
    // 6. Odwrocenie skladania bitow
    // ==========================================

    std::vector<uint8_t> A_planes(M * cols, 0);

    for (size_t i = 0; i < M; i++)
    {
        for (size_t j = 0; j < N; j++)
        {
            uint8_t pixel_byte =
                A_unshuffled[i * N + j];

            for (int k = 0; k < 8; k++)
            {
                int block_index = pln[k];

                uint8_t bit_val =
                    (pixel_byte >> k) & 1;

                A_planes[
                    i * cols +
                    static_cast<size_t>(block_index) * N +
                    j
                ] = bit_val;
            }
        }
    }

    // ==========================================
    // 7. Odtworzenie przesuniec kolumn
    // ==========================================

    // Kolejnosc xs:
    // [0 .. 7]                 ind
    // [8 .. 8+M-1]             r
    // [8+M]                    r_shift
    // [9+M .. 9+M+cols-1]      c
    // [9+M+cols]               c_shift

    const size_t c_start = 9 + M;

    std::vector<size_t> c(cols);

    for (size_t j = 0; j < cols; j++)
    {
        double xi = xs[c_start + j];

        if (!std::isfinite(xi) || xi < 0.0 || xi >= 1.0) {
            throw std::runtime_error("Invalid mapx value");
        }

        c[j] = static_cast<size_t>(
                   std::floor(static_cast<double>(M) * xi)
                   ) % M;
    }

    double xi_shift_c = xs[c_start + cols];

    if (!std::isfinite(xi_shift_c) ||
        xi_shift_c < 0.0 || xi_shift_c >= 1.0) {
        throw std::runtime_error("Invalid mapx value");
    }

    size_t c_shift = static_cast<size_t>(
                         std::floor(static_cast<double>(cols) * xi_shift_c)
                         ) % cols;

    if (c_shift > 0)
    {
        std::rotate(
            c.rbegin(),
            c.rbegin() + c_shift,
            c.rend()
            );
    }

    // ==========================================
    // 8. Odwrotna rotacja kolumn - w gore
    // ==========================================

    std::vector<uint8_t> temp_col(M);

    for (size_t j = 0; j < cols; j++)
    {
        size_t shift_val = c[j];

        if (shift_val > 0)
        {
            for (size_t i = 0; i < M; i++)
            {
                temp_col[i] = A_planes[i * cols + j];
            }

            std::rotate(
                temp_col.begin(),
                temp_col.begin() + shift_val,
                temp_col.end()
                );

            for (size_t i = 0; i < M; i++)
            {
                A_planes[i * cols + j] = temp_col[i];
            }
        }
    }

    // ==========================================
    // 9. Odtworzenie przesuniec wierszy
    // ==========================================

    const size_t r_start = 8;

    std::vector<size_t> r(M);

    for (size_t i = 0; i < M; i++)
    {
        double xi = xs[r_start + i];

        if (!std::isfinite(xi) || xi < 0.0 || xi >= 1.0) {
            throw std::runtime_error("Invalid mapx value");
        }

        r[i] = static_cast<size_t>(
                   std::floor(static_cast<double>(cols) * xi)
                   ) % cols;
    }

    double xi_shift = xs[r_start + M];

    if (!std::isfinite(xi_shift) ||
        xi_shift < 0.0 || xi_shift >= 1.0) {
        throw std::runtime_error("Invalid mapx value");
    }

    size_t r_shift = static_cast<size_t>(
                         std::floor(static_cast<double>(M) * xi_shift)
                         ) % M;

    if (r_shift > 0)
    {
        std::rotate(
            r.rbegin(),
            r.rbegin() + r_shift,
            r.rend()
            );
    }

    // ==========================================
    // 10. Odwrotna rotacja wierszy - w lewo
    // ==========================================

    for (size_t i = 0; i < M; i++)
    {
        size_t shift_val = r[i];

        if (shift_val > 0)
        {
            auto row_start = A_planes.begin() + i * cols;
            auto row_end = row_start + cols;

            std::rotate(
                row_start,
                row_start + shift_val,
                row_end
                );
        }
    }

    // ==========================================
    // 11. Odtworzenie permutacji ind
    // ==========================================

    std::vector<int> ind(8);

    for (int i = 0; i < 8; i++)
    {
        ind[i] = i;
    }

    std::sort(
        ind.begin(),
        ind.end(),
        [&xs](int i1, int i2) {
            return xs[i1] < xs[i2];
        }
        );

    // ==========================================
    // 12. Zlozenie bitow do oryginalnych bajtow
    // ==========================================

    for (size_t i = 0; i < M; i++)
    {
        for (size_t j = 0; j < N; j++)
        {
            uint8_t pixel_val = 0;

            for (int k = 0; k < 8; k++)
            {
                uint8_t bit_val =
                    A_planes[
                        i * cols +
                        static_cast<size_t>(k) * N +
                        j
                ];

                pixel_val |= static_cast<uint8_t>(
                    bit_val << ind[k]
                    );
            }

            pixels[i * N + j] = pixel_val;
        }
    }
}