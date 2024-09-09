using System;
using System.IO;
using Titanis.Cli;

namespace Dll2Shellcode
{
    public enum Arch
    {
        x86,
        x64
    }
    class parser
    {
        [Parameter(0, Mandatory = true)]
        public string inputdllpath { get; set; }

        [Parameter(1, Mandatory = true)] 
        public string outputShellcodePath { get; set; }

        [Parameter] 
        public SwitchParam NoClearHeaders { get; set; }
        
        [Parameter]
        public SwitchParam NoClearStagingMemory { get; set; }

        [Parameter] public ushort DelayImports { get; set; } = 0;

        [Parameter] public string ExportToCall { get; set; } = "";

    }
    
    class Program
    {
        static void Usage()
        {
            Console.WriteLine(@"Dll2Shellcode <Input Dll path> <output shellcode path>
Additional options:
    -NoClearHeaders (switch): Does not stomp the PE headers in the loaded section
    -NoClearStagingMemory (switch): Does not free the used memory of the initial dll blob load
    -DelayImports (int): Adds the specified number of seconds between each dll import resolution
    -ExportToCall (string): Calls the specified dll export after running dllmain.  Called export is started using beginthreadex");
        }
        
        static void Main(string[] args)
        {
            parser cmd = new parser();
            try
            {
                CommandLineParser.Parse(args, cmd);
            } catch (ArgumentException e)
            {
                Usage();
                Console.WriteLine();
                Console.WriteLine(e.Message);
                return;
            }

            Span<byte> bytes;
            int peoffset = 0;
            ushort machine = 0;

            byte[] input = File.ReadAllBytes(cmd.inputdllpath);

            bytes = input;
            peoffset = BitConverter.ToInt32(bytes.Slice(0x3c, 4));
            machine = BitConverter.ToUInt16(bytes.Slice(peoffset+24, 2));
            Arch dllarch = (machine == 0x10b) ? Arch.x86 : Arch.x64;
            if(!(machine == 0x10b || machine == 0x20b))
            {
                Console.WriteLine("Input Does not appear to be a valid x86 or x64 dll, Please report to developer if you think otherwise");
                return;
            }
            
            
            byte[] output = sRDI.ConvertToShellcode(
                input,
                dllarch,
                cmd.ExportToCall,
                !cmd.NoClearHeaders.GetValueOrDefault(false),
                !cmd.NoClearStagingMemory.GetValueOrDefault(false),
                cmd.DelayImports
            );

            File.WriteAllBytes(cmd.outputShellcodePath, output);
            Console.WriteLine($"Your {((dllarch == Arch.x64) ? "64bit" : "32bit")} Dll has been converted and output was written to {cmd.outputShellcodePath}");

        }
    }
}