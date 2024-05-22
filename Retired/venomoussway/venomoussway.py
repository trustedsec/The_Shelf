import sys
import platform
import os
sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), "lib/obfuscator"))
from lib.menus.main_loop import MainMenu
from importlib import import_module
from lib.core.helpers import Helpers
import argparse
import traceback

#TODO: from io import StringIO, if using automation, redirect standardout to given string buffer
# Should provide functions to retreive and reset this buffer


if platform.system() == 'Darwin':
    import readline
    import rlcompleter
    if 'libedit' in readline.__doc__:
        readline.parse_and_bind("bind ^I rl_complete")
    else:
        readline.parse_and_bind("tab: complete")
    readline.set_completer_delims(' \t\n')
elif platform.system() == 'Windows':
    import pyreadline as readline
else:
    import readline
    readline.set_completer_delims(' \t\n')


def banner():
    print("""
 ▄▀▀▄ ▄▀▀▄  ▄▀▀█▄▄▄▄  ▄▀▀▄ ▀▄  ▄▀▀▀▀▄   ▄▀▀▄ ▄▀▄  ▄▀▀▀▀▄   ▄▀▀▄ ▄▀▀▄  ▄▀▀▀▀▄ 
█   █    █ ▐  ▄▀   ▐ █  █ █ █ █      █ █  █ ▀  █ █      █ █   █    █ █ █   ▐ 
▐  █    █    █▄▄▄▄▄  ▐  █  ▀█ █      █ ▐  █    █ █      █ ▐  █    █     ▀▄   
   █   ▄▀    █    ▌    █   █  ▀▄    ▄▀   █    █  ▀▄    ▄▀   █    █   ▀▄   █  
    ▀▄▀     ▄▀▄▄▄▄   ▄▀   █     ▀▀▀▀   ▄▀   ▄▀     ▀▀▀▀      ▀▄▄▄▄▀   █▀▀▀   
            █    ▐   █    ▐            █    █                         ▐      
            ▐        ▐                 ▐    ▐                                
 ▄▀▀▀▀▄  ▄▀▀▄    ▄▀▀▄  ▄▀▀█▄   ▄▀▀▄ ▀▀▄                                      
█ █   ▐ █   █    ▐  █ ▐ ▄▀ ▀▄ █   ▀▄ ▄▀                                      
   ▀▄   ▐  █        █   █▄▄▄█ ▐     █                                        
▀▄   █    █   ▄    █   ▄▀   █       █                                        
 █▀▀▀      ▀▄▀ ▀▄ ▄▀  █   ▄▀      ▄▀                                         
 ▐               ▀    ▐   ▐       █                                          
                                  ▐          
""")


if __name__ == "__main__":

    banner()
    
    #Load modules/bypasses/template
    main_helpers = Helpers()
    main_helpers.load_templates("./modules", 'module')
    main_helpers.load_templates("./bypasses", 'bypass')
    main_helpers.load_templates("./templates", 'template')
    
    #Parse command-line arguments
    parser = argparse.ArgumentParser(description="VBA Generation Framework")
    parser.add_argument('-r', type=argparse.FileType('r'), default=None, help="rcfile with list of commands to execute in order")
    parser.add_argument('--debug', action='store_true', default=False, help="flag for outputting debug information")
    args = parser.parse_args()

    #Load rcfile if specified
    if args.r is not None:
        precommands = args.r.readlines()
        main_helpers.rccommands = [c.strip('\n') for c in precommands]

    menu = MainMenu(main_helpers, args.debug)
    menu.prompt = 'VenomousSway> '
    try:
        menu.cmdloop()
    except KeyboardInterrupt as msg:
        print('ctrl-C sent, exiting framework')
        sys.exit(0)
    # accept config options if provided, only accept full config -> this will need its own entrypoint
    # call into mainloop if options not provided
