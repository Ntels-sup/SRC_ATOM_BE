#!/usr/bin/python

import sys
import socket
import struct
import select

HOST = ""
PORT = 8000
SEQUENCE = 1
HEADSIZE = 36

def SendRequest(CLIENT, H, B) :
	global SEQUENCE

	head = struct.pack(">B10sBIIIIII",
								H['ver'],
								H['cmd'].encode('utf-8'), 
								H['flg'],
								H['snode'],
								H['sproc'],
								H['dnode'],
								H['dproc'],
								SEQUENCE,
							 	HEADSIZE)

	CLIENT.sendall(head)
	if (len(B) > 0) :
		CLIENT.sendall(B)

	SEQUENCE += 1

	return;

def SendResponse(CLIENT, H, B) :
	head = struct.pack(">B10sBIIIIII",
								0x01,
								H['cmd'].encode('utf-8'), 
								0x20,
								H['snode'],
								H['sproc'],
								H['dnode'],
								H['dproc'],
								H['seq'],
							 	HEADSIZE+len(B))
	CLIENT.sendall(head)
	CLIENT.sendall(B)

	return;

def SendNotice(CLIENT) :
	global SEQUENCE

	payload = 'this is body.payload'

	head = struct.pack(">B10sBIIIIII",
								0x01,
								'0000000001'.encode('utf-8'), 
								0x40,
								1,
								1,
								1,
								100,
								SEQUENCE,
							 	HEADSIZE+len(payload))
	CLIENT.sendall(head)
	CLIENT.sendall(payload.encode('utf-8'))

	SEQUENCE += 1

	return;

def SendError(CLIENT, H, B) :
	print(H['ver'])
	print(H['cmd'])
	print(H['seq'])
	head = struct.pack(">B10sBIIIIII",
								0x01,
								H['cmd'], 
								0x20 | 0x04,
								H['snode'],
								H['sproc'],
								H['dnode'],
								H['dproc'],
								H['seq'],
							 	len(B)+HEADSIZE)
	CLIENT.sendall(head)
	CLIENT.sendall(B)

	return;


if __name__ == "__main__" :
	if len(sys.argv) == 2 :
		PORT = int(sys.argv[1])
	elif len(sys.argv) > 2 :
		print ("Usage) %s <listen port>" % sys.argv[0])
		
	SERV = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	print ("Socket created OK")

	SERV.setblocking(0)
	SERV.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1);

	try :
		SERV.bind((HOST, PORT))
	except socket.error, msg :
		print ("Bind failed. Error code : " + str(msg[0]) + " Message " + msg[1])
		sys.exit()
	print ("Socket bind OK, port : %d" % PORT)

	SERV.listen(10)
	print ("Socekt now listening")


	CLIENT = False
	turnNotice = True
	
	while 1:

		if CLIENT :
			CLIENT.settimeout(0);
			sock_list = [CLIENT]
		else :
			sock_list = [SERV]
		readable, writeable, exceptional = select.select(sock_list, [], [], 5)

		if SERV in readable :
			CLIENT, addr = SERV.accept()
			print ("Connected with " + addr[0] + ":" + str(addr[1]))
			continue

		H = {}
		if CLIENT and (CLIENT in readable) :
			try :
				CLIENT.settimeout(5);
				head = CLIENT.recv(HEADSIZE, socket.MSG_WAITALL)
				if len(head) != HEADSIZE :
					if len(head) == 0 :
						print ("Socket disconnected")
					else :
						print ("Recv header size invalied, size : %d" % len(head))
					CLIENT.close()
					CLIENT = False
					continue

				'''
				# Test sample
				payload = 'this is body.payload';
				head = struct.pack('>B10sBIIIIII20s', 0x01, '0000000001'.encode('utf-8'), 0x10, 
								1, 1, 1, 100, 1, 56, payload.encode('utf-8')); 
				'''

				# Header validation check
				H['ver']	= struct.unpack(">B", head[0:1])[0]		# version, size 1
				H['cmd']	= struct.unpack(">10s", head[1:11])[0]	# command, size 10
				H['flg']	= struct.unpack(">B", head[11:12])[0]	# flag, size 1
				H['snode']	= struct.unpack(">I", head[12:16])[0]	# src nodeid, size 4
				H['sproc']	= struct.unpack(">I", head[16:20])[0]	# src procid, size 4
				H['dnode']	= struct.unpack(">I", head[20:24])[0]	# dst nodeid, size 4
				H['dproc']	= struct.unpack(">I", head[24:28])[0]	# dst procid, size 4
				H['seq']	= struct.unpack(">I", head[28:32])[0]	# sequence, size 4
				H['len']	= struct.unpack(">I", head[32:36])[0]	# length, size 4

				print ("Recv Data ----------------------- ")
				print ("Header.Version     : %d" % H['ver'])
				print ("Header.Command     : %s" % H['cmd'])
				print ("Header.Flag        : 0x%x" % H['flg'])
				print ("Header.Source Node : %d" % H['snode'])
				print ("Header.Source Proc : %d" % H['sproc'])
				print ("Header.Dest   Node : %d" % H['dnode'])
				print ("Header.Dest   Proc : %d" % H['dproc'])
				print ("Header.Sequence    : %d" % H['seq'])
				print ("Header.Length      : %d" % H['len'])
				print ("")

				if H['ver'] != 0x01 :
					payload = "Header, 'version' filed invalied value: %d" % H['ver']
					print (payload)
					SendError(CLIENT, H, payload.encode('utf-8'))
					CLIENT.close()
					CLIENT = False
					continue;

				if H['flg'] != 0x10 and H['flg'] != 0x40 and H['flg'] != 0x20 :
					payload = "Header, 'flag' filed invalied, value : 0x%02x" % H['flg']
					print (payload);
					SendError(CLIENT, H, payload.encode('utf-8'))
					CLIENT.close()
					CLIENT = False
					continue;

				if (H['len'] - HEADSIZE) > 0 :
					payload = CLIENT.recv(H['len'] - HEADSIZE, socket.MSG_WAITALL)
					if len(payload) != (H['len'] - HEADSIZE) :
						payload = "Body, payload size receive failed, size = %d" % len(payload)
						print (payload);
						SendError(CLIENT, H, payload.encode('utf-8'))
						CLIENT.close()
						CLIENT = False
						continue

			except Exception, x:
				print ("Disconnected, %s" % x)
				CLIENT.close()
				CLIENT = False
				continue
			
			if H['flg'] == 0x10 :
				try :
					payload = "Response Payload"
					SendResponse(CLIENT, H, payload.encode('utf-8'))
				except Exceptions, x :
					print ("DisCLIENTected")
					CLIENT.close()
					CLIENT = False
					continue

		elif CLIENT :
			if turnNotice :
				SendNotice(CLIENT)
				turnNotice = False
			else :
				H['ver']	= 0x01			# version, size 1
				H['cmd']	= '0000000001'	# command, size 10
				H['flg']	= 0x10			# flag, size 1
				H['snode']	= 1000			# src nodeid, size 4
				H['sproc']	= 1001			# src procid, size 4
				H['dnode']	= 2000			# dst nodeid, size 4
				H['dproc']	= 2001			# dst procid, size 4
				H['seq']	= 0				# sequence, size 4
				H['len']	= 0				# length, size 4
				B = ''
				SendRequest(CLIENT, H, B) 
				turnNotice = True

	if CLIENT :
		CLIENT.close()
	SERV.close()
