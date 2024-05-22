#!/usr/bin/env python

import optparse
import base64
import io
import binascii

def BinaryToVbaFunc(input_binary, output_function_name, encoding={}):

    data = ''
    countLine = 0
    countSubs = 1
    line = ''
    input_hex = ''

    
    if len(input_binary) == 0:
        print(" [*] ERROR: input_binary is empty")
        return data
    
               
    input_hex = binascii.hexlify(input_binary).decode("utf-8")

    outputString = io.StringIO()
    
    for key in encoding:
        input_hex = input_hex.replace(key, encoding[key])
    
    # Iterate through the input_binary buffer and 
    #  generate a sequence of sub-functions which 
    #  return substrings of the overall input_binary buffer
    # Each line of the VBA code is limited to 80 characters
    # Each sub-function is limited to 99 lines
    outputString.write('Private Function %s_%d() As String\n' % (output_function_name, countSubs))
    outputString.write('\tDim szBinary As String\n')
    outputString.write('\n')
    outputString.write('\tszBinary = ""\n')
        
    input_hex_lines = [ input_hex[i:i+64] for i in range(0, len(input_hex), 64) ]
    
    for line in input_hex_lines:
        outputString.write('\tszBinary = szBinary & "%s"\n' % line)
        countLine += 1
        if countLine > 99:
            countLine = 0
            outputString.write('\n')
            outputString.write('\t%s_%d = szBinary\n' % (output_function_name, countSubs))
            outputString.write('End Function\n')
            outputString.write('\n')
            countSubs += 1
            outputString.write('Private Function %s_%d() As String\n' % (output_function_name, countSubs))
            outputString.write('\tDim szBinary As String\n')
            outputString.write('\n')
            outputString.write('\tszBinary = ""\n')

    outputString.write('\n')
    outputString.write('\t%s_%d = szBinary\n' % (output_function_name, countSubs))
    outputString.write('End Function\n')
    outputString.write('\n')
    countSubs += 1
        

    # Create the overall input_binary buffer by
    #  concatenating the sub-strings returned by
    #  all the sub-functions
    outputString.write('Private Function %s() As Byte()\n' % output_function_name)
    outputString.write('\tDim objXmlDom As Object\n')
    outputString.write('\tDim objNode As Object\n')
    outputString.write('\tDim szBinary As String\n')
    outputString.write('\n')
    outputString.write('\tszBinary = ""\n')
    for iIter in range(1, countSubs):
        outputString.write('\tszBinary = szBinary & %s_%d()\n' % (output_function_name, iIter))
    outputString.write('\n')
    for key in encoding:
        outputString.write('\tszBinary = Replace(szBinary, "%s", "%s")\n' % (encoding[key], key))
    
    outputString.write('\n')
    outputString.write('\tSet objXmlDom = CreateObject("Microsoft.XMLDOM")\n')
    outputString.write('\tSet objNode = objXmlDom.createElement("tmp")\n')
    outputString.write('\tobjNode.DataType = "bin.hex"\n')
    outputString.write('\tobjNode.Text = szBinary\n')
    outputString.write('\n')
    outputString.write('\t%s = objNode.NodeTypedValue\n' % output_function_name)
    outputString.write('End Function\n')
    data = outputString.getvalue()
    outputString.close()
    return data

def Main():
    oParser = optparse.OptionParser(usage='usage: %prog [options] infile outfile\n' )
    oParser.add_option('-e', '--encoding',help='encodings')
    (options, args) = oParser.parse_args()
    
    with open( args[0], "rb" ) as fd:
        input_binary = fd.read()

    encodings = { "0": "-", "6": "!" }
    
    output_vba = BinaryToVbaFunc(input_binary, "BinaryToVbaFunc", encodings )
    
    with open( args[1], "w" ) as fd:
        fd.write(output_vba)
    
if __name__ == '__main__':
    Main()