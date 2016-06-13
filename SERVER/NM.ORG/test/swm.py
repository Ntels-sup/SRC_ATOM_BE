#!/usr/bin/python

import sys
import getopt
import socket
import struct
import select

HOST = "127.0.0.1"
PORT = 8000
SEQUENCE = 1
HEADSIZE = 36
NODEID = 1
PROCID = 9000

def SendRequest(SOCK, H, B) :
	global SEQUENCE

	head = struct.pack(">B10sBIIIIII",
                                H['ver'],
                                H['cmd'].encode('utf-8'),
                                H['flag'],
                                H['snode'],
                                H['sproc'],
                                H['dnode'],
                                H['dproc'],
                                SEQUENCE,
                                HEADSIZE + len(B))

	SOCK.sendall(head)
	if (len(B) > 0) :
		SOCK.sendall(B)

	SEQUENCE += 1

	return;


def SendResponse(SOCK, H, B) :
	head = struct.pack(">B10sBIIIIII",
								0x01,
								H['cmd'].encode('utf-8'), 
								0x20,
								H['snode'],
								H['sproc'],
								H['dnode'],
								H['dproc'],
								H['seq'],
							 	HEADSIZE + len(B))
	SOCK.sendall(head)
	SOCK.sendall(B)

	return;

def SendError(SOCK, H, B) :
	head = struct.pack(">B10sBIIIIII",
                                0x01,
                                H['cmd'],
                                0x20 | 0x04,
                                H['snode'],
                                H['sproc'],
                                H['dnode'],
                                H['dproc'],
                                H['seq'],
                                HEADSIZE + len(B))
	SOCK.sendall(head)
	SOCK.sendall(B)

	return;


if __name__ == "__main__" :

	try :
		opts, args = getopt.getopt(sys.argv[1:], "n:p:P:h", ["node=", "proc=", "port="])
	except getopt.GetoptError as err :
		print str(err)
		print ("usage: %s -n <node id> -p <proc id> -P <port>" % sys.argv[0])
		sys.exit(1)

	for opt, arg in opts:
		if opt == '-h' :
			print ("usage: %s -n <node id> -p <proc id> -P <port>" % sys.argv[0])
			sys.exit(1)
		elif opt in ('-n', '--node') :
			NODEID = int(arg)
		elif opt in ('-p', '--proc') :
			PROCID = int(arg)
		elif opt in ('-P', '--prot') :
			PORT = int(arg)
		else :
			print ("invalied option, %s %s" % opt, arg)
			

	if NODEID <= 0 or PROCID <= 0 :
		print("invalied node or proc id");
		print ("usage: %s -n <node id> -p <proc id> -P <port>" % sys.argv[0])
		sys.exit(1)

	SOCK = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	print ("Socket created OK")

	#SOCK.setblocking(0)
	#SOCK.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1);

	try :
		SOCK.connect((HOST, PORT))
	except socket.error, msg :
		print ("connect failed : " + str(msg[0]) + " Message " + msg[1])
		sys.exit()
	print ("Socket connect OK, port : %d" % PORT)

	H = {}

	# Regist messge
	H['ver']	= 0x01
	H['cmd']	= '0000000001'	# command, size 10
	H['flag']	= 0x10			# flag, size 1
	H['snode']	= NODEID		# src nodeid, size 4
	H['sproc']	= PROCID		# src procid, size 4
	H['dnode']	= 1				# dst nodeid, size 4
	H['dproc']	= 8				# dst procid, size 4
	H['seq']	= 0
	H['len']	= 0
	B = '{ "BODY": { "pkgname": "ATOM", "nodetype": "EMS", "procname": "WSM", "procno": %d } }' % PROCID
	print(B)
	SendRequest(SOCK, H, B) 
	
	while True :
		# Header receive
		#SOCK.settimeout(0);
		head = SOCK.recv(HEADSIZE, socket.MSG_WAITALL)
		if len(head) != HEADSIZE :
			if len(head) == 0 :
				print ("Socket disconnected")
			else :
				print ("Recv header size invalied, size : %d" % len(head))
			SOCK.close()
			sys.exit(1)

		H['ver']	= struct.unpack(">B", head[0:1])[0]		# version, size 1
		H['cmd']	= struct.unpack(">10s", head[1:11])[0]	# command, size 10
		H['flag']	= struct.unpack(">B", head[11:12])[0]	# flag, size 1
		H['snode']	= struct.unpack(">I", head[12:16])[0]	# src nodeid, size 4
		H['sproc']	= struct.unpack(">I", head[16:20])[0]	# src procid, size 4
		H['dnode']	= struct.unpack(">I", head[20:24])[0]	# dst nodeid, size 4
		H['dproc']	= struct.unpack(">I", head[24:28])[0]	# dst procid, size 4
		H['seq']	= struct.unpack(">I", head[28:32])[0]	# sequence, size 4
		H['len']	= struct.unpack(">I", head[32:36])[0]	# length, size 4

		print ("Recv Data ----------------------- ")
		print ("Header.Version     : %d" % H['ver'])
		print ("Header.Command     : %s" % H['cmd'])
		print ("Header.Flag        : 0x%x" % H['flag'])
		print ("Header.Source Node : %d" % H['snode'])
		print ("Header.Source Proc : %d" % H['sproc'])
		print ("Header.Dest   Node : %d" % H['dnode'])
		print ("Header.Dest   Proc : %d" % H['dproc'])
		print ("Header.Sequence    : %d" % H['seq'])
		print ("Header.Length      : %d" % H['len'])
		print ("")

		# Body receive
		if (H['len'] - HEADSIZE) > 0 :
			#SOCK.settimeout(5);
			payload = SOCK.recv(H['len'] - HEADSIZE, socket.MSG_WAITALL)

			if len(payload) != (H['len'] - HEADSIZE) and H['flag'] == 0x10 :
				payload = "Body, payload size receive failed, size = %d" % len(payload)
				print (payload);
				SendError(SOCK, H, payload.encode('utf-8'))

			print (payload)
			print ("")

		# PING request
		if H['cmd'] == '0000000003' and H['flag'] == 0x10 :
			H['ver']	= 0x01
			H['cmd']	= '0000000003'
			H['flag']	= 0x20
			H['dnode']	= H['snode']
			H['dproc']	= H['sproc']
			H['snode']	= NODEID
			H['sproc']	= PROCID
			H['seq']	= 0
			H['len']	= 0
			B = '{ "BODY": { "code": 0, "text": "i am alive" } }'
			SendResponse(SOCK, H, B) 

	# end while

	SOCK.close()
