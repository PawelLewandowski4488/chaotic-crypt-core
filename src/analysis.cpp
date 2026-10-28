#include "analysis.h"
#include "crypto.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>
#include <stdexcept>

ImageAnalysis analyze_image(const uint8_t* pixels, size_t size, uint32_t width, uint32_t height, PixelFormat format)
{
    if (pixels == nullptr || width == 0 || height == 0)
    {
        throw std::invalid_argument("Invalid image data");
    }

    if (format != PixelFormat::RGB && format != PixelFormat::RGBA)
    {
        throw std::invalid_argument("Unsupported pixel format");
    }

    const size_t channels = static_cast<size_t>(format);

    if (static_cast<size_t>(width) >
        std::numeric_limits<size_t>::max() / height / channels)
    {
        throw std::invalid_argument("Image dimensions overflow");
    }

    const size_t expected_size =
        static_cast<size_t>(width) * height * channels;

    if (size != expected_size)
    {
        throw std::invalid_argument("Pixel buffer size mismatch");
    }

    ImageAnalysis result{};

    uint64_t histogram[3][256]{};
    const uint64_t pixel_count = static_cast<uint64_t>(width) * height;

    long double total_sum = 0.0L;
    long double total_sum_squared = 0.0L;

    for (size_t i = 0; i < pixel_count; ++i)
    {
        for (size_t c = 0; c < 3; ++c)
        {
            const uint8_t value = pixels[i * channels + c];

            ++histogram[c][value];
            total_sum += value;
            total_sum_squared += static_cast<long double>(value) * value;
        }
    }

    double entropy_sum = 0.0;
    double uniformity_sum = 0.0;

    for (size_t c = 0; c < 3; ++c)
    {
        std::memcpy(result.histogram[c], histogram[c], sizeof(histogram[c]));

        double entropy = 0.0;
        double distance = 0.0;

        for (size_t i = 0; i < 256; ++i)
        {
            const double probability = static_cast<double>(histogram[c][i]) / pixel_count;

            if (probability > 0.0)
            {
                entropy -= probability * std::log2(probability);
            }

            distance += std::abs(probability - 1.0 / 256.0);
        }

        entropy_sum += entropy;
        uniformity_sum += 1.0 - distance / 2.0;
    }

    result.entropy = entropy_sum / 3.0;
    result.histogram_uniformity = uniformity_sum / 3.0;

    const long double sample_count = static_cast<long double>(pixel_count) * 3.0L;

    const long double mean = total_sum / sample_count;

    const long double variance = std::max(0.0L, total_sum_squared / sample_count - mean * mean);

    result.mean = static_cast<double>(mean);
    result.standard_deviation = static_cast<double>(std::sqrt(variance));

    // Korelacje poziome, pionowe i diagonalne.
    long double correlation_sum = 0.0L;
    uint64_t correlation_count = 0;

    auto calculate_correlation = [](long double sum_x, long double sum_y, long double sum_x2, long double sum_y2, long double sum_xy, uint64_t count) -> double
    {
        if (count < 2)
        {
            return 0.0;
        }

        const long double n = count;
        const long double covariance = n * sum_xy - sum_x * sum_y;
        const long double variance_x = n * sum_x2 - sum_x * sum_x;
        const long double variance_y = n * sum_y2 - sum_y * sum_y;

        if (variance_x <= 0.0L || variance_y <= 0.0L)
        {
            return 0.0;
        }

        return static_cast<double>(covariance / std::sqrt(variance_x * variance_y));
    };

    for (size_t c = 0; c < 3; ++c)
    {
        for (int direction = 0; direction < 3; ++direction)
        {
            long double sx = 0.0L;
            long double sy = 0.0L;
            long double sx2 = 0.0L;
            long double sy2 = 0.0L;
            long double sxy = 0.0L;
            uint64_t count = 0;

            for (uint32_t y = 0; y < height; ++y)
            {
                for (uint32_t x = 0; x < width; ++x)
                {
                    uint32_t nx = x;
                    uint32_t ny = y;

                    if (direction == 0)
                    {
                        if (x + 1 >= width)
                        {
                            continue;
                        }
                        nx = x + 1;
                    }
                    else if (direction == 1)
                    {
                        if (y + 1 >= height)
                        {
                            continue;
                        }
                        ny = y + 1;
                    }
                    else
                    {
                        if (x + 1 >= width || y + 1 >= height)
                        {
                            continue;
                        }
                        nx = x + 1;
                        ny = y + 1;
                    }

                    const uint8_t a = pixels[(static_cast<size_t>(y) * width + x) * channels + c];

                    const uint8_t b = pixels[(static_cast<size_t>(ny) * width + nx) * channels + c];

                    sx += a;
                    sy += b;
                    sx2 += static_cast<long double>(a) * a;
                    sy2 += static_cast<long double>(b) * b;
                    sxy += static_cast<long double>(a) * b;
                    ++count;
                }
            }

            if (count > 0)
            {
                correlation_sum += std::abs(calculate_correlation(
                    sx, sy, sx2, sy2, sxy, count
                    ));
                ++correlation_count;
            }
        }
    }

    result.pixel_correlation = correlation_count > 0 ? static_cast<double>(correlation_sum / correlation_count) : 0.0;

    return result;
}
