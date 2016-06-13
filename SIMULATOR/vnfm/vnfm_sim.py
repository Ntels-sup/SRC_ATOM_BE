#!/usr/bin/env python

#! -*- coding: utf-8 -*-

import sys
import threading
import BaseHTTPServer
import urllib2
import json
import socket
import random
import datetime
from select import select
import time
import pickle

class HttpData(object):
    def __init__(self):
        self.line = ''
        self.headers = {}
        self.body = ''

    def RecvHeader(self, _conn):
        self.headers = {}
        d = []

        while True:
            d.append(_conn.recv(1))

            if d[-1] == '\n':
                word4 = "".join(d[-4:])

                if word4 == '\r\n\r\n':
                    break

        l = "".join(d).split('\r\n')

        for word in l:
            temp = word.split(':')

            if len(temp) == 2:
                self.headers[temp[0].strip()] = temp[1].strip()

    def RecvBody(self, _conn, _len):
        self.body = _conn.recv(_len)

    def GetHeader(self, _field):
        return self.headers[_field]

    def Prn(self):
        print '---------- HEADER '
        for k in self.headers.keys():
            print '%-24s = %s' % (k, self.headers[k])

        print '---------- BODY  -'
        print self.body

    def GetBody(self):
        return self.body


    def MakeDefaultResponseData(self, _code, _code_msg, _body):
        line  = 'HTTP/1.1 %s %s' % (_code, _code_msg)

        headers= []
        headers.append('Connection: close')
        headers.append('Content-Length: %s' % len(_body))
        headers.append('Content-Type: application/json')

        raw_data = []
        raw_data.append(line)
        raw_data.append('\r\n')

        for line in headers:
            raw_data.append(line)
            raw_data.append('\r\n')

        raw_data.append('\r\n')
        raw_data.append(_body)

        return ''.join(raw_data)


    def MakeDefaultRequestData(self, _uri, _body):
        line = 'POST %s HTTP/1.1' % _uri

        headers= []
        headers.append('Content-Length: %s' % len(_body))
        headers.append('Content-Type: application/json')

        raw_data = []
        raw_data.append(line)
        raw_data.append('\r\n')

        for line in headers:
            raw_data.append(line)
            raw_data.append('\r\n')

        raw_data.append('\r\n')
        raw_data.append(_body)

        return ''.join(raw_data)

class HttpClient(object):
    def __init__(self, _callback_uri, _recvServer):

        host_and_port = _callback_uri[:_callback_uri.find('/vnf/action')]

        self.host = host_and_port[host_and_port.find('//')+2:host_and_port.rfind(':')]
        self.port = host_and_port[host_and_port.rfind(':')+1:]

        self.recvServer = _recvServer

    def Send(self, _data):

        conn = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        conn.connect((self.host, int(self.port)))
        sendn = conn.send(_data)

        print '===== SEND ='
        print _data
        print '=============='
        print

        self.recvServer(conn)
        conn.close()

class Vnfm(object):
    def __init__(self):
        self.map_listen_fd = {}
        self.httpData      = HttpData()

        self.dic_uuid      = {}

    def StartServer(self, _port):
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        s.bind(('', _port))
        s.listen(5)
        self.map_listen_fd[_port] = s

    def Do(self):
        self.doCmdServer()
        self.doNotiServer()

    def Send(self, _conn, _data):

        sendn = _conn.send(_data)

        print '===== SEND ='
        print _data
        print '=============='
        print

        _conn.close()

    def doCmdServer(self):
        r, w, e = select([self.map_listen_fd[19000]], [], [], 0)

        for fd in r:
            if fd == self.map_listen_fd[19000]:
                conn, addr = fd.accept()
                print addr

                self.cmdServer(conn)
                break

    def cmdServer(self, _conn):

        self.recvServer(_conn)

        if 'ready' in self.httpData.GetBody():

            js = json.loads(self.httpData.GetBody())
            client = HttpClient(js['ready']['action-uri'], self.recvServer)

            uuid = ''

            try:
                uuid = self.getUUID(js['ready']['action-uri'])
            except:
                uuid = ''
                print '----- DEBUG fail'

            if not uuid:
                print '---------  DEBUG uuid is ', uuid
                uuid = self.generateUUID(js['ready']['action-uri'])
                data = self.makeReadyResponse(uuid)
                self.Send(_conn, data)

                client.Send(self.makeInstallRequest(uuid))
            else:
                print '---------  DEBUG uuid finds ', uuid
                data = self.makeReadyResponse(uuid)
                self.Send(_conn, data)

                client.Send(self.makeStartRequest(uuid))

        else:
            data = self.httpData.MakeDefaultResponseData('200', 'Ok', '')
            self.Send(_conn, data)

    def makeReadyResponse(self, _uuid):
        templ = '''{
            "server" : {
                "created_at" : "%s",
                "name" : "Test-by-Vnfm-Sim",
                "server-uuid" : "%s"
           }
        }'''

        body = templ % (datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
                       _uuid)

        http_data = HttpData()
        return http_data.MakeDefaultResponseData('202', 'Accept', body)


    def generateUUID(self, _uri):
        source='0123456789-abcdef'
        uuid = ''.join(random.choice(source) for i in range(32))

        self.dic_uuid[_uri] = uuid
    
        with open('./uuid.dat', 'w') as f:    
            pickle.dump(self.dic_uuid, f)

    def getUUID(self, _uri):
        self.dic_uuid = {}
        with open('./uuid.dat', 'r') as f:
            self.dic_uuid = pickle.load(f)

        return self.dic_uuid[_uri]

    def makeInstallRequest(self, _uuid):

        templ = '''{
            "install" : {
                "details" : {
                    "version": "%s",
                    "download_path": "%s",
                    "checksum": "%s"
                },
                "server-uuid" : "%s",
                "timestamp" : "%s"
            }
        }'''

        body = templ % ('v1.0.0.0',
                        'ftp://192.168.10.5/PKG/vOFCS/vOFCS_1.0.0.0.tar.gz',
                        '0123456789',
                        _uuid,
                        datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"))

        http_data = HttpData()

        return http_data.MakeDefaultRequestData('/vnf/action/install', body)

    def makeStartRequest(self, _uuid):

        templ = '''{
            "start" : {
                "server-uuid" : "%s",
                "timestamp" : "%s"
            }
        }'''

        body = templ % (_uuid,
                        datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"))

        http_data = HttpData()

        return http_data.MakeDefaultRequestData('/vnf/action/start', body)


    def doNotiServer(self):
        r, w, e = select([self.map_listen_fd[29000]], [], [], 0)

        for fd in r:
            if fd == self.map_listen_fd[29000]:
                conn, addr = fd.accept()
                print addr

                self.recvServer(conn)

                data = self.httpData.MakeDefaultResponseData('200', 'Ok', '')
                self.Send(conn, data)
                break

    def recvServer(self, conn):
        print '===== RECV ='

        self.httpData.RecvHeader(conn)

        if int(self.httpData.GetHeader('Content-Length')) > 0:
            self.httpData.RecvBody(conn,
                                   int(self.httpData.GetHeader('Content-Length')))

        self.httpData.Prn()
        print '=============='
        print 


def help():
    print 'Invalid argument'
    print ' ex) vnfm_sim.py'


if __name__ == '__main__':

    if len(sys.argv) != 1:
        help()
        sys.exit()

    inst    = Vnfm()

    inst.StartServer(19000)
    inst.StartServer(29000)

    while True:
       inst.Do()
       time.sleep(0.1)

    sys.exit()
