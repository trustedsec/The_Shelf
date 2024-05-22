using ClickJack.Extensions;
using ClickJack.Modules;

class FindCandidates
{
    private static bool verbose = false;
    public static void Find(string[] args)
    {
        string rootDirectory = args.GetValue("--dir");
        verbose = args.Contains("--verbose");
        InjectApp.verbose = verbose;
        ProcessApplicationFiles(rootDirectory);
        Console.WriteLine("Done");
    }
    static void ProcessApplicationFiles(string directoryPath)
    {
        // Search for .application files in the current directory
        string[] applicationFiles = Directory.GetFiles(directoryPath, "*.application");

        // Process each .application file found
        foreach (string applicationFile in applicationFiles)
        {
            // Call the function on each .application file
            if (InjectApp.CheckInjectable(applicationFile))
            {
                var color = Console.ForegroundColor;
                Console.ForegroundColor = ConsoleColor.Green;
                Console.WriteLine($"[+] {applicationFile} is injectable!");
                Console.ForegroundColor = color;
            }
            else
            {
                if(verbose)
                    Download.WriteError($"[!] {applicationFile} is not injectable");
            }
            
        }

        // Recursively search subdirectories
        string[] subDirectories = Directory.GetDirectories(directoryPath);
        foreach (string subDirectory in subDirectories)
        {
            ProcessApplicationFiles(subDirectory);
        }
    }
}