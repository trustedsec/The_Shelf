using System.Text;
using System.Xml;
using ClickJack.Extensions;

namespace ClickJack.Modules;

//Potential mismatch can occur when we null everything, figure out what went wrong

public class FixManifest
{
    private static bool verbose = false;
    private static XmlWriterSettings _xmlWriterSettings = new XmlWriterSettings();
    public static void FixClickOnceManifest(string[] args)
    {
        _xmlWriterSettings.Indent = true;
        _xmlWriterSettings.NewLineChars = "\n";
        verbose = Enumerable.Contains(args, "--verbose");
        var appManifestPath = args.GetValue("--appmanifest");
        var hostingUrl = args.GetValue("--hosturl");
        File.Copy(appManifestPath, $"{appManifestPath}.original", true);
        XmlDocument appManifestXml = new XmlDocument();
        appManifestXml.Load(appManifestPath);
        //Null out public Keys in .application.  We always want to null both
        NullPubkeys(appManifestXml);
        appManifestXml.GetElementsByTagName("deploymentProvider")[0].Attributes["codebase"].Value = hostingUrl;
        //We hold off repairing hashes until we've processed the .manifest xml
        //resolve application manifest and do the same
        XmlNode? provider = appManifestXml.GetElementsByTagName("dependentAssembly")[0];
        if (provider == null)
        {
            Download.WriteError("Unable to resolve deployment manifest path");
            return;
        }
        string? manifestpath = provider.Attributes?["codebase"]?.Value?.FixPath();
        if (manifestpath == null)
        {
            Download.WriteError("Unable to resolve deployment manifest path codebase");
            return;
        }

        var appParentDirectory = Directory.GetParent(appManifestPath);
        manifestpath = Path.Combine(appParentDirectory.FullName, manifestpath);
        File.Copy(manifestpath, $"{manifestpath}.original", true);
        var deploymentManifestXml = new XmlDocument();
        deploymentManifestXml.Load(manifestpath);
        //NullPubkeys(deploymentManifestXml);
        var t = deploymentManifestXml.DocumentElement.FirstChild;
        t.Attributes["publicKeyToken"].Value = "0000000000000000";
        RepairHashes(deploymentManifestXml, manifestpath);
        using (var stream = new StreamWriter(manifestpath, false, Encoding.UTF8))
        {
            using (var xmlstream = XmlWriter.Create(stream, _xmlWriterSettings))
            {
                deploymentManifestXml.WriteTo(xmlstream);
            }
        }
        RepairHashes(appManifestXml, appManifestPath);
        using (var stream = new StreamWriter(appManifestPath, false, Encoding.UTF8))
        {
            using (var xmlstream = XmlWriter.Create(stream, _xmlWriterSettings))
            {
                appManifestXml.WriteTo(xmlstream);
            }
        }
        Console.WriteLine("Modifications have been made and saved");

    }
    
    public static void NullPubkeys(XmlDocument manifest)
    {
        XmlNodeList assemblyIdentities = manifest.GetElementsByTagName("assemblyIdentity");
        foreach (XmlNode ident in assemblyIdentities)
        {
            if (ident.Attributes == null) continue;
            foreach (XmlAttribute attr in ident.Attributes)
            {
                if (attr.Name == "publicKeyToken")
                {
                    attr.Value = "0000000000000000";
                }
            }
        }
    }

    public static void RemoveHashes(XmlDocument manifest)
    {
        XmlNodeList hashes = manifest.GetElementsByTagName("hash");
        List<Tuple<XmlNode?, XmlNode>> pairs = new List<Tuple<XmlNode?, XmlNode>>();
        foreach (XmlNode hash in hashes)
        {
            pairs.Add(Tuple.Create(hash.ParentNode, hash));
        }

        foreach (var item in pairs)
        {
            if (item.Item1 != null)
            {
                item.Item1.RemoveChild(item.Item2);
            }
        }
    }

    public static void RepairHashes(XmlDocument manifest, string manifestPath)
    {
        string basedir = Path.Join(manifestPath, "..");
        XmlNodeList hashes = manifest.GetElementsByTagName("hash");
        List<Tuple<XmlNode?, XmlNode>> pairs = new List<Tuple<XmlNode?, XmlNode>>();
        foreach (XmlNode hash in hashes)
        {
            pairs.Add(Tuple.Create(hash.ParentNode, hash));
        }
        foreach (var item in pairs)
        {
            if (item.Item1 != null)
            {
                string? targetpath = null;
                bool assembly = false;
                if (item.Item1.LocalName == "dependentAssembly")
                {
                    targetpath = item.Item1.Attributes?["codebase"]?.Value;
                    assembly = true;
                }
                else
                {
                    targetpath = item.Item1.Attributes?["name"]?.Value;
                }

                if (targetpath == null) continue;
                var path = Path.Join(basedir, targetpath.FixPath());
                if (!File.Exists(path))
                {
                    path = $"{path}.deploy";
                }

                var digest = path.FileHashDigest();
                var currentDigest = item.Item2.LastChild.InnerText;
                if (digest != currentDigest)
                {
                    item.Item2.LastChild.InnerText = digest;
                    var len = new FileInfo(path).Length;
                    item.Item1.Attributes["size"].Value = $"{len}";
                    if (assembly)
                    {
                        var ident = item.Item1.FirstChild;
                        var pubkeytoken = ident?.Attributes?["publicKeyToken"];
                        if (pubkeytoken != null && pubkeytoken.Value != "0000000000000000")
                        {
                            var color = Console.ForegroundColor;
                            Console.ForegroundColor = ConsoleColor.Red;
                            Console.WriteLine($"[!] Your modification is in a strongly named assembly, this likely won't work");
                            Console.ForegroundColor = color;
                        }
                    }
                }

            }
        }
    }
}