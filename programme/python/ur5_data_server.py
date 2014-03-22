#!/usr/bin/env python2.7

from my_utils import connection, Player, get_ip
import socket
import signal
import sys
import os

class SaveDataInterface():
    def __init__(self, interface="localhost"):
        self.__run_flag=False
        self.interface = interface
        self.socket=socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        ip = get_ip(interface)
        if ip is None:
            ip="127.0.0.1"
        else:
            ip = get_ip(interface)
        port = 8000
        self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        try:
            self.socket.bind((ip, port))
            print "server is listen on %s:%d" %(ip, port)
            self.socket.listen(1)
        except socket.error, e:
            print(e[0])
            self.socket.close()
            self.socket=None

    def run(self):
        self.__run_flag=True
        # print("starting send interface waiting for queue")
        while self.__run_flag:
            if self.socket is not None:
                conn, addr = self.socket.accept()
                conn.settimeout(2)
                print "client connected: {0}".format(addr)
                player=None
                while self.__run_flag:
                    msg = self.read_from_socket(conn)
                    if msg is not None:
                        if msg == "new_patient":
                            # print("wait for new patient name")
                            name = self.read_from_socket(conn)
                            player = connection.Player()
                            player.name = name.rstrip()
                            player.save()
                        elif msg == "load_patient":
                            print("wait for patient name")
                            name = self.read_from_socket(conn)
                            player = connection.Player.find_one({'name': name.rstrip()})
                            if player is not None:
                                print("send %s"%"1".encode('ascii'))
                                print("positions %s"%("(%s)"%player.start_pos[1:-1]).encode('ascii'))
                                conn.send("(1)".encode('ascii'))
                                conn.send(("(%s)"%player.start_pos[1:-1]).encode('ascii'))
                            else:
                                conn.send("(0)".encode('ascii'))
                        elif msg == "set_patient_data":
                            # print("wait for player data")
                            start_position = self.read_from_socket(conn)
                            player.start_pos=start_position.rstrip()
                        else:
                            print("recieved unknown command")
                    else:   
                        break
                conn.close()

        self.socket.close()
        return 0

    def read_from_socket(self, conn):
        msg = None
        while self.__run_flag:
            try:
                msg = conn.recv(1024)
            except socket.timeout, e:
                err = e.args[0]
                # this next if/else is a bit redundant, but illustrates how the
                # timeout exception is setup
                if err == 'timed out':
                    # print 'recv timed out, retry later'
                    continue
                else:
                    print e
                    msg=None
            except socket.error, e:
                print e
                msg=None
            else:
                if len(msg) == 0:
                    msg=None
                    break
                else:
                    print msg
                    break
        return msg


sdi = SaveDataInterface(interface="eth0")

sdi.run()