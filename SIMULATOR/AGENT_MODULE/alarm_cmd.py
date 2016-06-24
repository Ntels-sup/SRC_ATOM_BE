# -- coding:utf-8 --

import sys
import socket
import struct
import time

from select import select

class Command(object):
    def __init__(self):
        self.ip      = '127.0.0.1'
        self.port    =  20000

        self.stdin_fd= 0
        self.fd_list = []
        self.fd_list.append(self.stdin_fd)

    def start(self):
        self.client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        ret = self.client.connect((self.ip, self.port))
        self.fd_list.append(self.client.fileno())
        print '--- start ', ret


    def Do(self):
        while True:
            r, w, e = select(self.fd_list, [], [], 3)

            for conn in r:
                if conn == self.stdin_fd:
                    input = sys.stdin.readline()
                    self.proc_cmd(int(input[:-1]))
                if conn == self.client.fileno():
                    self.fd_list.remove(self.client.fileno())
                    self.client.close()
                    self.start()

    def proc_cmd(self, _cmd):
        if _cmd == 1 :
            D = self.make_alarm()
            H = str(len(D)).zfill(4)
            ret = self.client.sendall(H + D)
            print '-- send Alarm -', D
        elif _cmd == 2:
            D = self.make_process()
            H = str(len(D)).zfill(4)
            ret = self.client.sendall(H + D)
            print '-- send Process -', D
        elif _cmd == 3:
            D = self.make_ping()
            H = str(len(D)).zfill(4)
            ret = self.client.sendall(H + D)
            print '-- send Ping - ', D
        elif _cmd == 4:
            D = self.make_connect()
            H = str(len(D)).zfill(4)
            ret = self.client.sendall(H + D)
            print '-- send Connect - ', D
        elif _cmd == 5:
            for i in range(100):
                D = self.make_auto_alarm(i)
                H = str(len(D)).zfill(4)
                ret = self.client.sendall(H + D)
                print '-- send Alarm -', D
        else:
            help()

    def read_file(self, _fname):
        body = ''

        with open(_fname,'r') as f:
            while True:
                line = f.readline()

                if not line:
                    break

                body += line

        return body

    def make_alarm(self):

        return self.read_file('alarm.txt')
        '''
        data = ''

        data += 'CODE:90300250,'
        data += 'TARGET:eth5,'
        data += 'VALUE:DOWN,'
        data += 'COMPLEMENT:,'
        data += 'TEXT:,'

        return data
        '''

    def make_auto_alarm(self, index):

        data = ''

        data += 'CODE:90300250,'
        data += 'TARGET:%s,' % index
        data += 'VALUE:DOWN,'
        data += 'COMPLEMENT:,'
        data += 'TEXT:,'

        return data


    def make_process(self):

        return self.read_file('process.txt')

        '''
        data = ''

        data += 'WORST_STATUS:ABNORMAL,'
        data += 'PROC_NO:49,'
        data += 'PROC_NAME:STM,'
        data += 'PROC_STATUS:SUSPEND,'

        return data
        '''


    def make_ping(self):

        return self.read_file('ping.txt')

        '''
        data = ''

        data += 'PEER_NODE:2,'
        data += 'MY_IP:192.168.1.194,'
        data += 'PEER_IP:192.168.1.193,'
        data += 'STATUS:FAIL,'

        return data
        '''

    def make_connect(self):

        return self.read_file('connect.txt')

        '''
        data = ''

        data += 'MY_IP:192.168.1.194,'
        data += 'PEER_IP:192.168.1.193,'
        data += 'PORT:3456,'
        data += 'STATUS:CONNECT,'

        return data
        '''

def help():
    print '---------------------------------'
    print 'ex) CODE_NUMBER'
    print '    - 1 : alarm message'
    print '    - 2 : process 상태'
    print '    - 3 : ping 상태'
    print '    - 4 : Connect 상태'
    print '    - 5 : alarm 100 발생 상태'

if __name__ == '__main__':
    help()

    s = Command()
    s.start()
    s.Do()
