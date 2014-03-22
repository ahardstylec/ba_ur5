require 'thread'
require './deserialize.rb'

def recv_interface(interface)
	while true
		buf = interface.socket.recv(1254)
		RobotState.unpack(buf, interface.robot_state)
	end
end

module Ur5
	class Script
		attr_accessor :script, :scriptname

		def initialize(scriptname, debug=false)
			@scriptname = scriptname
			read_script
			@debug=debug
		end

		def read_script
			@script = ""
			puts("loading script..") if @debug
			script_file_socket = File.open(@scriptname, 'r')
			while(line = script_file_socket.gets)
				unless line =~ /#/
					@script << line
					puts line if @debug
				end
			end
			@script
		end

		def manipulate(text, val)
			puts "manipulate" if @debug
			abort("cant find text #{text} in script, cant manipulate") unless @script.gsub!(/#{text}/, val)
			puts @script if @debug
			@script
		end
	end

	class Controller
		attr_accessor :script, :secondary_interface, :primary_interface, :ip_address, :primary_port, :seconary_port

		def initialize(ip_address, ports={}, debug=false)
			self.ip_address= ip_address
			self.primary_port= ports[:primary] || 30001
			self.seconary_port= ports[:seconary] || 30002
			@debug=debug
		end

		def connect_controller
			puts("connect to Ur5Controller(#{self.ip_address})") if @debug
			begin
				self.secondary_interface= SecondaryInterface.new(self.ip_address, self.seconary_port) unless self.secondary_interface && self.secondary_interface.connected?
				# self.primary_interface= PrimaryInterface.new(self.ip_address, self.primary_port) unless self.primary_interface && self.primary_interface.connected?
				self.secondary_interface.connect
				# self.primary_interface.connect
			rescue Exception => e
				abort ("cant connect to Ur5SecondaryInterface: #{e.message}")
			end
		end

		def disconnect_controller
			self.secondary_interface.disconnect if self.secondary_interface
			self.primary_interface.disconnect if self.primary_interface
			puts("disconnect from Ur5Controller") if @debug
		end

		def send_script(ur5_script)
			puts("send script") if @debug
			puts ur5_script.script if @debug
			self.secondary_interface.send(ur5_script.script)
		end

		def test_controller
			begin
				@ur5.connect_controller unless @ur5.connected?
				script = Ur5::Script.new
				script.script="set digital out(0, True)"
				# TODO parse secondary interface for masterData and DigitalOut bits == 1
			rescue
				abort("connection to robot not stable")
			end
		end
	end

	
	class Ur5Interface
		attr_accessor :socket, :ip_address, :port, :robot_state
		
		def initialize(ip_address, port)
			self.ip_address= ip_address
			self.port= port
		end

		def connect
			self.socket= TCPSocket.new(self.ip_address, self.port) unless self.socket
			Thread.new{recv_interface(self)}
		end

		def connected?
			self.socket ? !self.socket.closed? : nil
		end
		
		def disconnect
			self.socket.close if self.connected?
			self.socket=nil
		end
	end

	class SecondaryInterface < Ur5Interface
		def initialize(*args)
			super(*args)
		end

		def send(txt)
			self.socket.puts(txt) if self.connected?
		end
	end

	class PrimaryInterface < Ur5Interface
		def initialize(*args)
			super(*args)
		end
	end
end