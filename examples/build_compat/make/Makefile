CUCC ?= nvcc
CXX ?= c++
BUILD ?= build
CXXFLAGS ?= -std=c++17 -O2

STATIC_OBJECTS := $(BUILD)/kernel_a.o
HOST_OBJECTS := $(BUILD)/main.o
SHARED_LIB := $(BUILD)/libkernel_b.so
STATIC_LIB := $(BUILD)/libkernel_a.a

.PHONY: all clean
all: $(BUILD)/build_compat_app

$(BUILD):
	mkdir -p $@

$(BUILD)/kernel_a.o: kernel_a.cu | $(BUILD)
	$(CUCC) -c $< -o $@ -std=c++17 -O2 -MD -MF $(@:.o=.d)

$(SHARED_LIB): kernel_b.cu | $(BUILD)
	$(CUCC) -shared $< -o $@ -std=c++17 -O2 -fPIC

$(STATIC_LIB): $(STATIC_OBJECTS)
	ar rcs $@ $^

$(BUILD)/main.o: main.cpp | $(BUILD)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD)/build_compat_app: $(STATIC_LIB) $(SHARED_LIB) $(HOST_OBJECTS)
	$(CUCC) $(HOST_OBJECTS) -L$(BUILD) -lkernel_a -lkernel_b -Wl,-rpath,$(abspath $(BUILD)) -o $@

-include $(STATIC_OBJECTS:.o=.d)

clean:
	rm -rf $(BUILD)
