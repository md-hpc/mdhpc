# Definitional proc to organize widgets for parameters.
proc init_gui { IPINST } {
  ipgui::add_param $IPINST -name "Component_Name"
  #Adding Page
  set Page_0 [ipgui::add_page $IPINST -name "Page 0"]
  ipgui::add_param $IPINST -name "AXIL_ADDR_WIDTH" -parent ${Page_0}
  ipgui::add_param $IPINST -name "AXIL_DATA_WIDTH" -parent ${Page_0}
  ipgui::add_param $IPINST -name "AXIS_SUMMARY_WIDTH" -parent ${Page_0}
  ipgui::add_param $IPINST -name "AXIS_TDATA_WIDTH" -parent ${Page_0}
  ipgui::add_param $IPINST -name "STREAMING_TDEST_WIDTH" -parent ${Page_0}


}

proc update_PARAM_VALUE.AXIL_ADDR_WIDTH { PARAM_VALUE.AXIL_ADDR_WIDTH } {
	# Procedure called to update AXIL_ADDR_WIDTH when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.AXIL_ADDR_WIDTH { PARAM_VALUE.AXIL_ADDR_WIDTH } {
	# Procedure called to validate AXIL_ADDR_WIDTH
	return true
}

proc update_PARAM_VALUE.AXIL_DATA_WIDTH { PARAM_VALUE.AXIL_DATA_WIDTH } {
	# Procedure called to update AXIL_DATA_WIDTH when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.AXIL_DATA_WIDTH { PARAM_VALUE.AXIL_DATA_WIDTH } {
	# Procedure called to validate AXIL_DATA_WIDTH
	return true
}

proc update_PARAM_VALUE.AXIS_SUMMARY_WIDTH { PARAM_VALUE.AXIS_SUMMARY_WIDTH } {
	# Procedure called to update AXIS_SUMMARY_WIDTH when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.AXIS_SUMMARY_WIDTH { PARAM_VALUE.AXIS_SUMMARY_WIDTH } {
	# Procedure called to validate AXIS_SUMMARY_WIDTH
	return true
}

proc update_PARAM_VALUE.AXIS_TDATA_WIDTH { PARAM_VALUE.AXIS_TDATA_WIDTH } {
	# Procedure called to update AXIS_TDATA_WIDTH when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.AXIS_TDATA_WIDTH { PARAM_VALUE.AXIS_TDATA_WIDTH } {
	# Procedure called to validate AXIS_TDATA_WIDTH
	return true
}

proc update_PARAM_VALUE.STREAMING_TDEST_WIDTH { PARAM_VALUE.STREAMING_TDEST_WIDTH } {
	# Procedure called to update STREAMING_TDEST_WIDTH when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.STREAMING_TDEST_WIDTH { PARAM_VALUE.STREAMING_TDEST_WIDTH } {
	# Procedure called to validate STREAMING_TDEST_WIDTH
	return true
}


proc update_MODELPARAM_VALUE.AXIS_TDATA_WIDTH { MODELPARAM_VALUE.AXIS_TDATA_WIDTH PARAM_VALUE.AXIS_TDATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.AXIS_TDATA_WIDTH}] ${MODELPARAM_VALUE.AXIS_TDATA_WIDTH}
}

proc update_MODELPARAM_VALUE.AXIS_SUMMARY_WIDTH { MODELPARAM_VALUE.AXIS_SUMMARY_WIDTH PARAM_VALUE.AXIS_SUMMARY_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.AXIS_SUMMARY_WIDTH}] ${MODELPARAM_VALUE.AXIS_SUMMARY_WIDTH}
}

proc update_MODELPARAM_VALUE.STREAMING_TDEST_WIDTH { MODELPARAM_VALUE.STREAMING_TDEST_WIDTH PARAM_VALUE.STREAMING_TDEST_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.STREAMING_TDEST_WIDTH}] ${MODELPARAM_VALUE.STREAMING_TDEST_WIDTH}
}

proc update_MODELPARAM_VALUE.AXIL_DATA_WIDTH { MODELPARAM_VALUE.AXIL_DATA_WIDTH PARAM_VALUE.AXIL_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.AXIL_DATA_WIDTH}] ${MODELPARAM_VALUE.AXIL_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.AXIL_ADDR_WIDTH { MODELPARAM_VALUE.AXIL_ADDR_WIDTH PARAM_VALUE.AXIL_ADDR_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.AXIL_ADDR_WIDTH}] ${MODELPARAM_VALUE.AXIL_ADDR_WIDTH}
}

