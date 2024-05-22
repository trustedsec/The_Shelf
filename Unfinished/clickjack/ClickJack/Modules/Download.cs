using System.Runtime.InteropServices;
using System.Text;
using System.Collections.Concurrent;
using System.Runtime.CompilerServices;

namespace ClickJack.Modules;
using Extensions;
using System.Text.RegularExpressions;
using System.Xml;

// Need to update to write out with bom_template
// actually save files with .deploy or not
//
// null out publicKeyToken 
//     remove digest hHash


public class Download
{

    static string appManifestPattern =
        @"(?:http(s)?:\/\/)?[\w.-]+(?:\.[\w\.-]+)+[\w\-\._~:/?#[\]@!\$&'\(\)\*\+,;=.]+(.application|.appref-ms)";

    private static bool verbose = false;

    private static string hrefManifestPattern =
        @"(?:href="")([\w\-_~:/?#[\]@!\$&'\(\)\*\+ ,;=]+.application|.appref-ms)";

    private static string baseOutputFolder;

    //static string appDeploymentPattern = @"<dependentAssembly ([\w="" ])+codebase=""(?<codebaseurl>[\w\-\._~:/?#[\]@!\$&'\(\)\*\+,;=. \\]+)";
    //private static Regex appDeploymentRgx =
    //    new Regex(appDeploymentPattern, RegexOptions.Compiled | RegexOptions.IgnoreCase);
    static Regex Rgx = new Regex(appManifestPattern, RegexOptions.Compiled | RegexOptions.IgnoreCase);
    private static Regex hrefRgx = new Regex(hrefManifestPattern, RegexOptions.Compiled | RegexOptions.IgnoreCase);
    private static ConcurrentDictionary<string, bool> dlitems = new ConcurrentDictionary<string, bool>();

    public static async Task DownloadClickOnceAppsAsync(string link)
    {
        
            baseOutputFolder = Path.GetFullPath(baseOutputFolder);
            var baseOutputDirectoryInfo = Directory.CreateDirectory(baseOutputFolder);
            if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
            {
                baseOutputDirectoryInfo.CreateSubdirectory("trusted");
                baseOutputDirectoryInfo.CreateSubdirectory("untrusted");
                baseOutputDirectoryInfo.CreateSubdirectory("unknown");
                baseOutputDirectoryInfo.CreateSubdirectory("EV");
            }
            
            var webclient = new HttpClient();
            var applicationManifest = new System.Xml.XmlDocument();
            var deploymentManifest = new System.Xml.XmlDocument();
            string applicationManifestText;

            List<string> results = new List<string>();
            if (!link.EndsWith(".application"))
            {
                string html;

                try
                {
                    html = await webclient.GetStringAsync(link);
                }
                catch (HttpRequestException e)
                {
                    WriteError($"[-] Failed to download html from page {link}");
                    return;
                }

                var matches = Rgx.Matches(html);

                if (matches.Count == 0)
                {
                    matches = hrefRgx.Matches(html);
                    var baselink = new Uri(link);
                    foreach (Match match in matches)
                    {
                        var targetlink = new Uri(baselink, $"{match.Groups[1].Value}").AbsoluteUri;
                        results.Add(targetlink);
                    }
                }
                else
                {
                    results = matches.Select(x => x.Value).ToList();
                }
            }
            else
            {
                results.Add(link);
            }

            foreach (string result in results)
            {
                string applink;
                if (result.EndsWith("appref-ms", StringComparison.OrdinalIgnoreCase))
                {
                    var apprefString = await webclient.GetStringAsync(result);
                    var apprefMatch = Rgx.Match(apprefString);
                    if (apprefMatch.Success)
                        applink = apprefMatch.Value;
                    else
                    {
                        continue;
                    }
                }
                else
                {
                    applink = result;
                }

                try
                {
                    applicationManifestText = await webclient.GetStringAsync(applink);
                    WriteDebug($"[+] downloaded application manifest from {applink}");
                }
                catch (HttpRequestException e)
                {
                    WriteError($"[-] Failed to download manifest at {applink}");
                    continue;
                }


                applicationManifest.LoadXml(applicationManifestText);
                string? codebase = applicationManifest.GetElementsByTagName("dependentAssembly")?.Item(0)
                    ?.Attributes?["codebase"]?.Value;
                if (codebase is null)
                {
                    WriteError("[-] Failed to find codebase for Deployment Manifest, defaulting to where we downloaded from");
                    continue;
                }

                string? app_codebase = applicationManifest.GetElementsByTagName("deploymentProvider")?.Item(0)
                    ?.Attributes?["codebase"]?.Value;
                if (app_codebase is null)
                {
                    WriteError("[-] Unable to find application codebase from downloaded manifest file, defaulting to where we dl .app from");
                    app_codebase = applink;
                }
                var appUri = new Uri(app_codebase);
                var mapExtentions = applicationManifest.GetElementsByTagName("deployment")?.Item(0)
                    ?.Attributes?["mapFileExtensions"]?.Value == "true";
                var name = applicationManifest.GetElementsByTagName("assemblyIdentity")?.Item(0)
                    ?.Attributes?["name"]
                    ?.Value;
                var version = applicationManifest.GetElementsByTagName("assemblyIdentity")?.Item(0)
                    ?.Attributes?["version"]?.Value ?? "1.0.0.0";
                
                if (name is null)
                {
                    name = Path.GetFileName(appUri.LocalPath);
                }

                name = $"{name}_{version}";
                if (dlitems.ContainsKey(name))
                {
                    WriteDebug("[.] already processed item from other source, continuing");
                    continue;
                }
                else
                {
                    dlitems[name] = true;
                }
                var deploymentManifestUri = new Uri(appUri, codebase);
                var appFolder = Directory.CreateDirectory(Path.Combine(baseOutputDirectoryInfo.FullName, name));
                string deploymentBaseDir;
                if (Path.DirectorySeparatorChar != '\\')
                {

                    deploymentBaseDir = appFolder
                        .CreateSubdirectory(Path.Combine(codebase.Replace('\\', Path.DirectorySeparatorChar), ".."))
                        .FullName;
                }
                else
                {
                    deploymentBaseDir = appFolder.CreateSubdirectory(Path.Combine(codebase, "..")).FullName;
                }

                File.WriteAllText(Path.Combine(appFolder.FullName, Path.GetFileName(appUri.LocalPath)),
                    applicationManifestText, Encoding.UTF8);

                string deploymentManifestText;
                try
                {
                    deploymentManifestText = await webclient.GetStringAsync(deploymentManifestUri);
                    WriteDebug($"[+] Downloaded Deployment Manifest from {deploymentManifestUri}");
                }
                catch (HttpRequestException e)
                {
                    WriteError(
                        $"[-] Failed to download Deployment manifest at uri {deploymentManifestUri.AbsoluteUri}");
                    Directory.Delete(appFolder.FullName, true);
                    continue;
                }

                File.WriteAllText(Path.Combine(deploymentBaseDir,
                    Path.GetFileName(deploymentManifestUri.LocalPath)), deploymentManifestText, Encoding.UTF8);

                deploymentManifest.LoadXml(deploymentManifestText);
                var deploymentDependencies = deploymentManifest.GetElementsByTagName("dependentAssembly");
                bool done = true;
                foreach (XmlNode depend in deploymentDependencies)
                {
                    if (depend.Attributes["dependencyType"]?.Value == "install")
                    {
                        var path = depend.Attributes["codebase"]?.Value;
                        if (path is not null)
                        {
                            if (!await DownloadDependencyAsync(deploymentManifestUri, path, deploymentBaseDir, mapExtentions))
                            {
                                WriteError($"[-] Failed to download dependent assembly at path {path}");
                                Directory.Delete(appFolder.FullName, true);
                                done = false;
                                break;
                            }

                            WriteDebug($"[+] Downloaded Dependent Assembly {path}");
                        }
                    }
                }

                if (!done)
                {
                    continue;
                }

                var fileDependencies = deploymentManifest.GetElementsByTagName("file");
                {
                    foreach (XmlNode depend in fileDependencies)
                    {
                        var path = depend.Attributes["name"]?.Value;
                        if (path is not null)
                        {
                            if (!await DownloadDependencyAsync(deploymentManifestUri, path, deploymentBaseDir, mapExtentions))
                            {
                                WriteError($"[-] Failed to download dependent file at path {path}");
                                Directory.Delete(appFolder.FullName, true);
                                done = false;
                                break;
                            }

                            WriteDebug($"[+] Downloaded Dependency {path}");
                        }
                    }
                }
                if (!done)
                {
                    continue;
                }

                var color = Console.ForegroundColor;
                Console.ForegroundColor = ConsoleColor.Green;
                Console.WriteLine($"[+] Finished processing {name}");
                Console.ForegroundColor = color;

                if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
                {
                    var nodes = deploymentManifest.GetElementsByTagName("commandLine");
                    var entryFile = nodes?[0]?.Attributes?["file"]?.Value;
                    var appName = appFolder.Name;
                    if (entryFile != null)
                    {
                        var file = Path.Join(deploymentBaseDir, entryFile.FixPath());
                        if (mapExtentions)
                        {
                            file += ".deploy";
                        }

                        var sig = file.VerifySignature();
                        while (true)
                        {
                            try
                            {
                                if (sig == SignLevel.Trusted)
                                {
                                    appFolder.MoveTo(Path.Join(Path.Combine(baseOutputDirectoryInfo.FullName,
                                        "trusted",
                                        appName)));
                                }
                                else if (sig == SignLevel.Untrusted)
                                {
                                    appFolder.MoveTo(Path.Join(Path.Combine(baseOutputDirectoryInfo.FullName,
                                        "untrusted",
                                        appName)));
                                }
                                else if (sig == SignLevel.EV)
                                {
                                    appFolder.MoveTo(Path.Join(Path.Combine(baseOutputDirectoryInfo.FullName, "EV",
                                        appName)));
                                }
                                else
                                {
                                    appFolder.MoveTo(Path.Join(Path.Combine(baseOutputDirectoryInfo.FullName,
                                        "unknown",
                                        appName)));
                                }

                                break;
                            }
                            catch (System.IO.IOException e)
                            {
                                Console.WriteLine(
                                    $"Directory already exists, likely already downloaded from other source\n error: {e}");
                                var end = appName[^4..];
                                var index = end.IndexOf('_');
                                if (index == -1)
                                {
                                    appName += "_1";
                                }
                                else
                                {
                                    index += 1;
                                    var number = end[index..];
                                    var n = IntPtr.Parse(number);
                                    n += 1;
                                    index -= 1;
                                    appName = $"{appName[..index]}_{n}";
                                }
                            }
                        }
                    }

                }

            }


            


        
    }

    public static async Task<bool> DownloadDependencyAsync(Uri baseUri, string path, string baseDirectory, bool mapExtension)
    {
        var webclient = new HttpClient();
        string target;
        if (mapExtension)
        {
            target = $"{path}.deploy";
        }
        else
        {
            target = path;
        }

        var targeturi = new Uri(baseUri, target);
        string targetpath = path.FixPath();
        if (mapExtension)
        {
            targetpath += ".deploy";
        }
        var filepath = Path.Combine(baseDirectory, targetpath);
        Directory.CreateDirectory(Path.Combine(filepath, ".."));
        try
        {
            var data = await webclient.GetByteArrayAsync(targeturi);
            await File.WriteAllBytesAsync(filepath, data);
        }
        catch (Exception e)
        {
            Console.WriteLine($"[-] Failed to download dependent file {path} : {e}");
            return false;
        }

        return true;
    }

    public static void WriteDebug(in string msg)
    {
        if (verbose)
        {
            Console.WriteLine(msg);
        }
    }

    public static void WriteError(in string msg)
    {
        var color = Console.ForegroundColor;
        Console.ForegroundColor = ConsoleColor.Red;
        Console.WriteLine(msg);
        Console.ForegroundColor = color;
    }
    
    public static async Task RunDownloadTasks(string[] args)
    {
        var linksFile = args.GetValue("--file");
        baseOutputFolder = args.GetValue("--outputdir");
        verbose = args.Contains("--verbose");
        
        if (linksFile == "" || baseOutputFolder == "")
        {
            Console.WriteLine("both --file and --outputdir are required");
            Environment.Exit(1);
        }
        // The semaphore limits to 10 concurrent downloads
        var semaphore = new SemaphoreSlim(30);

        var links = File.ReadAllLines(linksFile).Where(x => !string.IsNullOrEmpty(x)).ToList();

        var tasks = new List<Task>();

        foreach (var link in links)
        {
            await semaphore.WaitAsync();

            tasks.Add(
                DownloadClickOnceAppsAsync(link)
                    .ContinueWith(t =>
                    {
                        // Release the semaphore when the task completes so that more downloads can start
                        semaphore.Release();
                    }));
        }

        // Wait for all the downloads to complete
        await Task.WhenAll(tasks);
    }
}


