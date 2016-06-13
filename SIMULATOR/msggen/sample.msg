#/HEAD=command,flag,srcnode,srcproc,dstnode,dstproc
#     - flag : REQUEST, NOTIFY
#/BODY=
#/{
#/    "BODY" : {
#/        ....,
#/        ....
#/    }
#/}

# message 1, empty BODY
/HEAD=20004,NOTIFY,1,1000,1,46
/BODY=NONE

# message 2
/HEAD=20004,NOTIFY,1,1000,1,46
/BODY=
/{
/    "BODY" : {
/        "message":"connection status change",
/        "node_seq_id" : 0,
/        "node_no" : 10,
/        "my_ip" : "192.168.1.123",
/        "peer_ip" : "192.168.1.145",
/        "server_port" : 12345,
/        "status":"CONNECT",
/        "prc_date" : "2016-04-12 09:14:53.92",
/        "dst_yn" : "N"
/    }
/}

# message 3
/HEAD=20004,NOTIFY,1,1000,1,46
/BODY=
/{
/    "BODY" : {
/        "message":"connection status change",
/        "node_seq_id" : 0,
/        "node_no" : 10,
/        "my_ip" : "192.168.1.123",
/        "peer_ip" : "192.168.1.145",
/        "server_port" : 12345,
/        "status":"CONNECT",
/        "prc_date" : "2016-04-12 09:14:53.92",
/        "dst_yn" : "N"
/    }
/}
