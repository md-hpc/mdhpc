EXECUTABLE = ./md_fpga_host
HOST_SRCS += Includes/Vitis_Accel_Examples/common/includes/xcl2/xcl2.cpp host.cpp 
CXXFLAGS += -I Includes/Vitis_Accel_Examples/common/includes/xcl2

CXXFLAGS += -fmessage-length=0
LDFLAGS += -lrt -lstdc++ 
LDFLAGS += -pthread

XILINX_XRT = /opt/xilinx/xrt

EMCONFIG_DIR = tmp_dir
PLATFORM = xilinx_u280_gen3x16_xdma_1_202211_1

$(EXECUTABLE): $(HOST_SRCS)
		g++ -o $@ $^ $(CXXFLAGS) $(LDFLAGS)
		
emconfig:$(EMCONFIG_DIR)/emconfig.json
$(EMCONFIG_DIR)/emconfig.json:
	emconfigutil --platform $(PLATFORM) --od $(EMCONFIG_DIR)
