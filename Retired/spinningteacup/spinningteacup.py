from randomizers.vbarandomizer import vbaRandomizer
import sys
import argparse
import os

DEBUG = True

def banner():
    print(
    '''                        
 (`-').->_  (`-') _    <-. (`-')_<-. (`-')_ _    <-. (`-')_              (`-')     (`-')  (`-')  _                    _  (`-') 
 ( OO)_  \-.(OO )(_)      \( OO) )  \( OO) (_)      \( OO) )   .->       ( OO).->  ( OO).-(OO ).-/ _            .->   \-.(OO ) 
(_)--\_) _.'    \,-(`-',--./ ,--/,--./ ,--/,-(`-',--./ ,--/ ,---(`-')    /    '._ (,------/ ,---.  \-,-----,--.(,--.  _.'    \ 
/    _ /(_...--''| ( OO|   \ |  ||   \ |  || ( OO|   \ |  |'  .-(OO )    |'--...__)|  .---| \ /`.\  |  .--.|  | |(`-'(_...--'' 
\_..`--.|  |_.' ||  |  |  . '|  ||  . '|  ||  |  |  . '|  ||  | .-, \    `--.  .--(|  '--.'-'|_.' |/_) (`-'|  | |(OO |  |_.' | 
.-._)   |  .___.(|  |_/|  |\    ||  |\    (|  |_/|  |\    ||  | '.(_/       |  |   |  .--(|  .-.  |||  |OO |  | | |  |  .___.' 
\       |  |     |  |'-|  | \   ||  | \   ||  |'-|  | \   ||  '-'  |        |  |   |  `---|  | |  (_'  '--'\  '-'(_ .|  |      
 `-----'`--'     `--'  `--'  `--'`--'  `--'`--'  `--'  `--' `-----'         `--'   `------`--' `--'  `-----'`-----'  `--'                       
 '''
    )

def verifyfilepath(path):
    if os.access(path, os.R_OK):
        return path
    else:
        raise Exception("can not read path {}".format(path))

def main(argv):
    banner()
    parser = argparse.ArgumentParser(description="This is a macro obfuscating framework")
    globalflags = argparse.ArgumentParser(add_help=False)
    globalflags.add_argument("source", type=argparse.FileType("r"), help="Source script you would like to obfuscate")
    globalflags.add_argument("dest", type=argparse.FileType("w"), help="Where to output the script after obfuscation")
    supportedcommands = parser.add_subparsers(title='Supported Languages', dest='command')
    vba = supportedcommands.add_parser('vba', parents=[globalflags])
    vrtitle = vba.add_argument_group(title= 'VBA Variable and function randomization methods')
    vba_variable_replacement = vrtitle.add_mutually_exclusive_group(required=True)
    vba_variable_replacement.add_argument('--usebusinesswords', action='store_true', help="uses a set of precanned business related words")
    vba_variable_replacement.add_argument('--wordlistpath', type=verifyfilepath, default = None, metavar="path/to/wordlist", help="path to csv or newline separated wordlist" )
    vba_variable_replacement.add_argument('--norandomvariables', action='store_true', help="don't randomize variable or function names")
    inttitle = vba.add_argument_group(title="VBA Integer randomizations methods")
    vba_numeric_replacement = inttitle.add_mutually_exclusive_group(required=True)
    vba_numeric_replacement.add_argument('--math', action='store_true', help="randomize constant numbers using math")
    vba_numeric_replacement.add_argument('--norandomint', action='store_true', help='don\'t randomize constant numbers')
    strtitle = vba.add_argument_group(title="VBA string randomization methods")
    vba_string_replacement = strtitle.add_mutually_exclusive_group(required=True)
    vba_string_replacement.add_argument('--randomcuts', type=int, nargs=2, default=None, metavar=('mincut', 'maxcut'), help='use random string concatination, requires minlen maxlen values')
    vba_string_replacement.add_argument('--encodestring', action='store_true', help="encode strings using a rot19 function")
    vba_string_replacement.add_argument('--encodestring_calls', action='store_true', help="encode strings using a rot19 function\
                                                                                          Then Call them via a unique function name")
    vba_string_replacement.add_argument('--norandomstring', action='store_true', help='don\'t randomize constant numbers')

    args = parser.parse_args()
    data = args.source.read()
    vararg = {}
    intarg = {}
    strarg = {}
    #variable
    if args.usebusinesswords == True:
        vartype = 'businesswords'
    elif args.wordlistpath is not None:
        vartype = 'wordlist'
        vararg = {"wordlistpath":args.wordlistpath}
    else:
        vartype = "none"
    #int
    if args.math == True:
        inttype = 'math'
    else:
        inttype = 'none'
    # string
    if args.randomcuts is not None:
        strtype = "cuts"
        strarg = {"mincut":args.randomcuts[0], "maxcut":args.randomcuts[1]}
    elif args.encodestring:
        strtype = "encode"
    elif args.encodestring_calls:
        strtype= "encode_funcs"
    else:
        strtype = "none"

    randomizer = vbaRandomizer(data)
    randomizer.set_numeric_randomizer(inttype, **intarg)
    randomizer.set_string_randomizer(strtype, **strarg)
    randomizer.set_variable_randomizer(vartype, **vararg)
    obfuscated = randomizer.randomize()
    print("your script has been obfuscated and output to {}".format(args.dest.name))
    args.dest.write(obfuscated)






if __name__ == '__main__':
    main(sys.argv)