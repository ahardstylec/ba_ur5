#!/home/andi/.rvm/rubies/ruby-1.9.3-p194/bin/ruby

require 'socket'
require './ur5.rb'

debug=false
if ARGV.length > 1 && ARGV.last == "debug"
	debug=true
	ARGV.pop
end

@ur5 = Ur5::Controller.new("141.100.101.20", {}, debug)

at_exit do
	@ur5.disconnect_controller if @ur5
	puts "quit(#{($!.nil? && !$!.respond_to?(:status))? 0 : $!.status}) and disconnect" if debug
end

# @ur5.test_controller
@ur5.connect_controller

abort("no program given") if ARGV.empty?
abort("program #{ARGV[0]} not found") unless File.exists?(ARGV[0])

script = Ur5::Script.new(ARGV[0], debug)
	
#----- manipulate script here

script.manipulate("<%angle%>", ARGV[1]) if(ARGV[1])

#-----

# @ur5.send_script(script)

while true
end