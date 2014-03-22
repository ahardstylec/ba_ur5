#!/usr/bin/env python2.7
# -*- coding: utf-8 -*-

import deserialize
import sys
import os.path
import os
import signal
import time
import my_utils
from ur_interfaces import *
import gui
from game import Game

s_interface=None
script_file_given=False

# sigint handler, so tcp connection and thread are closed when strg+c is pressed

def close_program_handler(signum, frame):
	if(not (s_interface is None)):
		s_interface.disconnect_interfaces()
	os._exit(1)

signal.signal(signal.SIGINT, close_program_handler)

#------------------------------------------------------------------

# check startup arguments

debug=False
if(sys.argv[-1] == "debug"):
	debug=True
	sys.argv.pop()

if len(sys.argv) > 1 and os.path.isfile(sys.argv[1]):
	script_file_given=True
	inserts=[]
	if(len(sys.argv) > 2):
		inserts= sys.argv[2:len(sys.argv)]
		print("inserts found {0}".format(inserts[0]))
	
	script = Script()
	script.create(filename=sys.argv[1], inserts=inserts, debug=debug, load=true)

#-------------------------------------------------------------------

s_interface = SecondaryInterface(debug=debug)

if script_file_given:
	s_interface.connect_interfaces()
	print("start program...")
	s_interface.start_program(script.excecutable())
	s_interface.block_program()
	print("done")
	s_interface.disconnect_interfaces()
else:
	gui.Application.register("Start Game", Game)
	gui.load_gui(s_interface)
	s_interface.disconnect_interfaces()