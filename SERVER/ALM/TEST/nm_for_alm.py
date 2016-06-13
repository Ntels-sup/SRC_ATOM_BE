
# -- coding:utf-8 --

import sys
import socket
import struct
import time

from select import select

class NodeManager(object):
    def __init__(self):
        self.ip             = '127.0.0.1'
        self.port           = 5000
        self.fmt_header = '>1B10s1Biiiiii'

        self.server         = None
        self.sequence_id    = 0

        self.fd_list        = []
        self.input_fd       = 0
        self.fd_list.append(self.input_fd)

        self.client_list    = []


    def start(self):
        self.server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

        self.server.bind((self.ip, self.port))
        self.server.listen(5)

        self.fd_list.append(self.server.fileno())

    def Do(self):
        while True:
            r, w, e = select(self.fd_list, [], [], 3)

            for conn in r:
                if conn == self.server.fileno():
                    client, _ = self.server.accept()

                    self.fd_list.append(client.fileno())
                    self.client_list.append(client)
                    print 'Accept Success'
                elif conn == self.input_fd:
                    input = sys.stdin.readline()
                    self.proc_cmd(int(input[:-1]))
                else:
                    print 'another read'
                    for client in self.client_list:
                        if client.fileno() == conn:
                            buf = client.recv(1024)

                            if not buf:
                                self.fd_list.remove(client.fileno())
                                client.close()
                                self.client_list.remove(client)
                                break

                            print 'recv : ', len(buf)
                            print buf

                print self.fd_list

            print '-- SERVER .... ing'

    def stop(self):

        if not self.server:
            self.server.close()

        print '-- SERVER STOP'


    def make_header(self, _command, _flag, _body_len):

        self.sequence_id += 1

        H = struct.pack(self.fmt_header,
                        0x01,
                        str(_command).zfill(10),
                        _flag,
                        2,
                        20,
                        1,
                        12,
                        self.sequence_id,
                        _body_len+36)

        return H

    def make_body(self, _file_name):

        body = ''

        with open(_file_name,'r') as f:
            while True:
                line = f.readline()

                if not line:
                    break

                body += line

        #print body
        #print len(body)
        #time.sleep(10)
        return body


    def proc_cmd(self, _cmd):
        # SEND 는 모두 Flag 가 0x20 이예요, Request 니까요.
        print '[%s]' % _cmd

        H = None
        D = ''

        if _cmd == 1 :
            D = self.make_body('Alarm.txt')
            H = self.make_header(20000, 0x40, len(D))
            self.client_list[0].sendall(H + D)
        if _cmd == 2 :
            D = self.make_body('User.txt')
            H = self.make_header(20007, 0x10, len(D))
            self.client_list[0].sendall(H + D)
        if _cmd == 3 :
            D = self.make_body('Tca.txt')
            H = self.make_header(20001, 0x40, len(D))
            self.client_list[0].sendall(H + D)
        if _cmd == 4 :
            D = self.make_body('Node.txt')
            H = self.make_header(20002, 0x40, len(D))
            self.client_list[0].sendall(H + D)
        else :
            pass

        help()



def help():
    print '---------------------------------'
    print 'ex) CODE_NUMBER'
    print '    - 1 : alarm message'
    print '    - 2 : user clear alarm message'
    print '    - 3 : tca alarm message'
    print '    - 4 : node status message'

if __name__ == '__main__':

    s = NodeManager()
    s.start()
    s.Do()
    s.stop()
