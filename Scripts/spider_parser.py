#!/usr/bin/env -S uv run python3

import glob
import json
import re
import argparse

# Preset regex patterns
PRESETS = {
    'passwords': r'password|passwd|pwd|pass\.txt|credentials|creds|secret',
    'configs': r'\.config|\.conf|\.ini|\.xml|\.yaml|\.yml|settings',
    'keys': r'\.key|\.pem|\.pfx|\.p12|\.crt|\.cer|id_rsa|private',
    'scripts': r'\.ps1|\.bat|\.cmd|\.vbs|\.sh',
    'sensitive': r'password|secret|key|token|api|credential|private|confidential',
    'database': r'\.sql|\.db|\.mdb|\.accdb|\.sqlite|\.csv|database|backup\.bak',
    'backup': r'\.bak|\.backup|\.old|\.tmp|\.swp|\.csv|~',
    'code': r'\.py|\.java|\.cpp|\.c|\.cs|\.js|\.php|\.rb|\.go',
    'documents': r'\.doc|\.docx|\.xls|\.xlsx|\.pdf|\.txt|\.rtf|\.odt',
    'web': r'\.html|\.htm|\.asp|\.aspx|\.jsp|\.php',
    'financial': r'invoice|receipt|payment|budget|financial|accounting|payroll|salary|tax|revenue|expense|profit|loss|balance|statement|quickbooks|\.qb[owx]|sage|xero',
}

def load_exclusions(exclude_file):
    """Load exclusion list from file. Format: hostname or hostname,share (one per line)"""
    exclusions = {'hosts': set(), 'shares': set()}
    try:
        with open(exclude_file, 'r') as f:
            for line in f:
                line = line.strip()
                if line and not line.startswith('#'):  # Skip empty lines and comments
                    if ',' in line:
                        host, share = line.split(',', 1)
                        exclusions['shares'].add((host.strip(), share.strip()))
                    else:
                        exclusions['hosts'].add(line)
    except FileNotFoundError:
        pass  # No exclusion file is fine
    return exclusions

if __name__ == '__main__':
    # banner()
    parser = argparse.ArgumentParser(description='', epilog=f'Available presets: {", ".join(PRESETS.keys())}')
    parser.add_argument('-d','--dir', type=str, default='/root/.nxc/modules/nxc_spider_plus/', help='Directory of nxc spider plus. Default is /root/.nxc/modules/nxc_spider_plus/')
    parser.add_argument('REGEX', type=str, nargs='?', help='Regex pattern or preset name to search for')

    parser.add_argument('-m','--mount',default=False, action=argparse.BooleanOptionalAction, help='Display commands necessary to create mount shares')
    parser.add_argument('-u','--username', help='Username for mount')
    parser.add_argument('-p','--password', help='password for mount')
    parser.add_argument('--include-sysvol',default=False, action='store_true', help='Include SYSVOL share (ignored by default)')
    parser.add_argument('--list-presets', action='store_true', help='List all available regex presets')
    parser.add_argument('-e','--exclude', type=str, help='Path to exclusion file (format: hostname or hostname,share per line)')
    parser.add_argument('-H','--host', type=str, action='append', help='Only include specific host(s). Can be used multiple times')
    args = parser.parse_args()

    # Handle list-presets
    if args.list_presets:
        print("Available regex presets:")
        for name, pattern in sorted(PRESETS.items()):
            print(f"  {name:15} - {pattern}")
        exit(0)

    # Require REGEX if not listing presets
    if not args.REGEX:
        parser.error("REGEX argument is required")

    # Load exclusions
    exclusions = {'hosts': set(), 'shares': set()}
    if args.exclude:
        exclusions = load_exclusions(args.exclude)

    #list all files in directory
    files = glob.glob(args.dir+'*.json')

    #Compile regex pattern - check if it's a preset first
    if args.REGEX in PRESETS:
        regex_pattern = re.compile(PRESETS[args.REGEX], re.IGNORECASE)
    else:
        regex_pattern = re.compile(args.REGEX)

    # Track shares with matches for mount command generation
    matched_shares = set()

    for file_path in files:
        file = file_path.split('/')[-1:][0]
        hostname = '.'.join(file.split('.')[:-1])

        # Skip if not in include list (if specified)
        if args.host and hostname not in args.host:
            continue

        # Skip excluded hosts
        if hostname in exclusions['hosts']:
            continue

        with open(file_path) as open_file:
            try:
                a = json.load(open_file)
            except json.decoder.JSONDecodeError:
                pass
            for key in a.keys():
                # Skip excluded shares
                if (hostname, key) in exclusions['shares']:
                    continue
                #If share is IPC$ we can skip it, also skip SYSVOL unless --include-sysvol is set
                if key != 'IPC$' and (args.include_sysvol or key != 'SYSVOL'):
                    #if share is not IPC$ then iterate through the items in the share.
                    for item in a[key]:
                        if re.search(regex_pattern, item):
                            print(f'{hostname},{key},{item}')
                            matched_shares.add((hostname, key))
    if args.mount:
        for hostname, share in sorted(matched_shares):
            escaped_share = re.sub('\$','\\\$', share) #escape the dollar sign.
            print(f'mkdir -p /mnt/{hostname}/{escaped_share}')
            print(f'mount -t cifs //{hostname}/{escaped_share} /mnt/{hostname}/{escaped_share} -o username=\'{args.username}\',password=\'{args.password}\'')





