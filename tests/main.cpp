
#include "crypto_api.h"
#include "analysis.h"

#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdint>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

void print_analysis(const char* label, const ImageAnalysis& analysis)
{
    std::cout << "\n=== " << label << " ===\n";
    std::cout << "Entropy: " << analysis.entropy << " bits\n";
    std::cout << "Pixel correlation: " << analysis.pixel_correlation << '\n';
    std::cout << "Histogram uniformity: " << analysis.histogram_uniformity << '\n';
    std::cout << "Mean: " << analysis.mean << '\n';
    std::cout << "Standard deviation: " << analysis.standard_deviation << '\n';

    for (int c = 0; c < 3; ++c)
    {
        const char* channel = c == 0 ? "R" : c == 1 ? "G" : "B";
        uint64_t total = 0;

        for (int i = 0; i < 256; ++i)
        {
            total += analysis.histogram[c][i];
        }

        std::cout << "Histogram " << channel << ": " << total << " samples\n";
    }
}

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

    ImageAnalysis original_analysis = analyze_image(pixels.data(), pixels.size(), width, height, static_cast<PixelFormat>(channels));
    print_analysis("Original image", original_analysis);

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

    ImageAnalysis encrypted_analysis = analyze_image(pixels.data(), pixels.size(), width, height, static_cast<PixelFormat>(channels));
    print_analysis("Encrypted image", encrypted_analysis);

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

    ImageAnalysis decrypted_analysis = analyze_image(pixels.data(), pixels.size(), width, height, static_cast<PixelFormat>(channels));
    print_analysis("Decrypted image", decrypted_analysis);

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

    std::cout << "\n=== Statistical comparison ===\n";
    std::cout << "Entropy change: " << encrypted_analysis.entropy - original_analysis.entropy << '\n';
    std::cout << "Correlation change: " << encrypted_analysis.pixel_correlation - original_analysis.pixel_correlation << '\n';
    std::cout << "Uniformity change: " << encrypted_analysis.histogram_uniformity - original_analysis.histogram_uniformity << '\n';

    std::cerr << "FAILURE: Decrypted image differs from original\n";
    return 1;
}
