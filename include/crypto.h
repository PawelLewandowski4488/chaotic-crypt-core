#ifndef CRYPTO_H
#define CRYPTO_H

#include <array>
#include <cstddef>
#include <cstdint>

enum class PixelFormat : uint8_t {
    RGB = 3,
    RGBA = 4
};

std::array<double, 10> encrypt(uint8_t* pixels, size_t size, uint32_t width, uint32_t height, PixelFormat format);

void decrypt(uint8_t* pixels, size_t size, uint32_t width, uint32_t height, PixelFormat format, std::array<double, 10> key);

#endif