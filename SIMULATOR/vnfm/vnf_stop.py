#!/usr/bin/env python

import sys
import json
import socket
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
    def __init__(self, _host, _port):

        self.host = _host
        self.port = _port

        self.httpData = HttpData() 

    def Send(self, _data):

        conn = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        conn.connect((self.host, int(self.port)))
        sendn = conn.send(_data)

        print '===== SEND ='
        print _data
        print '=============='
        print

        self.RecvResponse(conn)
        conn.close()

    def MakeStopRequest(self, _uuid):

        templ = '''{
            "stop" : {
                "server-uuid" : "%s",
                "timestamp" : "%s"
            }
        }'''


        body = templ % (_uuid,
                        datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"))

        return self.httpData.MakeDefaultRequestData('/vnf/action/stop', body)

    def RecvResponse(self, _conn):
        print '===== RECV ='

        self.httpData.RecvHeader(_conn)

        if int(self.httpData.GetHeader('Content-Length')) > 0:
            self.httpData.RecvBody(conn,
                                   int(self.httpData.GetHeader('Content-Length')))

        self.httpData.Prn()
        print '=============='
        print


def help():
    print 'Invalid argument'
    print ' ex) vnf_stop.py [ip] [port]'

def getUUID(_host, _port):
    uri = 'http://%s:%s/vnf/action' % (_host, _port)

    dic_uuid = {}
    with open('./uuid.dat', 'r') as f:
        dic_uuid = pickle.load(f)

    return dic_uuid[uri]    


if __name__ == '__main__':

    if len(sys.argv) != 3:
        help()
        sys.exit()

    uuid = getUUID(sys.argv[1], sys.argv[2])

    client = HttpClient(sys.argv[1], sys.argv[2])

    data = client.MakeStopRequest(uuid)    

    client.Send(data)

