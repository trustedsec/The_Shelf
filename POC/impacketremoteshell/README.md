# impacketremoteshell

This project was born of the idea that if smbexec, wmiexec, atexec etc all are able to remotely call functions and redirect on relativly hardened network why not just install a legit application and interface with it over smb.  Then we can execute commands and get output back w/o the signature of cmd.exe / powershell.exe being called or the redirection typically used by those techniques.

Well after spending a few days developing it I tested it and it was immediatly flagged as inbound C2 tool transfer by multiple EDR solutions.  So... TLDR that was a fun thought experiment :)

I've included the base impacket .py file (remoteshell.py) but I have not included our internal_helpers we've extended impacket with. If you want to use this yourself you'll need to replace those calls or develop the subset of that interface used in this example.

The code here is our own, and impacket from this perspective is simply used as a dependency, so this code itself is MIT licensed.