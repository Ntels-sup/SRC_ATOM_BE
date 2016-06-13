#!/usr/bin/python

import sys
import getopt
import socket
import struct
import select
import thread

HOST = "127.0.0.1"
PORT = 8000
SEQUENCE = 1
HEADSIZE = 36
NODEID = 1
PROCID = -1
ATOM_MODULE = { "nm":1, "bjm":2, "swm":3, "wsm":4, "stm":5, "trm":6, "alm":7 }

def SendPrint(H, B) :
	print ("")
	print ("Send Data ----------------------- ")
	print ("Header.Version     : %d" % H['ver'])
	print ("Header.Command     : %s" % H['cmd'])
	print ("Header.Flag        : 0x%x" % H['flag'])
	print ("Header.Source Node : %d" % H['snode'])
	print ("Header.Source Proc : %d" % H['sproc'])
	print ("Header.Dest   Node : %d" % H['dnode'])
	print ("Header.Dest   Proc : %d" % H['dproc'])
	print ("Header.Sequence    : %d" % H['seq'])
	print ("Header.Length      : %d" % H['len'])
	print (B)
	print ("")

def SendRequest(SOCK, H, B) :
	global SEQUENCE

	B = B.encode('utf-8')
	H['seq'] = SEQUENCE
	H['len'] = HEADSIZE + len(B)

	head = struct.pack(">B10sBIIIIII",
								H['ver'],
								H['cmd'].encode('utf-8'), 
								H['flag'],
								H['snode'],
								H['sproc'],
								H['dnode'],
								H['dproc'],
								H['seq'],
							 	H['len'])

	SOCK.sendall(head)
	if (len(B) > 0) :
		SOCK.sendall(B)

	SendPrint(H, B)

	SEQUENCE += 1

	return;

def SendResponse(SOCK, H, B) :

	B = B.encode('utf-8')
	H['len'] = HEADSIZE + len(B)

	head = struct.pack(">B10sBIIIIII",
								0x01,
								H['cmd'].encode('utf-8'), 
								0x20,
								H['snode'],
								H['sproc'],
								H['dnode'],
								H['dproc'],
								H['seq'],
							 	H['len'])
	SOCK.sendall(head)
	SOCK.sendall(B)

	SendPrint(H, B)

	return;

def SendNotice(SOCK, H, B) :
	global SEQUENCE

	B = B.encode('utf-8')
	H['seq'] = SEQUENCE
	H['len'] = HEADSIZE + len(B)

	head = struct.pack(">B10sBIIIIII",
								0x01,
								H['cmd'].encode('utf-8'), 
								0x40,
								H['snode'],
								H['sproc'],
								H['dnode'],
								H['dproc'],
								H['seq'],
							 	H['len'])
	SOCK.sendall(head)
	SOCK.sendall(B)

	SendPrint(H, B)

	SEQUENCE += 1

	return;

def SendError(SOCK, H, B) :
	B = B.encode('utf-8')
	H['len'] = HEADSIZE + len(B)

	head = struct.pack(">B10sBIIIIII",
								0x01,
								H['cmd'], 
								0x20 | 0x04,
								H['snode'],
								H['sproc'],
								H['dnode'],
								H['dproc'],
								H['seq'],
							 	H['len'])
	SOCK.sendall(head)
	SOCK.sendall(B)

	SendPrint(H, B)

	return;

def ReceiveMesg (SOCK, isLoop) :
	
	while True :
		# Header receive
		#SOCK.settimeout(0);
		head = SOCK.recv(HEADSIZE, socket.MSG_WAITALL)
		if len(head) != HEADSIZE :
			if len(head) == 0 :
				print ("Socket disconnected")
				sys.exit(1)
			else :
				print ("Recv header size invalied, size : %d" % len(head))
				continue

		H['ver']	= struct.unpack(">B", head[0:1])[0]
		H['cmd']	= struct.unpack(">10s", head[1:11])[0]
		H['flag']	= struct.unpack(">B", head[11:12])[0]
		H['snode']	= struct.unpack(">I", head[12:16])[0]
		H['sproc']	= struct.unpack(">I", head[16:20])[0]
		H['dnode']	= struct.unpack(">I", head[20:24])[0]
		H['dproc']	= struct.unpack(">I", head[24:28])[0]
		H['seq']	= struct.unpack(">I", head[28:32])[0]
		H['len']	= struct.unpack(">I", head[32:36])[0]

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

		# Body receive
		if (H['len'] - HEADSIZE) > 0 :
			#SOCK.settimeout(5);
			body = SOCK.recv(H['len'] - HEADSIZE, socket.MSG_WAITALL)

			if len(body) != (H['len'] - HEADSIZE) and H['flag'] == 0x10 :
				body = "Body, receive failed, size = %d" % len(body)
				print (body);
				SendError(SOCK, H, body)

			print (body)

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

		if isLoop == False :
			break

	# end while
	

if __name__ == "__main__" :

	try :
		opts, args = getopt.getopt(sys.argv[1:], "n:p:P:h", ["node=", "proc=", "port="])
	except getopt.GetoptError :
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
			print("invalied options, %s %s" % (opt, arg))
			print("usage: %s -n <node id> -p <proc id> -P <port>" % sys.argv[0])
			sys.exit(1)
	
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

	# Regist
	H['ver']	= 0x01
	H['cmd']	= '0000000001'
	H['flag']	= 0x10
	H['snode']	= NODEID
	H['sproc']	= PROCID
	H['dnode']	= 2000
	H['dproc']	= 2001
	H['seq']	= 0
	H['len']	= 0
	B = '{ "BODY": { "pkgname": "ATOM", "nodetype": "EMS", "procname": "ATOM_SWM", "procno": %d } }' % PROCID
	SendRequest(SOCK, H, B) 
	ReceiveMesg(SOCK, False)

	#thread.start_new_thread(ReceiveMesg (SOCK,))

	# command
	while True :
		in_cmd = input("COMMAND              : ")
		in_flag = raw_input("FLAG(req or noti)    : ").lower()
		in_dnode= raw_input("DEST NODE(name or #) : ").lower()
		in_dproc= raw_input("DEST PROC(name or #) : ").lower()
		in_body = raw_input("BODY(json)           : ")

		H['ver']	= 0x01
		H['cmd']	= ("%010d" % in_cmd)
		if in_flag == 'req' :
			H['flag'] = 0x10
		elif if_flag == 'noti' :
			H['flag'] = 0x40
		else :
			print ('invalied flag')
			continue

		H['snode']	= NODEID
		H['sproc']	= PROCID
		
		if in_dnode in ATOM_MODULE.keys() :
			H['dnode']	= ATOM_MODULE[in_dnode]
		else :
			H['dnode'] = int(in_dnode)
		if in_dproc in ATOM_MODULE.keys() :
			H['dproc']	= ATOM_MODULE[in_dporc]
		else :
			H['dproc'] = int(in_dproc)
			
		H['seq']	= 0
		H['len']	= 0
		SendRequest(SOCK, H, in_body)

		ReceiveMesg(SOCK, False)

	SOCK.close()
