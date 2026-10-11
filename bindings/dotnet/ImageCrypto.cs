
using System;

public static class ImageCrypto
{
    public static double[] Encrypt(
        byte[] pixels,
        uint width,
        uint height,
        bool hasAlpha = false)
    {
        ValidatePixels(pixels, width, height, hasAlpha);

        double[] key = new double[10];

        CryptoNative.CryptoPixelFormat format = hasAlpha
            ? CryptoNative.CryptoPixelFormat.RGBA
            : CryptoNative.CryptoPixelFormat.RGB;

        CryptoNative.CryptoStatus status = CryptoNative.Encrypt(
            pixels,
            new UIntPtr((ulong)pixels.Length),
            width,
            height,
            format,
            key
        );

        CheckStatus(status);

        return key;
    }

    public static void Decrypt(
        byte[] pixels,
        uint width,
        uint height,
        double[] key,
        bool hasAlpha = false)
    {
        ValidatePixels(pixels, width, height, hasAlpha);

        if(key == null || key.Length != 10)
        {
            throw new ArgumentException("Key must contain exactly 10 doubles.");
        }

        CryptoNative.CryptoPixelFormat format = hasAlpha
            ? CryptoNative.CryptoPixelFormat.RGBA
            : CryptoNative.CryptoPixelFormat.RGB;

        CryptoNative.CryptoStatus status = CryptoNative.Decrypt(
            pixels,
            new UIntPtr((ulong)pixels.Length),
            width,
            height,
            format,
            key
        );

        CheckStatus(status);
    }

    private static void ValidatePixels(
        byte[] pixels,
        uint width,
        uint height,
        bool hasAlpha)
    {
        if(pixels == null)
        {
            throw new ArgumentNullException(nameof(pixels));
        }

        int channels = hasAlpha ? 4 : 3;
        ulong expectedSize = (ulong)width * height * (uint)channels;

        if(width == 0 || height == 0 || (ulong)pixels.Length != expectedSize)
        {
            throw new ArgumentException("Invalid image dimensions or pixel buffer size.");
        }
    }

    private static void CheckStatus(CryptoNative.CryptoStatus status)
    {
        if(status != CryptoNative.CryptoStatus.Success)
        {
            throw new InvalidOperationException(
                $"Native crypto operation failed: {status}"
            );
        }
    }
}
