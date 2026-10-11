
#include "crypto_api.h"
#include "crypto.h"

#include <array>
#include <algorithm>
#include <new>

namespace
{
bool valid_image(uint8_t* pixels, size_t size, uint32_t width, uint32_t height, CryptoPixelFormat format)
{
    if(pixels == nullptr || width == 0 || height == 0)
        return false;

    if(format != CRYPTO_RGB && format != CRYPTO_RGBA)
        return false;

    size_t channels = static_cast<size_t>(format);

    if(static_cast<size_t>(width) > SIZE_MAX / channels)
        return false;

    size_t rowSize = static_cast<size_t>(width) * channels;

    if(static_cast<size_t>(height) > SIZE_MAX / rowSize)
        return false;

    return size == rowSize * static_cast<size_t>(height);
}
}

CryptoStatus crypto_encrypt(uint8_t* pixels, size_t size, uint32_t width, uint32_t height, CryptoPixelFormat format, double* key)
{
    if(key == nullptr || !valid_image(pixels, size, width, height, format))
        return CRYPTO_INVALID_ARGUMENT;

    try
    {
        std::array<double, 10> result = encrypt(pixels, size, width, height, static_cast<PixelFormat>(format));

        std::copy(result.begin(), result.end(), key);

        return CRYPTO_SUCCESS;
    }
    catch(const std::bad_alloc&)
    {
        return CRYPTO_OUT_OF_MEMORY;
    }
    catch(...)
    {
        return CRYPTO_INTERNAL_ERROR;
    }
}

CryptoStatus crypto_decrypt(uint8_t* pixels, size_t size, uint32_t width, uint32_t height, CryptoPixelFormat format, const double* key)
{
    if(key == nullptr || !valid_image(pixels, size, width, height, format))
        return CRYPTO_INVALID_ARGUMENT;

    try
    {
        std::array<double, 10> inputKey;

        std::copy(key, key + 10, inputKey.begin());

        decrypt(pixels, size, width, height, static_cast<PixelFormat>(format), inputKey);

        return CRYPTO_SUCCESS;
    }
    catch(const std::bad_alloc&)
    {
        return CRYPTO_OUT_OF_MEMORY;
    }
    catch(...)
    {
        return CRYPTO_INTERNAL_ERROR;
    }
}
