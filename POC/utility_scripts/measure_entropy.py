import os
import sys
import math
#GPL'ed from Revelation code base, and stackoverflow.

def entropy(string):
        "Calculates the Shannon entropy of a string"

        # get probability of chars in string
        prob = [ float(string.count(c)) / len(string) for c in dict.fromkeys(list(string)) ]

        # calculate the entropy
        entropy = - sum([ p * math.log(p) / math.log(2.0) for p in prob ])

        return entropy

if __name__ == "__main__":
    fin = open(sys.argv[1], "rb")
    data = fin.read()
    fin.close()
    print(entropy(data))
