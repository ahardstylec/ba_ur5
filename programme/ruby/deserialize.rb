module PackageType
    ROBOT_MODE_DATA = 0
    JOINT_DATA = 1
    TOOL_DATA = 2
    MASTERBOARD_DATA = 3
    CARTESIAN_INFO = 4
    KINEMATICS_INFO = 5
    CONFIGURATION_DATA = 6
    FORCE_MODE_DATA = 7
    ADDITIONAL_INFO = 8
end


module RobotMode
    RUNNING = 0
    FREEDRIVE = 1
    READY = 2
    INITIALIZING = 3
    SECURITY_STOPPED = 4
    EMERGENCY_STOPPED = 5
    FATAL_ERROR = 6
    NO_POWER = 7
    NOT_CONNECTED = 8
    SHUTDOWN = 9
    SAFEGUARD_STOP = 10
end

class RobotModeData
	attr_accessor 'timestamp', 'robot_connected', 'real_robot_enabled',
                 'power_on_robot', 'emergency_stopped',
                 'security_stopped', 'program_running', 'program_paused',
                 'robot_mode', 'speed_fraction'

    def self.unpack(buf)
        rmd = RobotModeData.new
        rmd.timestamp, rmd.robot_connected, 
        rmd.real_robot_enabled, rmd.power_on_robot,
        rmd.emergency_stopped, rmd.security_stopped, rmd.program_running,
        rmd.program_paused, rmd.robot_mode, rmd.speed_fraction= buf.unpack("Q>CCCCCCCCG")
		rmd.robot_connected = rmd.robot_connected == 1
		rmd.real_robot_enabled = rmd.real_robot_enabled == 1
		rmd.power_on_robot = rmd.power_on_robot == 1
		rmd.emergency_stopped = rmd.emergency_stopped == 1
		rmd.security_stopped = rmd.security_stopped == 1
		rmd.program_running = rmd.program_running == 1
		rmd.program_paused = rmd.program_paused == 1
    end
end

class JointData
    attr_accessor 'q_actual', 'q_target', 'qd_actual',
                 'i_actual', 'v_actual', 't_motor', 't_micro', 'joint_mode'
    def self.unpack(buf)
        all_joints = []
        offset = 0
        (0..5).each do |i|
            jd = JointData.new()
            jd.q_actual, jd.q_target, jd.qd_actual, jd.i_actual, jd.v_actual, jd.t_motor, jd.t_micro, jd.joint_mode = buf[offset..-1].unpack("GGGggggC")
            offset += 41
            all_joints<< jd
        end
        return all_joints
    end
end


class ToolData
    attr_accessor 'analog_input_range2', 'analog_input_range3',
                 'analog_input2', 'analog_input3',
                 'tool_voltage_48V', 'tool_output_voltage', 'tool_current',
                 'tool_temperature', 'tool_mode'
    
    def self.unpack(buf)
        td = ToolData.new
        td.analog_input_range2, td.analog_input_range3,
        td.analog_input2, td.analog_input3,
        td.tool_voltage_48V, td.tool_output_voltage, td.tool_current,
        td.tool_temperature, td.tool_mode = buf.unpack("ccGGgCggC")
        return td
	end
end

class MasterboardData
    attr_accessor 'digital_input_bits', 'digital_output_bits',
                 'analog_input_range0', 'analog_input_range1',
                 'analog_input0', 'analog_input1',
                 'analog_output_domain0', 'analog_output_domain1',
                 'analog_output0', 'analog_output1',
                 'masterboard_temperature',
                 'robot_voltage_48V', 'robot_current',
                 'master_io_current', 'master_safety_state',
                 'master_onoff_state'
    
    def self.unpack(buf)
        md = MasterboardData.new
		md.digital_input_bits, md.digital_output_bits,
		md.analog_input_range0, md.analog_input_range1,
		md.analog_input0, md.analog_input1,
		md.analog_output_domain0, md.analog_output_domain1,
		md.analog_output0, md.analog_output1,
		md.masterboard_temperature,
		md.robot_voltage_48V, md.robot_current,
		md.master_io_current, md.master_safety_state,
		md.master_onoff_state = buf.unpack("s!>s!>ccGGccGGggggCCc")
        return md
	end 
end

class CartesianInfo
    attr_accessor 'x', 'y', 'z', 'rx', 'ry', 'rz'
    def self.unpack(buf)
        ci = CartesianInfo.new
    	ci.x, ci.y, ci.z, ci.rx, ci.ry, ci.rz = buf.unpack("GGGGGG")
        return ci
    end
end

class ForceModeData
    attr_accessor 'x', 'y', 'z', 'rx', 'ry', 'rz', 'robot_dexterity'
    
    def self.unpack(buf)
        fmd = ForceModeData.new
        fmd.x, fmd.y, fmd.z, fmd.rx, fmd.ry, fmd.rz, fmd.robot_dexterity = buf.unpack("GGGGGGG")
        return fmd
    end
end

class AdditionalInfo
    attr_accessor 'ctrl_bits', 'teach_button'
    
    def self.unpack(buf)
        ai = AdditionalInfo.new
        ai.ctrl_bits, ai.teach_button = buf.unpack("CC")
		ai.teach_button= ai.teach_button == 1
        return ai
    end
end

class JointLimitData
    attr_accessor 'min_limit', 'max_limit', 'max_speed', 'max_acceleration'
end

class ConfigurationData
    attr_accessor 'joint_limit_data',
                 'v_joint_default', 'a_joint_default', 
                 'v_tool_default', 'a_tool_default', 'eq_radius',
                 'dh_a', 'dh_d', 'dh_alpha', 'dh_theta',
                 'masterboard_version', 'controller_box_type',
                 'robot_type', 'robot_subtype'
    def self.unpack(buf)
        cd = ConfigurationData.new
        cd.joint_limit_data = []
        begin
	        (0..6).to_a.each do |i|
	            jld = JointLimitData.new
	            jld.min_limit, jld.max_limit = buf[(5+16*i)..-1].unpack("GG")
	            jld.max_speed, jld.max_acceleration = buf[(5+16*6+16*i)..-1].unpack("GG")
	            cd.joint_limit_data.push(jld)
	        end
	        cd.v_joint_default, cd.a_joint_default, cd.v_tool_default, cd.a_tool_default,cd.eq_radius = buf[5+32*6..-1].unpack("GGGGG")
	        cd.masterboard_version, cd.controller_box_type, cd.robot_type, cd.robot_subtype = buf[5+32*6+5*8+6*32..-1].unpack("L>l>l>l>")
    	rescue Exception => e
    		puts e.message
    	end
    	return cd
    end
end

class KinematicsInfo
    def self.unpack(buf)
        KinematicsInfo.new
    end
end

class RobotState
	include PackageType
	attr_accessor 'robot_mode_data', 'joint_data', 'tool_data',
                 'masterboard_data', 'cartesian_info',
                 'kinematics_info', 'configuration_data',
                 'force_mode_data', 'additional_info',
                 'unknown_ptypes'
    def initialize()
    	self.unknown_ptypes=[]
    end

    def self.unpack(buf, rs)
        return if(buf.size<5)
        length, mtype = buf[0..4].unpack("L>C")
		if length < len(buf)
            print("got to many packages")
            return RobotState.unpack(buf[buf.size-length..-1])
        elsif length > len(buf)
            abort("Could not unpack packet: length field is incorrect")
        end
        if mtype != 16
            if mtype == 20
                print("Likely a syntax error:")
                print(buf)
            end
        end
        rs = self.new unless rs
        offset = 5
        while offset < buf.size do
        	length, ptype = buf[offset..-1].unpack("L>C")
        	offset+=5
            package_buf = buf[offset..offset+length-5]
        	puts("unpacking package #{ptype.ord}")
            case ptype
            	when PackageType::ROBOT_MODE_DATA
            		rs.robot_mode_data = RobotModeData.unpack(package_buf)
            	when PackageType::JOINT_DATA
            		rs.joint_data = JointData.unpack(package_buf)
            	when PackageType::TOOL_DATA
            		rs.tool_data = ToolData.unpack(package_buf)
            	when PackageType::MASTERBOARD_DATA
            		rs.masterboard_data = MasterboardData.unpack(package_buf)
            	when PackageType::CARTESIAN_INFO
            		rs.cartesian_info = CartesianInfo.unpack(package_buf)
            	when PackageType::FORCE_MODE_DATA
            		rs.force_mode_data = ForceModeData.unpack(package_buf)
            	when PackageType::ADDITIONAL_INFO
            		rs.additional_info = AdditionalInfo.unpack(package_buf)
            	when PackageType::CONFIGURATION_DATA
            		rs.configuration_data = ConfigurationData.unpack(package_buf)
            	when PackageType::KINEMATICS_INFO
            		rs.kinematics_info = KinematicsInfo.unpack(package_buf)
            	else
            		rs.unknown_ptypes << ptype
            end
            offset += length-5
        end
    end
end