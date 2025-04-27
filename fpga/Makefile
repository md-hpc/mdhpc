CP = cp -rf


XCLBIN := ./xclbin
VPP := v++

#BINARY_CONTAINER_vadd_OBJS += networklayer.xo
BINARY_CONTAINER_vadd_OBJS += MD_RL.xo
BINARY_CONTAINER_vadd_OBJS += krnl_mm2s.xo
BINARY_CONTAINER_vadd_OBJS += krnl_s2mm.xo
TARGET := hw
DEVICE := xilinx_u280_gen3x16_xdma_1_202211_1
CLFLAGS += -t $(TARGET) --platform $(DEVICE) --save-temps --config configuration_benchmark_if0.tmp.ini


HLS_IP_FOLDER  = $(shell readlink -f ./NET/NetLayers/100G-fpga-network-stack-core/synthesis_results_HBM)
LIST_REPOS = --user_ip_repo_paths $(HLS_IP_FOLDER)

.PHONY: xclbin
xclbin: $(XCLBIN)/MD_FPGA.xclbin

# Building kernel
$(XCLBIN)/MD_FPGA.xclbin: $(BINARY_CONTAINER_vadd_OBJS)
	mkdir -p $(XCLBIN)
	$(VPP) $(CLFLAGS) $(LDCLFLAGS) --vivado.synth.jobs 8 --vivado.impl.jobs 8 --clock.defaultFreqHz 15000000 -R2 -l -o '$@' $^ $(LIST_REPOS) 
	$(CP) $(XCLBIN)/MD_FPGA.xclbin ../
	
