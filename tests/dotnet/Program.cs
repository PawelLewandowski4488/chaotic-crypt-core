using System;
using System.Linq;

const uint width = 100;
const uint height = 100;
const int channels = 3;

byte[] pixels = new byte[(int)(width * height * channels)];

Random random = new Random(12345);
random.NextBytes(pixels);

byte[] original = (byte[])pixels.Clone();

Console.WriteLine("Encrypting...");

double[] key = ImageCrypto.Encrypt(pixels, width, height);

Console.WriteLine("Encryption successful");

Console.WriteLine("Decrypting...");

ImageCrypto.Decrypt(pixels, width, height, key);

Console.WriteLine("Decryption successful");

if(pixels.SequenceEqual(original))
{
    Console.WriteLine("SUCCESS: Decrypted pixels match original");
}
else
{
    Console.WriteLine("FAILURE: Decrypted pixels differ from original");
    Environment.ExitCode = 1;
}
