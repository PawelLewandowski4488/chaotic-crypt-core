using System;
using System.Runtime.InteropServices;

internal static class CryptoNative
{
    private const string LibraryName = "libimagecrypto";

    internal enum CryptoStatus : int
    {
        Success = 0,
        InvalidArgument = 1,
        OutOfMemory = 2,
        InternalError = 3
    }

    internal enum CryptoPixelFormat : int
    {
        RGB = 3,
        RGBA = 4
    }

    [DllImport(LibraryName, EntryPoint = "crypto_encrypt",
        CallingConvention = CallingConvention.Cdecl)]
    internal static extern CryptoStatus Encrypt(
        byte[] pixels,
        UIntPtr size,
        uint width,
        uint height,
        CryptoPixelFormat format,
        [Out] double[] key
    );

    [DllImport(LibraryName, EntryPoint = "crypto_decrypt",
        CallingConvention = CallingConvention.Cdecl)]
    internal static extern CryptoStatus Decrypt(
        byte[] pixels,
        UIntPtr size,
        uint width,
        uint height,
        CryptoPixelFormat format,
        [In] double[] key
    );
}
