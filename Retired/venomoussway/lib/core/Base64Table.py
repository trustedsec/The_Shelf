#base64_charset = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
base64_charset = "/+9876543210zyxwvutsrqponmlkjihgfedcbaZYXWVUTSRQPONMLKJIHGFEDCBA"

dictio = {
    "000000": base64_charset[0],
    "000001": base64_charset[1],
    "000010": base64_charset[2],
    "000011": base64_charset[3],
    "000100": base64_charset[4],
    "000101": base64_charset[5],
    "000110": base64_charset[6],
    "000111": base64_charset[7],
    "001000": base64_charset[8],
    "001001": base64_charset[9],
    "001010": base64_charset[10],
    "001011": base64_charset[11],
    "001100": base64_charset[12],
    "001101": base64_charset[13],
    "001110": base64_charset[14],
    "001111": base64_charset[15],
    "010000": base64_charset[16],
    "010001": base64_charset[17],
    "010010": base64_charset[18],
    "010011": base64_charset[19],
    "010100": base64_charset[20],
    "010101": base64_charset[21],
    "010110": base64_charset[22],
    "010111": base64_charset[23],
    "011000": base64_charset[24],
    "011001": base64_charset[25],
    "011010": base64_charset[26],
    "011011": base64_charset[27],
    "011100": base64_charset[28],
    "011101": base64_charset[29],
    "011110": base64_charset[30],
    "011111": base64_charset[31],
    "100000": base64_charset[32],
    "100001": base64_charset[33],
    "100010": base64_charset[34],
    "100011": base64_charset[35],
    "100100": base64_charset[36],
    "100101": base64_charset[37],
    "100110": base64_charset[38],
    "100111": base64_charset[39],
    "101000": base64_charset[40],
    "101001": base64_charset[41],
    "101010": base64_charset[42],
    "101011": base64_charset[43],
    "101100": base64_charset[44],
    "101101": base64_charset[45],
    "101110": base64_charset[46],
    "101111": base64_charset[47],
    "110000": base64_charset[48],
    "110001": base64_charset[49],
    "110010": base64_charset[50],
    "110011": base64_charset[51],
    "110100": base64_charset[52],
    "110101": base64_charset[53],
    "110110": base64_charset[54],
    "110111": base64_charset[55],
    "111000": base64_charset[56],
    "111001": base64_charset[57],
    "111010": base64_charset[58],
    "111011": base64_charset[59],
    "111100": base64_charset[60],
    "111101": base64_charset[61],
    "111110": base64_charset[62],
    "111111": base64_charset[63]
}

switchDictio = {v: k for k, v in dictio.items()}
