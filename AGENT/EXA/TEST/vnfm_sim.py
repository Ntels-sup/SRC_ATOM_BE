#! -*- coding: utf-8 -*-

import sys
import threading
import BaseHTTPServer
import urllib2
import json



class Client(object):
    def __init__(self, info):
        self.server_ip        = info[0]
        self.server_port      = info[1]

    def help(self):
        print ''
        print '----------------------'
        print 'Command:'
        print ' install'
        print ' stop'
        print ' start'
        print ' lifecycle'
        print ' exit'

    def do(self):
        while(True):
            self.help()
            val = raw_input("")

            if val == 'install':
                self.install()
            elif val == 'stop':
                self.stop()
            elif val == 'start':
                self.start()
            elif val == 'lifecycle':
                # self.lifecycle()
                pass
            elif val == 'exit':
                self.exit()
            else:
                print 'Unknown Command'


    def start(self):
        header = {  "Host": "127.0.0.1:9000",
                    "Content-Type": "application/json",
                    "Accept": "application/json",
                    "Connection": "Keep-Alive",
                    "X-Auth-Token": "fdcd8651-da46-41e7-a29d-09e6893e23d4"
                 }

        body = {
           "start" : {
              "server-uuid" : "52fe8307-4223-4918-a933-072cb76700c0",
              "timestamp" : "2016-05-03 19:31:57"
           }
        }

        #req = urllib2.Request('http://%s:%d/vnf/action/start' % (self.server_ip, self.server_port))
        req = urllib2.Request('http://127.0.0.1:9000/vnf/action/start')
        req.add_header('Content-Type', 'application/json')

        response = urllib2.urlopen(req, json.dumps(body))



    def install(self):
        header = {  "Host": "127.0.0.1:9000",
                    "Content-Type": "application/json",
                    "Accept": "application/json",
                    "Connection": "Keep-Alive",
                    "X-Auth-Token": "fdcd8651-da46-41e7-a29d-09e6893e23d4"
                 }

        body = {
                   "install" : {
                      "details" : {
                         "package_file_name" : "vMSS-SIT_20150922_1850_test_with_md5sum.tar",
                         "sw_file_name" : "V-IMS_1.0.0_CSCF_TOTAL_SW_DB_VCSCF_V-EPC_VM_150817_112200.tar",
                         "sw_file_path" : "/home/vnfm/CMS/package/vMSS-SIT_20150922_1850_test_with_md5sum",
                         "vnfd_id" : "f0719440-aa07-11e5-8646-bfb54c",
                         "vnfd_path" : "/home/vnfm/CMS/package/vMSS-SIT_20150922_1850_test_with_md5sum/vCSCF -vMSS-ST_1.0.0-0.99_20150812-2050.xml"
                      },
                      "server-uuid" : "52fe8307-4223-4918-a933-072cb76700c0",
                      "timestamp" : "2016-05-03 19:26:08"
                   }
                }


        req = urllib2.Request('http://%s:%d/vnf/action/install' % (self.server_ip, self.server_port))
        req.add_header('Content-Type', 'application/json')

        response = urllib2.urlopen(req, json.dumps(body))

    def stop(self):
        header = {  "Host": "127.0.0.1:9000",
                    "Content-Type": "application/json",
                    "Accept": "application/json",
                    "Connection": "Keep-Alive",
                    "X-Auth-Token": "fdcd8651-da46-41e7-a29d-09e6893e23d4"
                 }

        body = {
                    "stop" : {
                        "server-uuid" : "52fe8307-4223-4918-a933-072cb76700c0",
                        "timestamp" : "2016-05-23 19:45:00"
                    }
               }

#        req = urllib2.Request('http://%s:%d/vnf/action/stop' % (self.server_ip, self.server_port))
        req = urllib2.Request('http://127.0.0.1:9000/vnf/action/stop')
        req.add_header('Content-Type', 'application/json')

        response = urllib2.urlopen(req, json.dumps(body))




    def exit(self):
        sys.exit(0)

if __name__ == '__main__':

    client   = Client(('127.0.0.1', 9000))

    client.do()

    sys.exit(0)

