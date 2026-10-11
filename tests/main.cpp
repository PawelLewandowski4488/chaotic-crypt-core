
#include "crypto_api.h"

#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdint>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

int main(int argc, char* argv[])
{
    if(argc < 2)
    {
        std::cerr << "Usage: crypto_test <image.png>\n";
        return 1;
    }

    int width, height, channels;

    unsigned char* loaded = stbi_load(argv[1], &width, &height, &channels, 0);

    if(loaded == nullptr)
    {
        std::cerr << "Failed to load image: " << stbi_failure_reason() << '\n';
        return 1;
    }

    if(channels != 3 && channels != 4)
    {
        std::cerr << "Only RGB and RGBA images are supported\n";
        stbi_image_free(loaded);
        return 1;
    }

    size_t size = static_cast<size_t>(width) * height * channels;

    std::vector<uint8_t> pixels(loaded, loaded + size);
    std::vector<uint8_t> original = pixels;

    stbi_image_free(loaded);

    CryptoPixelFormat format = channels == 3 ? CRYPTO_RGB : CRYPTO_RGBA;

    double key[10];

    CryptoStatus status = crypto_encrypt(
        pixels.data(),
        pixels.size(),
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height),
        format,
        key
        );

    if(status != CRYPTO_SUCCESS)
    {
        std::cerr << "Encryption failed: " << status << '\n';
        return 1;
    }

    std::cout << "Encryption successful\n";

    if(!stbi_write_png("encrypted.png", width, height, channels, pixels.data(), width * channels))
    {
        std::cerr << "Failed to save encrypted image\n";
        return 1;
    }

    status = crypto_decrypt(
        pixels.data(),
        pixels.size(),
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height),
        format,
        key
        );

    if(status != CRYPTO_SUCCESS)
    {
        std::cerr << "Decryption failed: " << status << '\n';
        return 1;
    }

    std::cout << "Decryption successful\n";

    if(!stbi_write_png("decrypted.png", width, height, channels, pixels.data(), width * channels))
    {
        std::cerr << "Failed to save decrypted image\n";
        return 1;
    }

    if(pixels == original)
    {
        std::cout << "SUCCESS: Decrypted image matches original\n";
        return 0;
    }

    std::cerr << "FAILURE: Decrypted image differs from original\n";
    return 1;
}
