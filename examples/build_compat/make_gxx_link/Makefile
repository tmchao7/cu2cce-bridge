# Deliberately links with the PLAIN host compiler + -lcudart: exercises the
# CUDA-named lib64 symlink family that make_cce builds into its virtual CUDA toolkit
# (cu-bridge link.json pre_make parity). Run through make_cce, which puts
# nvcc (-> cucc) and the host compiler passthroughs on PATH and exports
# CUDA_PATH pointing at the virtual CUDA toolkit.
CXX      ?= g++
NVCC     ?= nvcc
BUILD    ?= build

KERNEL_O := $(BUILD)/kernel.o
MAIN_O   := $(BUILD)/main.o

.PHONY: all clean
all: $(BUILD)/app

$(BUILD):
	mkdir -p $@

$(KERNEL_O): kernel.cu | $(BUILD)
	$(NVCC) -c $< -o $@ -std=c++17 -O2

$(MAIN_O): main.cpp | $(BUILD)
	$(CXX) -c $< -o $@ -std=c++17 -O2 \
	    -I$(CUDA_PATH)/include \
	    -I$(ASCEND_TOOLKIT_HOME)/include

# The lib64 symlink family maps -lcudart to the bridge runtime; g++ never
# sees a real NVIDIA libcudart. The puresimt kernel.o carries Ascend runtime
# references (rtConfigureCall & co), so the Ascend runtime must be on the
# link line explicitly (GNU ld: "DSO missing from command line" otherwise).
$(BUILD)/app: $(MAIN_O) $(KERNEL_O) | $(BUILD)
	$(CXX) $(MAIN_O) $(KERNEL_O) \
	    -L$(CUDA_PATH)/lib64 -lcudart \
	    -L$(ASCEND_TOOLKIT_HOME)/lib64 -lruntime \
	    -Wl,-rpath,$(CUCC_PATH)/lib \
	    -Wl,-rpath,$(ASCEND_TOOLKIT_HOME)/lib64 \
	    -o $@

clean:
	rm -rf $(BUILD)
