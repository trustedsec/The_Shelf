using System.Security.Cryptography;

namespace ClickJack.Modules;
using ClickJack.Extensions;

public class Tests
{
    public static void HashFile(string[] args)
    {
        var filepath = args.GetValue("--path");
        var data = File.ReadAllBytes(filepath);
        byte[] hash;
        using (var sha1Hasher = SHA256.Create())
        {
            hash = sha1Hasher.ComputeHash(data);
        }

        var hash64 = Convert.ToBase64String(hash);
        Console.WriteLine(hash64);


    }
}