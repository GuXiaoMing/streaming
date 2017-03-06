"""
# @file socket_server.py
# @Synopsis  tail the streaming play log, and try to send by socket
# @author Ming Gu(guming@itv.baidu.com))
# @version 1.0
# @date 2016-10-17
"""
import time
import subprocess
import select
import socket
import sys
import thread

HOST = ''   # Symbolic name, meaning all available interfaces
PORT = 9999

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
print 'Socket created'


def tail_and_send(conn, filename):
    """
    # @Synopsis  tail log file and send new content by socket
    #
    # @Args conn socket connection
    # @Args filename file to tail
    #
    # @Returns   None
    """
    f = subprocess.Popen(['tail', '-F', filename],\
            stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    p = select.poll()
    p.register(f.stdout)

    while True:
        if p.poll(1):
            new_line = f.stdout.readline()
            print new_line
            try:
                conn.sendall(new_line)
            except socket.error as e:
                print e
                f.terminate()
                break
        time.sleep(0.01)
    conn.close()


if __name__ == '__main__':
    filename = sys.argv[1]
    try:
        s.bind((HOST, PORT))
    except socket.error as msg:
        print 'Bind failed. Error Code : ' + str(msg[0]) + ' Message ' + msg[1]
        sys.exit()

    print 'Socket bind complete'

    #Start listening on socket
    s.listen(0)
    print 'Socket now listening'

    #now keep talking with the client
    while 1:
        conn, addr = s.accept()
        conn.settimeout(1)
        print 'Connected with ' + addr[0] + ':' + str(addr[1])
        thread.start_new_thread(tail_and_send, (conn, filename))

    s.close()
