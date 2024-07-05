# Kerberos Dump
This is just a simplified version of the following but written in C and 
and runs on both mac and linux.

https://github.com/its-a-feature/bifrost


## Notable Differences
- Instead of requiring an ASN.1 Library I have an function that will generate 
a binary ccache ticket encoded in base64 to stdout. 

## Using tickets in other tools
To use the tickets in other tools convert the tickets with this.

- https://github.com/Zer1t0/ticket_converter.git
