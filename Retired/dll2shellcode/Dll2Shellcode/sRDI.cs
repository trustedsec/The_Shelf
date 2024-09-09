using System;
using System.IO;
using System.Linq;
using System.Text;


namespace Dll2Shellcode
{
    public static class sRDI
    {
        public static Func<UInt64, Int32, Int32, UInt64> ror = (UInt64 val, Int32 r_bits, Int32 max_bits) =>
            ((val & Convert.ToUInt64(Math.Pow(2 ,max_bits) - 1)) 
             >> (r_bits % max_bits)) |
            (val << (max_bits - (r_bits % max_bits)) & Convert.ToUInt64(Math.Pow(2 ,max_bits) - 1));

        public static UInt32 HashFunctionName(string name, string module = null)
        {
            if (name is null)
                return 0;
            var function = Encoding.UTF8.GetBytes(name).Concat(new byte[]{0}).ToArray();
            UInt64 functionHash = 0;
            if (module is not null)
            {
                var encoder = new UnicodeEncoding(false, false);
                var bmod = encoder.GetBytes(module.ToUpper()).Concat(new byte[]{0,0}).ToArray();

                foreach (byte b in function)
                {
                    functionHash = ror(functionHash, 13, 32);
                    functionHash += b;
                }

                UInt64 ModuleHash = 0;

                foreach (byte b in bmod)
                {
                    ModuleHash = ror(ModuleHash, 13, 32);
                    ModuleHash += b;
                }

                functionHash += ModuleHash;

                if (functionHash > 0xFFFFFFFF)
                    //functionHash = Convert.ToUInt32(functionHash - 0x100000000);
                    functionHash -= 0x100000000;
            }
            else
            {
                foreach (byte b in function)
                {
                    functionHash = ror(functionHash, 13, 32);
                    functionHash += b;
                }
            }

            return Convert.ToUInt32(functionHash);
        }
        
        public static byte[] ConvertToShellcode(in byte[] input, in Arch arch, string entryname, bool clearHeaders, bool ClearStage, ushort ObfuscateTime)
        {
            UInt32 flags = 0;
            if (clearHeaders)
                flags += 1;
            if (ClearStage)
                flags += 2;
            if (ObfuscateTime != 0)
            {
                flags += 4;
                flags += Convert.ToUInt32(ObfuscateTime) << 16;
            }

            byte[] loadercode = ResourceReader.ReadBinaryResource($"ShellcodeRDI_{arch}.bin");
            //byte[] loadercode = ResourceReader.ReadBinaryResource($"originalx64");
            using (var ms = new MemoryStream())
            {
                using (var bootstrap = new BinaryWriter(ms))
                {
                    if (arch == Arch.x64)
                    {
                        var bootstrapSize = 64;
                        bootstrap.Write(new byte[]{0xe8, 0,0,0,0});
                        UInt32 dllOffset = Convert.ToUInt32(bootstrapSize - 5 + loadercode.Length);
                        bootstrap.Write((byte)0x59);
                        bootstrap.Write(new byte[]{0x49,0x89,0xc8});
                        bootstrap.Write(new byte[]{0x48,0x81,0xc1});
                        bootstrap.Write(dllOffset);
                        bootstrap.Write((byte)0xba);
                        bootstrap.Write(HashFunctionName(entryname));
                        bootstrap.Write(new byte[]{0x49,0x81,0xc0});
                        UInt32 userdataLoc = dllOffset + Convert.ToUInt32(input.Length);
                        bootstrap.Write(userdataLoc);
                        bootstrap.Write(new byte[]{0x41,0xb9});
                        bootstrap.Write(Convert.ToUInt32(2)); //Will be two null bytes
                        //bootstrap.Write(Convert.ToUInt32(4));
                        bootstrap.Write(new byte[]
                        {
                            0x56,
                            0x48,0x89,0xe6,
                            0x48,0x83,0xe4,0xf0,
                            0x48,0x83,0xec,
                            0x30,
                            0xc7,0x44,0x24,
                            0x20
                        });
                        bootstrap.Write(flags); //Can update this to update flags, maybe take args later
                        bootstrap.Write((byte)0xe8);
                        bootstrap.Write(Convert.ToByte(bootstrapSize - bootstrap.BaseStream.Length - 4));
                        bootstrap.Write(new byte[]
                        {
                            0, 0, 0,
                            0x48, 0x89, 0xf4,
                            0x5e, 
                            0xc3
                        });
                        if (bootstrap.BaseStream.Length != bootstrapSize)
                            throw new InvalidProgramException("sRDI shellcode size mismatch(Report to dev)");
                        
                        bootstrap.Close();
                        var data = ms.ToArray();
                        return data.Concat(loadercode).Concat(input).Concat(new byte[] {0,0}).ToArray();

                    }
                    else if (arch == Arch.x86)
                    {
                        UInt32 bootstrapSize = 49;
                        UInt32 dllOffset = Convert.ToUInt32(bootstrapSize - 5 + loadercode.Length);
                        bootstrap.Write(new byte[]
                        {
                            0xe8, 00, 00, 00, 00,
                            0x58,
                            0x55,
                            0x89, 0xe5,
                            0x89, 0xc2,
                            0x05
                        });
                        bootstrap.Write(dllOffset);
                        bootstrap.Write(new byte[] {0x81, 0xc2});
                        UInt32 userDataLocation = dllOffset + Convert.ToUInt32(input.Length);
                        bootstrap.Write(userDataLocation);
                        bootstrap.Write((byte)0x68);
                        bootstrap.Write(flags);
                        bootstrap.Write(new byte[]
                        {
                            0x68,
                            0x02,0x00,0x00,0x00, //userdata len, hardcoded to 2 null bytes
                            0x52,
                            0x68
                        });
                        var a = HashFunctionName(entryname);
                        bootstrap.Write(a);
                        bootstrap.Write(new byte[]
                        {
                            0x50,
                            0xe8,
                        });
                        bootstrap.Write(Convert.ToByte(bootstrapSize - bootstrap.BaseStream.Length - 4));
                        bootstrap.Write(new byte[]
                        {
                            0, 0, 0,
                            0x83, 0xc4, 0x14,
                            0xc9,
                            0xc3
                        });


                        if (bootstrap.BaseStream.Length != bootstrapSize)
                            throw new InvalidProgramException("sRDI shellcode wrong size");
                        bootstrap.Close();
                        var data = ms.ToArray();
                        return data.Concat(loadercode).Concat(input).Concat(new byte[] {0, 0}).ToArray();

                    }
                    else
                    {
                        throw new InvalidProgramException("Invalid Arch given to convert shellcode");
                    }
                }
            }

            throw new InvalidProgramException("How did you get here?");
        }
        
        
    }
}