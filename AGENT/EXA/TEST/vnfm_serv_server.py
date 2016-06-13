import sys
import BaseHTTPServer
import json


BODY_RESPONSE_FOR_READY = '''{
    "server" : {
      "created_at" : "2016-03-29 10:55:14",
      "name" : "vHP-PCRF-LB1",
      "server-uuid" : "52fe8307-4223-4918-a933-072cb76700c0"
   }
}'''

BODY_RESPONSE_FOR_SUBSCRIBER = '''{
   "subscriber" : {
      "callback_uri" : "http://127.0.0.1:9000/vnf/notification/lifecycle",
      "created_at" : "2016-03-29 13:55:02",
      "enabled_state" : "Enabled",
      "expiration_time" : 60,
      "id" : "x3g9cocs-stzv-1css-p7dc-yv395vz3oddb",
      "name" : "VNF",
      "notification_type" : "VNF Lifecycle Change",
      "server-uuid" : "52fe8307-4223-4918-a933-072cb76700c0"
   }
}'''

class ServiceHandler(BaseHTTPServer.BaseHTTPRequestHandler):
    def do_GET(self):
        print ''
        print '----- GET'
        self.send_response(200)

    def do_POST(self):
        print ''
        print '----- POST :', self.path
        len = int(self.headers['content-length'])
        print self.rfile.read(len)

        if 'ready' in self.path:
            self.make_response_for_ready()
        elif 'subscriber' in self.path:
            self.make_response_for_subscriber()
        else:
            self.send_response(200)

    def do_PUT(self):
        print ''
        print '----- PUT'
        self.send_response(200)


    def make_response_for_ready(self):
        self.send_response(202)
        self.send_header('Connection', 'close')
        self.send_header('Content-Length', str(len(BODY_RESPONSE_FOR_READY)))
        self.send_header('Content-Type', 'application/json')
        self.send_header('Date', self.date_time_string())
        self.end_headers()

        self.wfile.write(BODY_RESPONSE_FOR_READY)

    def make_response_for_subscriber(self):
        self.send_response(200)
        self.send_header('Connection', 'close')
        self.send_header('Content-Length', str(len(BODY_RESPONSE_FOR_SUBSCRIBER)))
        self.send_header('Content-Type', 'application/json')
        self.send_header('Date', self.date_time_string())
        self.end_headers()

        self.wfile.write(BODY_RESPONSE_FOR_SUBSCRIBER)

print '--[SERVE SERVER]----'

service = BaseHTTPServer.HTTPServer
httpd = service(('127.0.0.1', 19000), ServiceHandler)
httpd.serve_forever()