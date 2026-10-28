using System;
using System.Runtime.InteropServices;

public static class CryptoNative
{
    private const string LibraryName = "libimagecrypto";

    internal enum CryptoStatus : int
    {
        Success = 0,
        InvalidArgument = 1,
        InvalidKey = 2,
        OutOfMemory = 3,
        InternalError = 4
    }

    internal enum CryptoPixelFormat : int
    {
        RGB = 3,
        RGBA = 4
    }

    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct ImageAnalysis
    {
        public double Entropy;
        public double PixelCorrelation;
        public double HistogramUniformity;
        public double Mean;
        public double StandardDeviation;

        public fixed ulong Histogram[768];
    }

    [DllImport(LibraryName, EntryPoint = "crypto_encrypt", CallingConvention = CallingConvention.Cdecl)]
    internal static extern CryptoStatus Encrypt(
        byte[] pixels,
        UIntPtr size,
        uint width,
        uint height,
        CryptoPixelFormat format,
        [Out] double[] key
    );

    [DllImport(LibraryName, EntryPoint = "crypto_decrypt", CallingConvention = CallingConvention.Cdecl)]
    internal static extern CryptoStatus Decrypt(
        byte[] pixels,
        UIntPtr size,
        uint width,
        uint height,
        CryptoPixelFormat format,
        [In] double[] key
    );

    [DllImport(LibraryName, EntryPoint = "crypto_analyze", CallingConvention = CallingConvention.Cdecl)]
    internal static extern CryptoStatus Analyze(
        byte[] pixels,
        UIntPtr size,
        uint width,
        uint height,
        CryptoPixelFormat format,
        out ImageAnalysis result
    );
}
