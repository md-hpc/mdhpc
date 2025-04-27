from MD_ctrl import *
from dask.distributed import Client, get_client
from dummy_packet_gen import *
from pynq.pl_server.xrt_device import XrtStream
from vnx_utils import *
import struct
import ctypes
import os
import platform
import pynq
import re
import sys
import tempfile
import time

buffers = []
def parse_hex_line(line):
    line = line.strip()
    if len(line) < 51:
        raise ValueError(f"Line too short to parse: {line}")

    idx = len(line)
    int_32bit_list = []
    for _ in range(6):
        int_32bit = int(line[idx-8:idx], 16)
        int_32bit_list.append(int_32bit)
        idx -= 8
    # Next 2 characters = 8-bit int
    int_8bit = int(line[idx-2:idx], 16)
    idx -= 2
    # Last 3 characters = 12-bit int
    int_12bit = int(line[idx-3:idx], 16)
    idx -= 3
    # Next 6 * 8 characters = 6 x 32-bit ints (from right to left)
    

    return int_32bit_list, int_8bit, int_12bit

def read_and_parse_file(filename):
    results = []
    with open(filename, 'r') as f:
        for line_number, line in enumerate(f, 1):
            try:
                parsed = parse_hex_line(line)
                results.append(parsed)
            except ValueError as e:
                print(f"Error on line {line_number}: {e}")
    return results


input_file = "BRAM_INIT.txt"  # Replace with your file
parsed_data = read_and_parse_file(input_file)

def verify_workers():
    node_name = platform.node()
    shell_version = os.popen("xbutil dump | grep dsa_name").read()
    return node_name, shell_version[24:-2]

# Functions that will be called in the context of dask
def _invalidate(bo, offset, size):
    buf = bytearray(size)
    pynq.Device.active_device.invalidate(bo, offset, 0, size)
    pynq.Device.active_device.buffer_read(bo, offset, buf)
    return bytes(buf)


def _flush(bo, offset, size, data):
    pynq.Device.active_device.buffer_write(bo, offset, bytearray(data))
    pynq.Device.active_device.flush(bo, offset, 0, size)


def _read_registers(address, length):
    return pynq.Device.active_device.read_registers(address, length)


def _write_registers(address, data):
    pynq.Device.active_device.write_registers(address, data)


def _download(bitstream_data):
    with tempfile.NamedTemporaryFile() as f:
        f.write(bitstream_data)
        f.flush()
        ol = pynq.Overlay(f.name)


def _alloc(size, memdesc):
    mem = pynq.Device.active_device.get_memory(memdesc)
    buf = mem.allocate((size,), 'u1')
    buffers.append(buf)
    return buf.bo, buf.device_address


class DaskMemory:
    """Memory object proxied over dask

    """
    def __init__(self, device, desc):
        self._desc = desc
        self._device = device

    def allocate(self, shape, dtype):
        from pynq.buffer import PynqBuffer
        buf = PynqBuffer(shape, dtype, device_address=0,
                         bo=0, device=self._device, coherent=False)
        bo, addr = self._device._call_dask(_alloc, buf.nbytes, self._desc)
        buf.bo = bo
        buf.device_address = addr
        return buf


class DaskDevice(pynq.Device):
    """PYNQ Proxy device for using PYNQ via dask

    """
    def __init__(self, client, worker):
        """The worker ID should be unique

        """
        super().__init__("dask-" + re.sub(r'[^\w]', '_', worker))
        self._dask_client = client
        self._worker = worker
        self.capabilities = {
            'REGISTER_RW': True,
            'CALLABLE': True
        }
        self._streams = {}
        self.sync_to_device = self.flush
        self.sync_from_device = self.invalidate

    def _call_dask(self, func, *args):
        future = self._dask_client.submit(func, *args, workers=self._worker,
                                          pure=False)
        return future.result()

    def invalidate(self, bo, offset, ptr, size):
        """Copy buffer from the device to the host
        """

        ctype = ctypes.c_uint8 * size
        target = ctype.from_address(ptr)
        target[:] = self._call_dask(_invalidate, bo, offset, size)

    def flush(self, bo, offset, ptr, size):
        """Copy buffer from the host to the device
        """

        ctype = ctypes.c_uint8 * size
        target = ctype.from_address(ptr)
        self._call_dask(_flush, bo, offset, size, bytes(target))

    def read_registers(self, address, length):
        return self._call_dask(_read_registers, address, length)

    def write_registers(self, address, data):
        self._call_dask(_write_registers, address, bytes(data))

    def get_bitfile_metadata(self, bitfile_name):
        return pynq.pl_server.xclbin_parser.XclBin(bitfile_name)

    def open_context(self, description, shared=True):
        return pynq.Device.active_device.open_context(description, shared)

    def close_context(self, cu_name):
        pynq.Device.active_device.close_context(cu_name)

    def download(self, bitstream, parser=None):
        with open(bitstream.bitfile_name, 'rb') as f:
            bitstream_data = f.read()
        self._call_dask(_download, bitstream_data)
        super().post_download(bitstream, parser)

    def get_memory(self, desc):
        if desc['streaming']:
            if desc['idx'] not in self._streams:
                self._streams[desc['idx']] = XrtStream(self, desc)
            return self._streams[desc['idx']]
        else:
            return DaskMemory(self, desc)

    def get_memory_by_idx(self, idx):
        for m in self.mem_dict.values():
            if m['idx'] == idx:
                return self.get_memory(m)
        raise RuntimeError("Could not find memory")


num_cells = 27
num_particles = 300

# Board init
#pynq.PL.reset()
print(pynq.Device.devices)
ol_w0 = pynq.Overlay("MD_FPGA.xclbin",device=pynq.Device.devices[0])
ol_w0.reset()
print(ol_w0.__doc__)
ol_w0_md = init_kernel(ol_w0, 0, 1)
ol_w0_md.register_map.elem_write = 0
ol_w0_md.register_map.debug_fsms = 0
ol_w0_md.register_map.pos_x = 0
ol_w0_md.register_map.pos_y = 0
ol_w0_md.register_map.pos_z32 = 0

ol_w0_md.register_map.vel_x = 0
ol_w0_md.register_map.vel_y = 0
ol_w0_md.register_map.vel_z = 0
ol_w0_md.register_map.Cell = 0
ol_w0_md.register_map.Address = 0
ol_w0_md.register_map.reset_fsm = 0
ol_w0_md.register_map.read_ctrl = 0

ol_w0_md.register_map.reset_fsm = 0


ol_w0_md.register_map.debug_fsms = 1
ol_w0_md.register_map.debug_fsms = 0
print(ol_w0_md.register_map)
print("Boards initialized. ")
XCV_COOLDOWN = 2
# set data sending cooldown
#ol_w0_md.register_map.xcv_cooldown_cycles = 1

#ol_w0_md.register_map.init_npc = 300

#ol_w0_md.register_map.dump_bank_sel = 300

# Placeholder, should add another parameter for it
#ol_w0_md.register_map.dump_filter_sel = 600

# Pos init
rcvd_list = []
rcvd = 0


# Clear transmission FIFO

while(ol_w0_md.register_map.dest_id.__int__() != 0):
    ol_w0_md.register_map.read_ctrl = 1
    ol_w0_md.register_map.read_ctrl = 0

ol_w0_md.register_map.MD_state = 1   # 0: IDLE; 1: INIT; 2: READY_TO_RECV; 3: RUN
#time.sleep(1)
ol_w0_md.register_map.debug_fsms = 1
num_transmitted = ol_w0_md.register_map.reset_fsm.__int__()
current_diff = 0
prev_diff = 0
diff = 0
for i, (ints_32, int_8, int_12) in enumerate(parsed_data):
    ol_w0_md.register_map.elem_write = 0
    ol_w0_md.register_map.pos_x = ints_32[0]
    ol_w0_md.register_map.pos_y = ints_32[1]
    ol_w0_md.register_map.pos_z32 = ints_32[2]

    ol_w0_md.register_map.vel_x = ints_32[3]
    ol_w0_md.register_map.vel_y = ints_32[4]
    ol_w0_md.register_map.vel_z = ints_32[5]
    ol_w0_md.register_map.Cell = int_8
    ol_w0_md.register_map.Address = int_12
    
    ol_w0_md.register_map.elem_write = 0
    time.sleep(0.01)
    ol_w0_md.register_map.elem_write = 2147483648
    time.sleep(0.01)
    ol_w0_md.register_map.elem_write = 0
    while(ol_w0_md.register_map.reset_fsm.__int__() == num_transmitted):
    	continue
    ol_w0_md.register_map.elem_write = 0
    #print(ol_w0_md.register_map)
    print(str(i) + "  " + str(ol_w0_md.register_map.reset_fsm.__int__()-1))
    prev_diff = current_diff
    
    current_diff = ol_w0_md.register_map.reset_fsm.__int__()-1
    if(current_diff - prev_diff>1):
        print(current_diff - prev_diff)
    
    ol_w0_md.register_map.elem_write = 0
    num_transmitted = ol_w0_md.register_map.reset_fsm.__int__()
    ol_w0_md.register_map.elem_write = 0
    if(num_transmitted == 300):
        break
#print(ol_w0_md.register_map)
print("Particles transmitted")

#mm2s = init_pos(ol_w0, 27, 1000, 0)  # 2^22 4194304

ol_w0_md.register_map.step = 0



#time.sleep(2)

# Recv step 0

#time.sleep(2)

# Ready to perform MD

prev_transmission = [ol_w0_md.register_map.pos_x_out_A.__int__(),ol_w0_md.register_map.pos_y_out_A.__int__(),ol_w0_md.register_map.pos_z_out_A.__int__(),ol_w0_md.register_map.pos_x_out_B.__int__(),ol_w0_md.register_map.pos_y_out_B.__int__(),ol_w0_md.register_map.pos_z_out_B.__int__()]
ol_w0_md.register_map.MD_state = 2

print(ol_w0_md.register_map)
num_iterations = 10
current_step = ol_w0_md.register_map.step.__int__()

start_time_perf = time.perf_counter()
while(current_step < num_iterations):
    while(ol_w0_md.register_map.done23 == 0):
        continue
	

    print("MD iteration complete")
    ol_w0_md.register_map.read_ctrl = 0
    ol_w0_md.register_map.step = ol_w0_md.register_map.step.__int__()+1
    while(ol_w0_md.register_map.done23 == 0):
        continue
    print(ol_w0_md.register_map)
    print(len(rcvd_list))
    #print(rcvd_list)
    print(current_step)
    ol_w0_md.register_map.read_ctrl = 0
    while(len(rcvd_list)<300*current_step):
	    #time.sleep(0.5)
	    #print(ol_w0_md.register_map)
	    #print(prev_transmission)
	    ol_w0_md.register_map.read_ctrl = 0
	    if(prev_transmission != [ol_w0_md.register_map.pos_x_out_A.__int__(),ol_w0_md.register_map.pos_y_out_A.__int__(),ol_w0_md.register_map.pos_z_out_A.__int__(),ol_w0_md.register_map.pos_x_out_B.__int__(),ol_w0_md.register_map.pos_y_out_B.__int__(),ol_w0_md.register_map.pos_z_out_B.__int__()]):
		    prev_transmission = [ol_w0_md.register_map.pos_x_out_A.__int__(),ol_w0_md.register_map.pos_y_out_A.__int__(),ol_w0_md.register_map.pos_z_out_A.__int__(),ol_w0_md.register_map.pos_x_out_B.__int__(),ol_w0_md.register_map.pos_y_out_B.__int__(),ol_w0_md.register_map.pos_z_out_B.__int__()]
		    
	    if([ol_w0_md.register_map.pos_x_out_A.__int__(),ol_w0_md.register_map.pos_y_out_A.__int__(),ol_w0_md.register_map.pos_z_out_A.__int__()] != [0,0,0] and [ol_w0_md.register_map.pos_x_out_A.__int__(),ol_w0_md.register_map.pos_y_out_A.__int__(),ol_w0_md.register_map.pos_z_out_A.__int__()] not in rcvd_list):
		    rcvd_list.append([ol_w0_md.register_map.pos_x_out_A.__int__(),ol_w0_md.register_map.pos_y_out_A.__int__(),ol_w0_md.register_map.pos_z_out_A.__int__()])
	    if([ol_w0_md.register_map.pos_x_out_B.__int__(),ol_w0_md.register_map.pos_y_out_B.__int__(),ol_w0_md.register_map.pos_z_out_B.__int__()] != [0,0,0] and [ol_w0_md.register_map.pos_x_out_B.__int__(),ol_w0_md.register_map.pos_y_out_B.__int__(),ol_w0_md.register_map.pos_z_out_B.__int__()] not in rcvd_list):
		    rcvd_list.append([ol_w0_md.register_map.pos_x_out_B.__int__(),ol_w0_md.register_map.pos_y_out_B.__int__(),ol_w0_md.register_map.pos_z_out_B.__int__()])
		    #print(len(rcvd_list))
		    ol_w0_md.register_map.Address = ol_w0_md.register_map.Address.__int__()+1
	    
		    #print(ol_w0_md.register_map.Address.__int__())
	    else:
		    ol_w0_md.register_map.read_ctrl = 1
    print()
    print(len(rcvd_list))
    #print("all pos in", w_pos_in)
    #print("all pos out", w_pos_out)
    #print("all frc in", w_frc_in)
    #print("all frc out", w_frc_out)

    print()
    print(ol_w0_md.register_map)
    ol_w0_md.register_map.step = current_step + 1
    current_step = ol_w0_md.register_map.step.__int__()
    print(current_step)
end_time_perf = time.perf_counter()
elapsed_time_perf = end_time_perf - start_time_perf
print(f"Elapsed time using perf_counter(): {elapsed_time_perf:.4f} seconds")

print(f"Elapsed time to run {num_iterations} iterations: {elapsed_time_perf:.4f} seconds")
print(f"Time per iterations: {elapsed_time_perf/(num_iterations+1)}")
#time.sleep(2)


