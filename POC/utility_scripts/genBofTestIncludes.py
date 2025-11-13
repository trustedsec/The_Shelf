import sys

input = sys.argv[1]

with open(input) as fp:
    original = fp.read().split("#ifdef BOF")[1].split("#else")[0]

lines = original.split('\n')

imports = []
for line in lines:
    token = line.find('$')
    if token != -1:
        start = line.rfind(' ', 0, token)
        end = line.find('(', token)
        imports.append(line[start+1:end])

prelude = '''#define intAlloc(size) KERNEL32$HeapAlloc(KERNEL32$GetProcessHeap(), HEAP_ZERO_MEMORY, size)
#define intRealloc(ptr, size) (ptr) ? KERNEL32$HeapReAlloc(KERNEL32$GetProcessHeap(), HEAP_ZERO_MEMORY, ptr, size) : KERNEL32$HeapAlloc(KERNEL32$GetProcessHeap(), HEAP_ZERO_MEMORY, size)
#define intFree(addr) KERNEL32$HeapFree(KERNEL32$GetProcessHeap(), 0, addr)
#define intZeroMemory(addr,size) MSVCRT$memset((addr),0,size)
'''

print(prelude)
for i in imports:
    print("#define {} {}".format(i, i.split('$')[1]))

print('#define BeaconPrintf(x, y, ...) printf(y, ##__VA_ARGS__)')
print('#define internal_printf printf')


