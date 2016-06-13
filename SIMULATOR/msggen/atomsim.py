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
PROCID = 9000


def MessagePrint(H, B, M = 'Message') :
	print ("")
	print ("%s -----------------------" % M)
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

	MessagePrint(H, B, 'Send Message')

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

	MessagePrint(H, B, 'Send Message')

	return;

def SendNotify(SOCK, H, B) :
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

	MessagePrint(H, B, 'Send Message')

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

	MessagePrint(H, B, 'Send Message')

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


		# Body receive
		if (H['len'] - HEADSIZE) > 0 :
			#SOCK.settimeout(5);
			B = SOCK.recv(H['len'] - HEADSIZE, socket.MSG_WAITALL)

			if len(B) != (H['len'] - HEADSIZE) and H['flag'] == 0x10 :
				B = "Body, receive failed, size = %d" % len(B)
				SendError(SOCK, H, B)

		MessagePrint(H, B, "Recevie Message")

		if isLoop == False :
			break

	# end while
	

if __name__ == "__main__" :

	if len(sys.argv) != 2 :
		print("usage: %s <message file>" % sys.argv[0])
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

	# open message file
	try :
		fp = file(sys.argv[1], 'r')
	except Exceptions, e :
		print(e)
		sys.exit(1)

	onMessage = False
	onBody = False
	B = ''

	# command
	while True :
		line = fp.readline()
		if len(line) == 0 :
			break
			
		line.replace('\n', ' ')
		if line[0] != '/' :
			continue

		# header message
		if line[:6] == '/HEAD=' :
			field = line[6:].split(',')

			H['ver']	= 0x01
			H['cmd']	= ("%010d" % int(field[0]))
			if field[1][:4] == 'REQU' :
				H['flag'] = 0x10
			elif field[1][:4] == 'NOTI' :
				H['flag'] = 0x40
			H['snode']	= int(field[2])
			H['sproc']	= int(field[3])
			H['dnode']	= int(field[4])
			H['dproc']	= int(field[5])
			H['seq']	= 0
			H['len']	= 0			
			continue

		if line[:6] == '/BODY=' :
			onBody = True
			B = ''
			if len(line) >= 10 and line[6:10] == 'NONE' :
				onBody = False 
				onMessage = True
			else :
				continue

		if onBody and line[:2] == '/{' :
			B += line[1:].strip()
			continue

		if onBody and line[:2] == '/}' :
			B += line[1:].strip()
			onBody = False
			onMessage = True

		if onBody and line[0] == '/' :
			line = line[1:].strip()
			B += line
			continue
			
		if onMessage :
			if H['flag'] == 0x10 :
				SendRequest(SOCK, H, B)
				ReceiveMesg(SOCK, False)
			elif H['flag'] == 0x40 :
				SendNotify(SOCK, H, B)

			onMessage = False

	# end while

	SOCK.close()
