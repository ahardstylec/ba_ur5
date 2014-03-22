from Tkinter import *
from tkMessageBox import *
import math
from my_utils import connection, Player

class Game(Toplevel): 
	def __init__(self, s_interface=None, master=None):
		Toplevel.__init__(self)
		self.protocol('WM_DELETE_WINDOW', self.close_window)
		self.title("Game")
		self.master=master
		self.player=None
		self.rotation_count=0
		self.side_changed=False
		self.mainframe=None
		self.player_label_name = StringVar()
		self.player_label_name.set("Aktueller Spieler: Niemand ausgewaehlt")
		self.s_interface=s_interface
		self.add_player_window=None
		self.player_found=None

		menubar = Menu(self)
		self.filemenu= Menu(menubar, tearoff=0)
		self.filemenu.add_command(label="new player", command=self.new_player)
		self.filemenu.add_command(label="load player",command=self.load_player)
		self.filemenu.add_separator()
		self.filemenu.add_command(label="exit", command=self.close_window)
		menubar.add_cascade(label="File", menu=self.filemenu)
		self.config(menu=menubar)

		self.current_player_label= Label(self, textvariable=self.player_label_name)
		self.change_start_button = self.master.new_robot_button(self, text="change start position", command=self.change_start_position, grid_options={'column': 1, 'row':0})
		self.start_button= self.master.new_robot_button(master=self, text="Play", command=self.play, grid_options={'column':2, 'row':0})
		Frame(self, height=2, bd=1, relief=SUNKEN).grid(sticky=E+W, column=0, columnspan=3, row=1)

		self.current_player_label.grid(column=0, row=0)

	def new_mainframe(self):
		if self.mainframe is not None:
			self.mainframe.destroy()
		self.mainframe=Frame(self)
		self.mainframe.grid(column=0, row=2, columnspan=3)
		return self.mainframe

	def close_window(self):
		if(self.add_player_window is not None):
			self.add_player_window.close_window()
		self.destroy()

	def play(self):
		self.start_button.config(state='disabled')
		if self.player is None:
			showinfo("Spiel Information", "Spiele mit Spieler Unknown")
			player= connection.Player()
			player['name']="Unknown"
			self.player_changed(player)
		self.s_interface.movej(self.player['start_pos'])
		self.control_frame()

	def load_player(self):
		frame=self.new_mainframe()
		Label(frame, text="What's the players name?").grid(column=0, row=0)
		playername = Entry(frame)
		playername.grid(column=1,row=0)
		use_button = Button(frame, text="use player", state='disabled', command=lambda: self.close_load_player(frame))
		found_label_var= StringVar()
		search_button = Button(frame, text="search", command=lambda: self.search_player(playername.get(), use_button, found_label_var))
		found_label = Label(frame, textvariable=found_label_var)
		search_button.grid(column=2,row=0)
		use_button.grid(column=0,row=1)
		found_label.grid(column=1,row=1)

	def close_load_player(self, t):
		self.player_changed(self.player_found)
		t.destroy()

	def search_player(self, name, button, label):
		player = connection.Player.find_one({'name': name})
		if player is not None:
			self.player_found=player
			button.config(state='normal')
			label.set("%s found"%name)
		else:
			button.config(state='disabled')
			label.set("Player with name %s not found"%name)

	def player_changed(self, player):
		self.player=player
		if(self.player is not None):
			name = self.player['name']
			self.change_start_button.config(state='normal')
		else:
			name = "Niemand ausgewaehlt"
			self.change_start_button.config(state='disabled')
		self.player_label_name.set("Aktueller Spieler: %s" % name)

	def rotate_left_object(self):
		positions=self.s_interface.get_joint_positions()
		if self.rotation_count<4:
			positions[5]+=math.radians(71.299)
			self.rotation_count+=1
		else:
			positions[5]-=math.radians(71.299)*4
			self.rotation_count-=4
		self.s_interface.movej(positions)

	def rotate_right_object(self):
		positions=self.s_interface.get_joint_positions()
		if self.rotation_count>-4:
			positions[5]-=math.radians(71.299)
			self.rotation_count-=1
		else:
			self.rotation_count+=4
			positions[5]+=math.radians(71.299)*4
		self.s_interface.movej(positions)
				
	def change_side(self):
		self.s_interface.movej(positions=self.player['start_pos'])
		x, y, z, rx, ry, rz= self.s_interface.get_tcp_position()
		positions =[x,y,z,rx,ry,rz]
		if self.side_changed == False:
			positions[4]+= math.radians(-50)
			self.side_changed=True
		else:
			positions[4]+= math.radians(50)
			self.side_changed=False
		self.s_interface.movel(positions)

	def restart(self):
		self.rotation_count=0
		self.side_changed=False
		self.s_interface.movej(positions=self.player['start_pos'])

	def change_start_position(self):
		t = Toplevel(self)
		self.s_interface.movej(positions=self.player['start_pos'])
		t.title="Start position aendern"
		Label(t, text="Benutze das control Window um die position fuer dich anzupassen.\nWenn der Roboter auf richtiger Hoehe steht clicke auf Ok.\nWenn du die Position nicht aenderen willst clicke auf Abbrechen").grid(column=0, row=0, columnspan=2)
		Button(t, text="Ok", command=lambda: self.change_start_position_ok_button(t)).grid(row=1,column=0)
		Button(t, text="Abbrechen", command=t.destroy).grid(row=1,column=1)

	def change_start_position_ok_button(self, t):
		self.player['start_pos']=str(self.s_interface.get_joint_positions())
		self.player.save_player()
		t.destroy()

	def new_player(self):
		frame = self.new_mainframe()
		new_player = Button(frame, text="Spieler Hinzufuegen", command=lambda: self.create_player(frame))
		name_label= Label(frame, text="name des spielers")
		self.name_field = Entry(frame)
		name_label.grid(column=0, row=0)
		self.name_field.grid(column=1, row=0)
		new_player.grid(column=3, row=2)

	def create_player(self,frame):
		Player = connection.Player()
		Player['name']= self.name_field.get()
		Player.save()
		self.player_changed(Player)
		frame.destroy()

	def finished_game(self, frame):
		self.s_interface.movej(self.player['start_pos'])
		frame.destroy()

	def control_frame(self):
		frame = self.new_mainframe()
		Button(frame, text="Rotate right", command=self.rotate_right_object).grid(column=1,row=0)
		Button(frame, text="Rotate left", command=self.rotate_left_object).grid(column=0,row=0)
		Button(frame, text="change side", command=self.change_side).grid(column=0,row=1)
		Button(frame, text="finish", command= lambda: self.finished_game(frame)).grid(column=2,row=1)
		Button(frame, text="start position", command=self.restart).grid(column=1,row=1)