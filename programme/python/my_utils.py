import sys, socket, struct, fcntl
import os.path
from mongokit import Document, Connection
import re

def get_ip(interface="eth0"):
	sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	sockfd = sock.fileno()
	SIOCGIFADDR = 0x8915

	ifreq = struct.pack('16sH14s', interface, socket.AF_INET, '\x00'*14)
	try:
		res = fcntl.ioctl(sockfd, SIOCGIFADDR, ifreq)
	except:
		return None
	ip = struct.unpack('16sH2x4s8x', res)[2]
	return socket.inet_ntoa(ip)

connection = Connection()

@connection.register
class Player(Document):
	__collection__ = 'players'
	__database__ = 'robot'

	use_dot_notation= True

	structure={
		'name':str,
		'start_pos':str,
		'age':int
	}

	required_fields = ['name']
	default_values= {
		'start_pos': "[1.2639,-2.5147,-1.3484,-0.8468,-1.57,0.1046]"
	}

	def save_player(self):
		self.name= self.name.encode("utf8")
		self.start_pos=self.start_pos.encode("utf8")
		self.save()

@connection.register
class Script(Document):
	__collection__ = 'scripts'
	__database__ = 'robot'

	structure={
		'raw':str,
		'inserts':dict,
		'filename':str
	}

	use_dot_notation= True

	default_values={'raw':'','inserts':{}, 'filename':''}

	def create(self, filename=None, inserts={}, debug=False, load=False):
		self.raw=""
		self.executed=""
		self.defines={}
		self.inserts=inserts
		self.filename=filename
		if load:
			self.load_script()

	def load_script(self):
		script_file = open(self.filename, 'r')
		self.raw = script_file.read()
		script_file.close()

	def save_script(self):
		self.raw=self.raw.encode("utf8")
		self.filename=self.filename.encode("utf8")
		if os.path.isfile(self.filename):
			file = open(self.filename, mode="w")
			file.write(self.raw)
			file.close()
		self.save()

	def save_as(self, filename=None):
		self.filename= filename or self.filename or ""
		self.save_script()

    # if in the script are defines like:
    # #define a 0
    # #define b "hello world"
    # then with the swap_defines method a and b are replaced with their value
	def get_defines(self):
		p = re.compile(r'#define (.*) ([\w\d\"]*)')
		for (key, value) in re.findall(p, self.raw):
			h={}
			h[key] = value
			self.defines.append(h)

    # replaced the defines with their values
	def swap_defines(self, txt):
		for (key,value) in self.defines.items():
			txt.replace(key, value)
		return

	def remove_comments(self, txt):
		rs =""
		lines=txt.splitlines(True)
		p = re.compile(r'.*#.*')
		for line in range(len(lines)):
			if not p.match(lines[line]):
				rs = rs+lines[line]
		return rs

	def replace_inserts(self, text):
		for insert in re.findall(r'<%(.*)%>', self.raw):
			text.replace("<%{0}%>".format(insert), self.inserts[insert])

	def excecutable(self):
		rs = self.raw
		self.swap_defines(rs)
		rs = self.remove_comments(rs)
		self.replace_inserts(rs)
		return rs

connection.register([Script,Player])