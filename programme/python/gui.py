from Tkinter import *
from tkMessageBox import *
from PIL import Image, ImageTk
from tkFileDialog import askopenfilename, asksaveasfile, asksaveasfilename
from ur_interfaces import SecondaryInterface, Script
from my_utils import *

# -*- coding: utf-8 -*-

# Main gui
class Application(Frame):
	registered_elements={}

	def __init__(self, root=None, s_interface=None):
		Frame.__init__(self)
		
		# settings for Frame ---------------------------------------------
		#-----------------------------------------------------------------

		self.root = root 			 # value to the main tk object
		if root is not None:
			self.root.protocol('WM_DELETE_WINDOW', self.quit)
		self.title="h-da Robot Interface"
		self.grid()

		#-----------------------------------------------------------------
		#-----------------------------------------------------------------

		# default Values -------------------------------------------------
		#-----------------------------------------------------------------

		self.connected=False 		 # value set if Application is connected to Robot
		self.script=None 			 # Script value if Ur5 Script is Used
		self.s_interface=s_interface # value for the s_interface alls actions to Robot are over the s_interface
		self.roboter_buttons=[] 	 # all widgeds that are used to controll the robot. should be disabled when Application is not connected to Robot

		#-----------------------------------------------------------------
		#-----------------------------------------------------------------

		self.createControlFrame()


		image = Image.open("robot.png")
		resized = image.resize((200, 250), Image.ANTIALIAS)	
		photo = ImageTk.PhotoImage(resized)
		self.image = Label(self, image=photo)
		self.image.image = photo
		self.image.grid(column=0, row=0, rowspan=2)


		#  Connection Info Icons
		Label(self, text="connection:").grid(row=0, column=1, sticky=NE)
		self.connected_img = ImageTk.PhotoImage(Image.open("connected.png"))
		self.not_connected_img = ImageTk.PhotoImage(Image.open("not_connected.png"))

		self.connected_image=Label(self, image=self.connected_img)
		self.connected_image.image=self.connected_img
		self.connected_image.grid(row=0, column=2, sticky=NW)
		
		self.not_connected_image=Label(self, image=self.not_connected_img)
		self.not_connected_image.image=self.not_connected_img
		self.not_connected_image.grid(row=0, column=2, sticky=NW)

		self.robot_mode_label = StringVar()
		self.robot_mode_label.set("Mode: %s"%self.s_interface.robot_mode_str())
		Label(self, textvariable=self.robot_mode_label).grid(row=0, column=3,sticky=NE)

		s_interface.robot_state.robot_mode_data.add_robot_mode_change_handler(fun=self.mode_changed_handler)

		# ---------------------------------------------------------------------------
		# ---------------------------------------------------------------------------


		# Load Menu including submenu's File and App's 
		self.menu()
		# ---------------------------------------------------------------------------
		# ---------------------------------------------------------------------------
		

		# disbable buttons if not connected to Robot
		if not self.connected:
			self.disableButtons()

		# in mainloop of application check if robot has connection, if so enable widgets and switch connection icons
		if root is not None:
			root.after(0, self.check_connection)
#----------------------------------------------------------------------------------------------------
#--------------------------- INIT END ---------------------------------------------------------------


	# def move_up_command(self, event):
	# 	self.s_interface.move_to_direction(SecondaryInterface.UP)
	# def move_down_command(self, event):
	# 	self.s_interface.move_to_direction(SecondaryInterface.DOWN)
	# def move_right_command(self, event):
	# 	self.s_interface.move_to_direction(SecondaryInterface.RIGHT)
	# def move_left_command(self, event):
	# 	self.s_interface.move_to_direction(SecondaryInterface.LEFT)
	# def move_to_direction(self, event):
	# 	self.s_interface.move_to_direction(SecondaryInterface.LEFT)

	def mode_changed_handler(self, mode=8, mode_str="Not Connected"):
		self.robot_mode_label.set("Mode: %s"%mode_str)

	def stop_program(self, event):
		self.s_interface.stop_program()

	def free_mode_command(self):
		if self.s_interface.is_robot_running_mode():
			self.s_interface.set_freedrive_mode()
			self.free_mode_button.config(text="Running Mode")
		else:
			self.s_interface.set_run_mode()
			self.free_mode_button.config(text="Freedrive Mode")

	def enableButton(self, button):
		button.config(state='normal')

	def disableButton(self, button):
		button.config(state='disabled')

	def enableButtons(self):
		# self.roboter_buttons = [x for x in self.roboter_buttons if x is not None]
		for button in self.roboter_buttons:
			self.enableButton(button)

	def disableButtons(self):
		# self.roboter_buttons = [x for x in self.roboter_buttons if x is not None]
		for button in self.roboter_buttons:
			self.disableButton(button)

	def createControlFrame(self):
		control_frame = Frame(self)

		self.free_mode_button = self.new_robot_button(control_frame, text="Free Mode", command=self.free_mode_command, grid_options={'row':4, 'column':0})
		self.stop = self.new_robot_button(control_frame, text="Stop!", command=self.s_interface.stop_program, grid_options={'row':1, 'column':1})

		self.move_up = self.new_robot_button(control_frame, text="Move Up", grid_options={'row': 0, 'column': 1})
		self.move_up.bind("<ButtonRelease>", self.stop_program)
		self.move_up.bind("<ButtonPress>", lambda event: self.s_interface.move_to_direction(SecondaryInterface.UP))

		self.move_down = self.new_robot_button(control_frame, text="Move Down", grid_options={'row':2, 'column':1})
		self.move_down.bind("<ButtonRelease>", self.stop_program)
		self.move_down.bind("<ButtonPress>", lambda event: self.s_interface.move_to_direction(SecondaryInterface.DOWN))

		self.move_left = self.new_robot_button(control_frame, text="Move left",grid_options={'row':1, 'column':0})
		self.move_left.bind("<ButtonRelease>", self.stop_program)
		self.move_left.bind("<ButtonPress>", lambda event: self.s_interface.move_to_direction(SecondaryInterface.LEFT))

		self.move_right = self.new_robot_button(control_frame, text="Move right",grid_options={'row':1, 'column':2})
		self.move_right.bind("<ButtonRelease>", self.stop_program)
		self.move_right.bind("<ButtonPress>", lambda event: self.s_interface.move_to_direction(SecondaryInterface.RIGHT))

		self.move_backward = self.new_robot_button(control_frame, text="Move backward",grid_options={'row':0, 'column':2})
		self.move_backward.bind("<ButtonRelease>", self.stop_program)
		self.move_backward.bind("<ButtonPress>", lambda event: self.s_interface.move_to_direction(SecondaryInterface.BACKWARD))

		self.move_forward = self.new_robot_button(control_frame, text="Move forward",grid_options={'row':2, 'column':2})
		self.move_forward.bind("<ButtonRelease>", self.stop_program)
		self.move_forward.bind("<ButtonPress>", lambda event: self.s_interface.move_to_direction(SecondaryInterface.FORWARD))

		control_frame.grid(row=1, column=1, sticky=NW, columnspan=2)

	def append_registered_elements(self):
		for element_key in Application.registered_elements.keys():
			self.appmenu.add_command(label=element_key, command= lambda: Application.registered_elements[element_key](master=self, s_interface=self.s_interface))

	def load_script(self, filename=None):
		if filename is None:
			filename = askopenfilename(parent=self)
		if connection.Script.find_one({'filename': filename}) is not None:
			self.script = connection.Script.find_one({'filename': filename})
		else: 
			self.script = connection.Script()
			self.script.create(filename=filename)
		self.script.load_script() 
		ScriptGui(self, self.script)

	def new_script(self):
		self.script = connection.Script()
		ScriptGui(master=self, script=self.script)

	def connect(self):
		if((self.s_interface is not None) and self.connected is False):
			connected= self.s_interface.connect_interfaces()
			if connected is not True:
				showwarning("connect interfaces", "could not connect to Robot")
				self.s_interface.disconnect_interfaces()
			else:
				self.mode_changed_handler=8
				# self.s_interface.robot_state.robot_mode_data.robot_mode=0
				# self.mode_changed_handler(self.s_interface.robot_mode_str())
				self.filemenu.entryconfigure(2, label="disconnect")
		else:
			self.s_interface.disconnect_interfaces()
			# self.s_interface.robot_state.robot_mode_data.robot_mode=8
			# self.mode_changed_handler(self.s_interface.robot_mode_str())
			self.filemenu.entryconfigure(2, label="connect")
	
	def check_connection(self):
		if self.s_interface.connected:#self.s_interface.send_thread is not None and self.s_interface.send_thread.socket is not None:
			if self.connected == False:
				self.connected_image.lift()
				self.enableButtons()
				self.connected=True
		else:			
			if self.connected == True:
				print("robot is not connected")	
				self.connected_image.lower()
				self.disableButtons()
				self.connected=False
		self.root.after(500, self.check_connection)

	def new_robot_button(self, master=None, text="No Title given", command=None, options=None, grid_options=None):
		if master is None:
			return
		if options is None:
			options={}
		if grid_options is None:
			grid_options={}

		b= Button(master, text=text, command=command, **options)
		if(not self.connected):
			b.config(state='disabled')
		b.grid(**grid_options)
		self.roboter_buttons.append(b)
		return b

	def menu(self):
		self.menubar = Menu(self.root)
		self.filemenu = Menu(self.menubar, tearoff=0)
		self.filemenu.add_command(label="New Script",command=self.new_script)
		self.filemenu.add_command(label="Load Script", command=self.load_script)
		self.filemenu.add_command(label="Connect", command=self.connect)
		self.filemenu.add_separator()
		self.filemenu.add_command(label="Exit", command=self.quit)
		self.menubar.add_cascade(label="File", menu=self.filemenu)

		self.appmenu= Menu(self.menubar, tearoff=0)
		self.menubar.add_cascade(label="Apps", menu=self.appmenu)
		self.append_registered_elements()

	@staticmethod
	def register(name, gui_element_fun):
		Application.registered_elements[name]=gui_element_fun


class ScriptGui(Toplevel):
	def __init__(self, master, script):
		Toplevel.__init__(self)
		self.protocol('WM_DELETE_WINDOW', self.close_window)
		self.master=master
		self.script=script
		self.inserts={}
		self.text = Text(self)
		self.text.insert(END, self.script.raw)

		menubar = Menu(self)
		filemenu = Menu(menubar, tearoff=0)
		filemenu.add_command(label="save",command=self.save)
		filemenu.add_command(label="save as",command=self.save_as)
		filemenu.add_separator()
		filemenu.add_command(label="Exit",command=self.close_window)
		menubar.add_cascade(label="File", menu=filemenu)
		self.config(menu=menubar)

		self.start_button = Button(self, text="Run Script", command= self.start_script)
		self.master.roboter_buttons.append(self.start_button)
		self.filenamelabel= StringVar()
		self.filenamelabel.set("File: %s"%self.script.filename or "File: not saved yet")
		Label(self, textvariable=self.filenamelabel).grid(row=0, columnspan=3)

		self.frame = Frame(self)
		self.frame.grid(column=1, row=1, sticky=N)
		self.refresh_inserts_button = Button(self.frame, text="refresh inserts", command=self.refresh_inserts)
		self.refresh_inserts_button.grid(column=0, row=0, columnspan=2)
		self.refresh_inserts()
		self.bind('<Control-s>', lambda event: self.save())
		row_count = self.search_for_inserts()

		self.text.grid(column=0, row=1)
		self.start_button.grid(column=0, row=2)

		if self.master.connected is False:
			self.master.disableButton(self.start_button)

	def save(self):
		self.initial_save()
		if self.script.filename is None or len(self.script.filename)==0:
			self.script.filename = asksaveasfilename(parent=self)
		self.script.save_script()
		self.filenamelabel.set("File: %s"%self.script.filename)

	def save_as(self):
		self.initial_save()
		filename = asksaveasfilename(parent=self)
		self.script.save_as(filename)
		self.filenamelabel.set("File: %s"%self.script.filename)

	def initial_save(self):
		self.script.raw = self.text.get('1.0', END)
		for insertname in self.inserts.keys():
			self.script.inserts[insertname] = self.inserts[insertname].get()

	def start_script(self):
		self.script.inserts={}
		self.script.raw=self.text.get('1.0', END)
		for insertname in self.inserts:
			self.script.inserts[insertname] = self.inserts[insertname].get()
		self.master.s_interface.start_program(self.script.excecutable())

	def refresh_inserts(self):
		self.script.raw= self.text.get('1.0', END)
		for key in self.inserts.keys():
			self.inserts[key].destroy()
			del self.inserts[key]
		self.search_for_inserts()

	def search_for_inserts(self):
		count=1
		for insertname in re.findall(r'<%(.*?)%>', self.script.raw):
			if not self.inserts.has_key(insertname):
				print("new found insert %s" % insertname)
				Label(self.frame, text=insertname).grid(column=0, row=count, sticky=N)
				self.inserts[insertname]=Entry(self.frame)
				self.inserts[insertname].grid(column=1, row=count, sticky=N)
				if self.script.inserts.has_key(insertname):
					self.inserts[insertname].insert(0, self.script.inserts[insertname])
			count+=1
		return count

	def close_window(self):
		print("ScriptGui close window fired")
		self.master.roboter_buttons.remove(self.start_button)
		self.destroy()

def load_gui(s_interface):
	root = Tk()
	app = Application(root=root, s_interface=s_interface)
	root.config(menu=app.menubar)
	app.mainloop()
	root.destroy()