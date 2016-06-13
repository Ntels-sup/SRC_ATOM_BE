
# -- coding:utf-8 --

import sys
import socket
import time
import random
import struct

class Client(object):
    def __init__(self):
        self.fd = None
        # code , body length
        self.fmt_header     = '<ii'
        self.fmt_noti_body  = '<32s'
        self.fmt_event_body = '<32s32s16s64s128s4s16s'
        self.fmt_perf_inc_date = '<32s16s16s16s'
        self.fmt_perf       = '<16s16s16s'

    def connect(self):
        self.fd = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.fd.connect(('127.0.0.1', 9001))

    def send(self, data):
        self.fd.send(data)

    def clear(self):
        self.fd.close()

    def noti(self, _code):
        date = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime())

        D2 = struct.pack(self.fmt_noti_body, date)

        code    = _code
        length  = len(D2)

        D1 = struct.pack(self.fmt_header,
                         code,
                         length)

        return D1 + D2

    def event(self, _code):
        date                = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime())
        event_type          = 'Processing Error'
        severity            = 'Major'
        probable_cuase      = 'Disk Failure'
        additional_text     = 'ALRM-DISK-OVER'
        retry_cnt           = ''
        coresponse_action   = ''

        D2 = struct.pack(self.fmt_event_body,
                         date,
                         event_type,
                         severity,
                         probable_cuase,
                         additional_text,
                         retry_cnt,
                         coresponse_action)

        code    = _code
        length  = len(D2)

        D1 = struct.pack(self.fmt_header,
                         code,
                         length)


        return D1 + D2

    def event_reboot(self, _code):
        date                = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime())
        event_type          = 'Processing Error'
        severity            = 'Critical'
        probable_cuase      = 'Disk Failure'
        additional_text     = 'ALRM-DISK-OVER'
        retry_cnt           = '2'
        coresponse_action   = 'Reboot'

        D2 = struct.pack(self.fmt_event_body,
                         date,
                         event_type,
                         severity,
                         probable_cuase,
                         additional_text,
                         retry_cnt,
                         coresponse_action)

        code    = _code
        length  = len(D2)

        D1 = struct.pack(self.fmt_header,
                         code,
                         length)


        return D1 + D2

    def make_perf(self, _code, _name, _value, _supplement):
        date                = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime())
        name                = _name
        value               = _value
        supplement          = _supplement

        D2 = struct.pack(self.fmt_perf_inc_date,
                         date,
                         name,
                         value,
                         supplement)

        code    = _code
        length  = len(D2)

        D1 = struct.pack(self.fmt_header,
                         code,
                         length)

        return D1 + D2

    def cpu_perf(self, _code):
        return self.make_perf(_code, 'CPU', '37.7', '')

    def mem_perf(self, _code):
        return self.make_perf(_code, 'MEM', '42.3l', '')

    def tps_perf(self, _code):
        return self.make_perf(_code, 'TPS', '123900', 'DIAMETER-CNT')

    def disk_perf(self, _code):
        D3  = struct.pack(self.fmt_perf, 'DISK', '1.90', '/data')
        D3 += struct.pack(self.fmt_perf, 'DISK', '2.09', '/')
        D3 += struct.pack(self.fmt_perf, 'DISK', '3.09', '/home')
        D3 += struct.pack(self.fmt_perf, 'DISK', '4.09', '/etc')
        D3 += struct.pack(self.fmt_perf, 'DISK', '5.09', '/var')
        D3 += struct.pack(self.fmt_perf, 'DISK', '6.09', '/usr')
        D3 += struct.pack(self.fmt_perf, 'DISK', '7.09', '/opt')

        date    = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime())
        D2  = struct.pack('<32s', date)

        code    = _code
        length  = len(D2 + D3)

        D1 = struct.pack(self.fmt_header,
                         _code,
                         length)

        return D1 + D2 + D3

    def net_perf(self, _code):

        D3  = struct.pack(self.fmt_perf, 'eth0', '11111', 'tx')
        D3 += struct.pack(self.fmt_perf, 'eth0', '22222', 'tx')
        D3 += struct.pack(self.fmt_perf, 'eth1', '44444', 'rx')
        D3 += struct.pack(self.fmt_perf, 'eth1', '33333', 'rx')

        date    = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime())
        D2  = struct.pack('<32s', date)

        code    = _code
        length  = len(D2 + D3)

        D1 = struct.pack(self.fmt_header,
                         _code,
                         length)

        return D1 + D2 + D3

    def proc(self, _code):

        data = None

        if   _code == 1:
            data = self.noti(70100)
        elif _code == 2:
            data = self.noti(70200)
        elif _code == 3:
            data = self.noti(70300)
        elif _code == 4:
            data = self.noti(70400)
        elif _code == 5:
            data = self.event(70500)
        elif _code == 6:
            data = self.event_reboot(70500)
        elif _code == 7:
            data = self.tps_perf(70601)
        elif _code == 8:
            data = self.cpu_perf(70602)
        elif _code == 9:
            data = self.mem_perf(70603)
        elif _code == 10:
            data = self.disk_perf(70604)
        elif _code == 11:
            data = self.net_perf(70605)
        else:
            print 'Unknown'

        if data:
            self.send(data)
            print '--- SEND ', len(data)


def help():
    print '---------------------------------'
    print 'ex) CODE_NUMBER'
    print '    - 1 : ready'
    print '    - 2 : started'
    print '    - 3 : stoppend'
    print '    - 4 : subscriber'
    print '    - 5 : event'
    print '    - 6 : event - reboot'
    print '    - 7 : perf  - tps'
    print '    - 8 : perf  - cpu'
    print '    - 9 : perf  - mem'
    print '    -10 : perf  - disk'
    print '    -11 : perf  - net'

if __name__ == '__main__':

    s = Client()
    s.connect()

    while True:
        help()
        val  = raw_input("input code-number: ")
        s.proc(int(val))

    s.clear()
