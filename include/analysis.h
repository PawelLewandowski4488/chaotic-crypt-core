#ifndef ANALYSIS_H
#define ANALYSIS_H

#include <cstdint>
#include <crypto.h>

struct ImageAnalysis
{
    double entropy;
    double pixel_correlation;
    double histogram_uniformity;
    double mean;
    double standard_deviation;

    uint64_t histogram[3][256];
};

ImageAnalysis analyze_image(const uint8_t* pixels, size_t size, uint32_t width, uint32_t height, PixelFormat format);

#endif // ANALYSIS_H
