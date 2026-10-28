using System;
using System.Linq;

const uint width = 100;
const uint height = 100;
const int channels = 3;

byte[] pixels = new byte[(int)(width * height * channels)];

Random random = new Random(12345);
random.NextBytes(pixels);

byte[] original = (byte[])pixels.Clone();

Console.WriteLine("Analizowanie oryginalnego obrazu...");

CryptoNative.ImageAnalysis originalAnalysis = ImageCrypto.Analyze(pixels, width, height);

Console.WriteLine($"  Entropia: {originalAnalysis.Entropy:F4}");
Console.WriteLine($"  Srednia:  {originalAnalysis.Mean:F2}");
Console.WriteLine($"  Odch. std: {originalAnalysis.StandardDeviation:F2}");

unsafe
{
    ulong redZeroBin = originalAnalysis.Histogram[0];
    Console.WriteLine($"  Histogram R[0]: {redZeroBin}");
}

Console.WriteLine("\nSzyfrowanie...");

double[] key = ImageCrypto.Encrypt(pixels, width, height);

Console.WriteLine("Szyfrowanie zakonczone sukcesem.");

Console.WriteLine("\nAnalizowanie zaszyfrowanego obrazu...");

CryptoNative.ImageAnalysis encryptedAnalysis = ImageCrypto.Analyze(pixels, width, height);

Console.WriteLine($"  Entropia: {encryptedAnalysis.Entropy:F4}");
Console.WriteLine($"  Srednia: {encryptedAnalysis.Mean:F2}");

Console.WriteLine("\nOdszyfrowywanie...");

ImageCrypto.Decrypt(pixels, width, height, key);

Console.WriteLine("Odszyfrowywanie zakonczone sukcesem.");

Console.WriteLine("\nWeryfikacja pikseli...");

if (pixels.SequenceEqual(original))
{
    Console.WriteLine("SUKCES: Odszyfrowane piksele sa identyczne z oryginalem.");
}
else
{
    Console.WriteLine("BLAD: Odszyfrowane piksele roznia sie od oryginalu!");
    Environment.ExitCode = 1;
}