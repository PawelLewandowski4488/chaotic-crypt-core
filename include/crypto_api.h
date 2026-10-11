
#ifndef CRYPTO_API_H
#define CRYPTO_API_H

#include <stdint.h>
#include <stddef.h>

#if defined(_WIN32)
#if defined(CRYPTO_BUILD)
#define CRYPTO_API __declspec(dllexport)
#else
#define CRYPTO_API __declspec(dllimport)
#endif
#else
#define CRYPTO_API __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    CRYPTO_SUCCESS = 0,
    CRYPTO_INVALID_ARGUMENT = 1,
    CRYPTO_INVALID_KEY = 2,
    CRYPTO_OUT_OF_MEMORY = 3,
    CRYPTO_INTERNAL_ERROR = 4
} CryptoStatus;

typedef enum
{
    CRYPTO_RGB = 3,
    CRYPTO_RGBA = 4
} CryptoPixelFormat;

CRYPTO_API CryptoStatus crypto_encrypt(
    uint8_t* pixels,
    size_t size,
    uint32_t width,
    uint32_t height,
    CryptoPixelFormat format,
    double* key
    );

CRYPTO_API CryptoStatus crypto_decrypt(
    uint8_t* pixels,
    size_t size,
    uint32_t width,
    uint32_t height,
    CryptoPixelFormat format,
    const double* key
    );

CRYPTO_API uint32_t crypto_get_version();

#ifdef __cplusplus
}
#endif

#endif
