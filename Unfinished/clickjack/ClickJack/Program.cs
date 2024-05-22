using System;
using ClickJack.Modules;

namespace ClickJack // Note: actual namespace depends on the project name.
{
    internal class Program
    {
        private const string art = @"
        CCCCCCCCCCCCClllllll   iiii                      kkkkkkkk                  JJJJJJJJJJJ                                   kkkkkkkk           
     CCC::::::::::::Cl:::::l  i::::i                     k::::::k                  J:::::::::J                                   k::::::k           
   CC:::::::::::::::Cl:::::l   iiii                      k::::::k                  J:::::::::J                                   k::::::k           
  C:::::CCCCCCCC::::Cl:::::l                             k::::::k                  JJ:::::::JJ                                   k::::::k           
 C:::::C       CCCCCC l::::l iiiiiii     cccccccccccccccc k:::::k    kkkkkkk         J:::::J  aaaaaaaaaaaaa      cccccccccccccccc k:::::k    kkkkkkk
C:::::C               l::::l i:::::i   cc:::::::::::::::c k:::::k   k:::::k          J:::::J  a::::::::::::a   cc:::::::::::::::c k:::::k   k:::::k 
C:::::C               l::::l  i::::i  c:::::::::::::::::c k:::::k  k:::::k           J:::::J  aaaaaaaaa:::::a c:::::::::::::::::c k:::::k  k:::::k  
C:::::C               l::::l  i::::i c:::::::cccccc:::::c k:::::k k:::::k            J:::::j           a::::ac:::::::cccccc:::::c k:::::k k:::::k   
C:::::C               l::::l  i::::i c::::::c     ccccccc k::::::k:::::k             J:::::J    aaaaaaa:::::ac::::::c     ccccccc k::::::k:::::k    
C:::::C               l::::l  i::::i c:::::c              k:::::::::::k  JJJJJJJ     J:::::J  aa::::::::::::ac:::::c              k:::::::::::k     
C:::::C               l::::l  i::::i c:::::c              k:::::::::::k  J:::::J     J:::::J a::::aaaa::::::ac:::::c              k:::::::::::k     
 C:::::C       CCCCCC l::::l  i::::i c::::::c     ccccccc k::::::k:::::k J::::::J   J::::::Ja::::a    a:::::ac::::::c     ccccccc k::::::k:::::k    
  C:::::CCCCCCCC::::Cl::::::li::::::ic:::::::cccccc:::::ck::::::k k:::::kJ:::::::JJJ:::::::Ja::::a    a:::::ac:::::::cccccc:::::ck::::::k k:::::k   
   CC:::::::::::::::Cl::::::li::::::i c:::::::::::::::::ck::::::k  k:::::kJJ:::::::::::::JJ a:::::aaaa::::::a c:::::::::::::::::ck::::::k  k:::::k  
     CCC::::::::::::Cl::::::li::::::i  cc:::::::::::::::ck::::::k   k:::::k JJ:::::::::JJ    a::::::::::aa:::a cc:::::::::::::::ck::::::k   k:::::k 
        CCCCCCCCCCCCClllllllliiiiiiii    cccccccccccccccckkkkkkkk    kkkkkkk  JJJJJJJJJ       aaaaaaaaaa  aaaa   cccccccccccccccckkkkkkkk    kkkkkkk
";
        static void Usage()
        {
            Console.WriteLine($@"{art}

This program is designed to assist with downloading and modifying clickonce packages.

--Download:         (Download clickonce packages via links to the .application or referencing .html page)
    --file:         (required.  Path to file containing links we will download)
    --outputdir:    (required.  Path to folder where packages will be output
    --verbose:      (optional, flag.  add move verbose output

--FixManifest:      (Repair clickonce manifest after modifying dependent files)
    --appmanifest   (required.  Path to .application file of clickonce package to fix)
    --hosturl       (required. full URL where you intend to host the .application file.
    --verbose       (optional, flag.  Add more verbose output

--CollectLinks:     (Download .application links found on grayhatwarfair)
    --apitoken      (required. api token to interact with grayhat api
    --outfile       (required. file to write links into

--Inject:           (Inject a static C# method into the first weakly called assembly of a c# app
    --stub          (required if --dryrun is not specified. C# assembly that defines a static void StubMethod() which is your code to inject
    --app           (required. .application file you want to attempt to inject
    --dryrun        (flag.   when specified rather then injecting the app we see if it is injectable and trace it
    --verbose       (flag.  print all calls leading up to a weak assembly call

--FindCandidates    (recursively search a given directory for .application files that are injectable candidates
    --dir           (required, directory to start search at.
    --verbose       (flag.  Show all the errors that occur");
            Environment.Exit(1);
        }
        
        public static async Task Main(string[] args)
        {
            if (args.Length == 0)
            {
                Usage();
            }
            else
            {
                if (args.Contains("--Download"))
                {
                    await Download.RunDownloadTasks(args);
                }
                else if (args.Contains("--FixManifest"))
                {
                    FixManifest.FixClickOnceManifest(args);
                }
                else if (args.Contains("--CollectLinks"))
                {
                    await GrayhatApi.DownloadGrayhatLinks(args);
                }
                else if (args.Contains("--testhash"))
                {
                    Tests.HashFile(args);
                }
                else if (args.Contains("--Inject"))
                {
                    InjectApp.InfectClickonceApp(args);
                }
                else if (args.Contains("--FindCandidates"))
                {
                    FindCandidates.Find(args);
                }
                else
                {
                    Console.WriteLine("Did not find a valid command");
                    Usage();
                }
            }
            
        }
    }
}