using System.Runtime.InteropServices;
using System.Security.Cryptography;
using Microsoft.Security.Extensions;
namespace ClickJack.Extensions;

public enum SignLevel
{
    Unknown,
    Untrusted,
    Trusted,
    EV
}

public static class General
{
    public static string FixPath(this string path)
    {
        return Path.DirectorySeparatorChar != '\\' ? path.Replace('\\', Path.DirectorySeparatorChar) : path;
    }

    public static string FileHashDigest(this string filepath)
    {
        var data = File.ReadAllBytes(filepath);
        byte[] hash;
        using (var sha1Hasher = SHA256.Create())
        {
            hash = sha1Hasher.ComputeHash(data);
        }

        var hash64 = Convert.ToBase64String(hash);
        return hash64;
    }

    public static SignLevel VerifySignature(this string path)
    {
        var possibleEVNames = new List<string>() { " EV ", "ExtendedValidation" };
        if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
        {
            using (FileStream fs = File.OpenRead(path))
            {
                FileSignatureInfo sigInfo = FileSignatureInfo.GetFromFileStream(fs);
                if (sigInfo.State == SignatureState.SignedAndTrusted)
                {
                    if (possibleEVNames.Any(sigInfo.SigningCertificate.Issuer.Contains))
                    {
                        return SignLevel.EV;
                    }
                    return SignLevel.Trusted;
                }

                return SignLevel.Untrusted;
            }
        }
        else
        {
            return SignLevel.Unknown;
        }
    }
}