import threading
import sys
import math
import numpy as np
import socket
import time
import re
from deserialize import *
from collections import deque
from my_utils import *

BUFFER_SIZE=2048



# Thread For Sending Script Commands to the Ur5 Roboter
# --------------------------------------------------
class OpenSecondSendInterface(threading.Thread):
    send_lock=threading.Lock()
    run_lock=threading.Lock()

    def __init__(self,socket, s_interface):
        threading.Thread.__init__(self)
        self.s_interface = s_interface
        self.__run_flag=False
        self.socket=socket

    def stop(self):
        self.__run_flag=False

    def run(self):
        # print("starting send interface waiting for queue")
        self.__run_flag=True
        while self.__run_flag:
            OpenSecondSendInterface.send_lock.acquire()
            queue_len = len(self.s_interface.send_messages_queue)
            if(queue_len > 0):
                # print("message queue contains messsages %d" % len(self.s_interface.send_messages_queue))
                message=self.s_interface.send_messages_queue.popleft()
            else:
                message=None
            OpenSecondSendInterface.send_lock.release()
            if(message is not None):
                try:
                    print("send: %s"%message)
                    self.socket.sendall(message+"\n")
                except self.socket.error, e:
                    print "Tcp socket of send interface has disconnected : %s" % e
                    self.__run_flag=False
        self.socket.close()
        return 0


# Thread For Recieving Packages from the Ur5 Roboter
# --------------------------------------------------
class OpenSecondRecvInterface(threading.Thread):
    recv_lock=threading.Lock()
    run_lock=threading.Lock()

    def __init__(self, socket, s_interface):
        threading.Thread.__init__(self)
        self.s_interface = s_interface
        self.__run_flag=False
        self.socket=socket

    def stop(self):
        self.__run_flag=False

    def run(self):
        buf=None
        if self.socket is None:
            return
        self.__run_flag=True
        buf_queue=deque()
        self.socket.settimeout(1.5)
        while self.__run_flag:
            try:
                buf= self.socket.recv(BUFFER_SIZE)
            except socket.timeout,e:
                err = e.args[0]
                # this next if/else is a bit redundant, but illustrates how the
                # timeout exception is setup
                if err == 'timed out':
                    # print 'recv timed out, retry later'
                    print("recieve socket timout error")
                    self.s_interface.disconnect_interfaces()
                    self.s_interface.connect_interfaces()
                    return
                else:
                    print e
                    signal.send_signal(signal.SIGINT)
                    # sys.exit(1)
            except socket.error, e:
                # Something else happened, handle error, exit, etc.
                print e
                signal.send_signal(signal.SIGINT)
            else:
                # buf_queue.append(buf)
                RobotState.unpack(buf, self.s_interface.robot_state)
        self.socket.close()
        return 0

# Thread for handling a Deque.
# This Queue contains programs or Script Commands for the Ur5 Roboter
# Each Programm or Script Command will be send if the Previous programm has finished 
class DequePrograms(threading.Thread):
    lock=threading.Lock()

    def __init__(self, s_interface):
        threading.Thread.__init__(self)
        self.s_interface = s_interface
        self.__run_flag=False

    def stop(self):
        self.__run_flag=False

    def run(self):
        self.__run_flag=True
        while self.__run_flag:
            DequePrograms.lock.acquire()
            if(len(self.s_interface.program_queue) > 0):
                # print("message queue contains messsages %d" % len(self.s_interface.program_queue))
                message=self.s_interface.program_queue.popleft()
            else:
                message=None
            DequePrograms.lock.release()
            if(message is not None):
                OpenSecondSendInterface.send_lock.acquire()
                self.s_interface.send_messages_queue.append(message)
                OpenSecondSendInterface.send_lock.release()
                self.s_interface.block_program()
            time.sleep(0.2)
        return 0


# SecondaryInterface: Adapter to Ur5 Robot
# creates connection to Robot
# sends script or commands to robot

class SecondaryInterface(object):
    LEFT=0
    RIGHT=1
    DOWN=2
    UP=3
    FORWARD=4
    BACKWARD=5

    def __init__(self, tcp="141.100.101.20", port=30002, debug=False, network_interface="eth0"):
        self.tcp_ip=tcp 
        self.tcp_port=port
        self.recv_thread=None
        self.send_thread=None
        self.save_data_thread=None
        self.deque_thread=None
        self.network_interface=network_interface
        self.robot_state = RobotState()
        self.robot_state.robot_mode_data= RobotModeData()
        self.robot_state.robot_mode_data.program_running= False
        self.robot_state.robot_mode_data.robot_mode=8
        self.robot_state.masterboard_data= MasterboardData()
        self.robot_state.masterboard_data.digital_output_bits=0
        self.debug=debug
        self.send_messages_queue=deque()
        self.connected=False
        self.radius=0.85
        self.program_queue=deque()

    def create_socket(self):
        r_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        r_socket.settimeout(0.5)
        try:
            r_socket.connect((self.tcp_ip, self.tcp_port))
        except:
            print("couldnt connect to Server")
            r_socket.close()
            r_socket=None

        # except socket.error as msg:
        #     print(msg)
        if(r_socket is not None):
            r_socket.settimeout(None)
        return r_socket


    def block_program(self):
        while True:
            if self.is_program_running():
                break
        print("program is started")
        while True:
            if not self.is_program_running():
                break

    def connect_interfaces(self):
        print("connect to Robot...")
        recv_socket= self.create_socket()
        rv = True
        if recv_socket is not None:
            self.recv_thread = OpenSecondRecvInterface(recv_socket, self)
            self.recv_thread.setDaemon(True)
            self.recv_thread.start()
        else:
            print("(Recv Interface)could not connect to robot")
            rv= False

        send_socket= self.create_socket()
        if send_socket is not None:
            self.send_thread = OpenSecondSendInterface(send_socket, self)
            self.send_thread.setDaemon(True)
            self.send_thread.start()
        else:
            print("(Send Interface)could not connect to robot")
            rv= False

        self.deque_thread = DequePrograms(self)
        self.deque_thread.setDaemon(True)
        self.deque_thread.start()

        if rv:
            self.connected=True

        return rv

    def disconnect_interfaces(self):
        print("closing threads")
        if(self.send_thread is not None):
            print("\tclose send interface...")
            self.send_thread.stop()
            self.send_thread=None
            print("done\n")

        if(self.recv_thread is not None):
            print("\tclose recieve interface...")
            self.recv_thread.stop()
            print("done\n")

        if(self.deque_thread is not None):
            print("\tclose program queue interface...")
            self.deque_thread.stop()
            self.deque_thread=None
            print("done\n")

        self.connected=False

    def start_program(self, txt):
        if(self.debug):
            print(txt)
        DequePrograms.lock.acquire()
        self.program_queue.append(txt)
        DequePrograms.lock.release()

    def is_program_running(self):
        return self.robot_state.robot_mode_data.program_running == True

    def get_joint_positions(self):
        join_positions=[]
        for i in range(6):
            join_positions.append(self.robot_state.joint_data[i].q_actual)
        return join_positions
    
    def get_tcp_position(self):
        ci = self.robot_state.cartesian_info
        tcp_position=[ci.x,ci.y,ci.z,ci.rx,ci.ry,ci.rz]
        return tcp_position;

    def send_command(self, txt):
        OpenSecondSendInterface.send_lock.acquire()
        self.send_messages_queue.append(txt)
        OpenSecondSendInterface.send_lock.release()

    def is_robot_freedrive_mode(self):
        return self.robot_state.robot_mode_data.robot_mode == RobotMode.FREEDRIVE

    def is_robot_running_mode(self):
        return self.robot_state.robot_mode_data.robot_mode == RobotMode.RUNNING

    def robot_mode_str(self):
        return RobotMode.ModeString[self.robot_state.robot_mode_data.robot_mode]


#--------------------------------------------------------------------------------------------------
#------------------------- UR5 ROBOT COMMANDS -------------------------------------------------
#--------------------------------------------------------------------------------------------------

    def get_distance(self, p_peak, p_now, side):
        b = p_peak - p_now
        return np.sqrt(b.dot(b))

    def move_to_direction(self, direction, distance=0.5):
        x,y,z,rx,ry,rz=self.get_tcp_position()
        position=[x,y,z,rx,ry,rz]
        get_x = lambda r: r*r-pow(y*y+z*z,2)
        get_y = lambda r: r*r-pow(x*x+z*z,2)
        get_z = lambda r: r*r-pow(y*y+x*x,2)
        p_now=np.array([x,y,z])
        if direction == SecondaryInterface.LEFT:
            distance = get_x(self.radius)# self.get_distance(np.array([get_x(self.radius), y, z]), p_now, x)
            position[0] = distance
            # print("distance %f"%distance)
            insert_command= "movel(p%s, a=0.1, v=0.15)" % str(position)
            # insert_command= "movel(pose_add(get_actual_tcp_pose(), p[%f,0.0,0.0,0.0,0.0,0.0]), a=0.1, v=0.15)" % distance
        elif direction == SecondaryInterface.RIGHT:
            distance = get_x(self.radius)#self.get_distance(np.array([get_x(-self.radius),y,z]), p_now, x)
            position[0]=-distance
            insert_command= "movel(p%s, a=0.1, v=0.15)" % str(position)
            # print("distance %f"%distance)
            # insert_command= "movel(pose_add(get_actual_tcp_pose(), p[%f,0.0,0.0,0.0,0.0,0.0]), a=0.1, v=0.15)" % -distance
        elif direction == SecondaryInterface.BACKWARD:
            # distance = self.get_distance(np.array([x,get_y(-self.radius),z]), p_now, y)
            # print("distance %f"%distance)
            # insert_command= "movel(pose_add(get_actual_tcp_pose(), p[0.0,%f,0.0,0.0,0.0,0.0]), a=0.1, v=0.15)" % -distance
            distance = get_y(self.radius)#self.get_distance(np.array([get_x(-self.radius),y,z]), p_now, x) 
            position[1]=-distance
            insert_command= "movel(p%s, a=0.1, v=0.15)" % str(position)
        elif direction == SecondaryInterface.FORWARD:
            # distance = self.get_distance(np.array([x,get_y(self.radius),z]), p_now, y)
            # print("distance %f"%distance)
            # insert_command= "movel(pose_add(get_actual_tcp_pose(), p[0.0,%f,0.0,0.0,0.0,0.0]), a=0.1, v=0.15)" % distance
            distance = get_y(self.radius)#self.get_distance(np.array([get_x(-self.radius),y,z]), p_now, x)      
            position[1]=distance
            insert_command= "movel(p%s, a=0.1, v=0.15)" % str(position)
        elif direction == SecondaryInterface.UP:
            # distance = self.get_distance(np.array([x,y,get_z(self.radius)]), p_now, z)
            # insert_command= "movel(pose_add(get_actual_tcp_pose(), p[0.0,0.0,%f,0.0,0.0,0.0]), a=0.1, v=0.15)" % distance
            distance = get_z(self.radius)#self.get_distance(np.array([get_x(-self.radius),y,z]), p_now, x)
            print("distance %f"%distance)
            position[2]=distance
            insert_command= "movel(p%s, a=0.1, v=0.15)" % str(position)
        elif direction == SecondaryInterface.DOWN:
            # distance = self.get_distance(np.array([x,y,get_z(-self.radius)]), p_now, z)
            # print("distance %f"%distance)
            # insert_command= "movel(pose_add(get_actual_tcp_pose(), p[0.0,0.0,%f,0.0,0.0,0.0]), a=0.1, v=0.15)" % -distance  
            distance = get_z(self.radius)#self.get_distance(np.array([get_x(-self.radius),y,z]), p_now, x)  
            position[2]=-distance
            insert_command= "movel(p%s, a=0.1, v=0.15)" % str(position)
        else:
            return 
        message="""def move_to_direction():
    thread move_down_thread():
        %s
    end
    move_handler= run move_down_thread()
    while digital_in[1] == False:
        sync()
    end
    while digital_in[1] == True:
        sync()
    end
    kill move_handler
end
""" % insert_command
        self.start_program(message)

    def stop_program(self):
        self.send_command("stop program\n")
        self.robot_state.robot_mode_data.program_running=False

    def set_freedrive_mode(self):
        self.send_command("set robotmode freedrive\n")

    def set_run_mode(self):
        self.send_command("set robotmode run\n")

    def movej(self, positions=None, a_max=None, v_max=None):
        if positions is None:
            positions= self.get_join_positions()
        if a_max is None:
            a_max=math.radians(40)
        if v_max is None:
            v_max=math.radians(60)
        message="""movej(%s,a=%f,v=%f)
        """%(positions,a_max,v_max)
        print message
        self.start_program(message)

    def movel(self, positions=None, a_max=None, v_max=None):
        if positions is None:
            positions= self.get_join_positions()
        if a_max is None:
            a_max=math.radians(40)
        if v_max is None:
            v_max=math.radians(60)
        message="""movel(p%s,a=%f,v=%f)
        """%(positions, a_max, v_max)
        print message
        self.start_program(message)