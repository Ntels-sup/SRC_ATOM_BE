import sys
import BaseHTTPServer
import json

class PerfHandler(BaseHTTPServer.BaseHTTPRequestHandler):
    def do_GET(self):
        print ''
        print '----- GET'
        self.send_response(200)

    def do_POST(self):
        print ''
        print '----- POST :', self.path
        len = int(self.headers['content-length'])
        print self.rfile.read(len)


        self.send_response(200)

    def do_PUT(self):
        print ''
        print '----- PUT'
        self.send_response(200)

print '--[PERFORMANCE SERVER]----'

service = BaseHTTPServer.HTTPServer
httpd = service(('127.0.0.1', 29000), PerfHandler)
httpd.serve_forever()