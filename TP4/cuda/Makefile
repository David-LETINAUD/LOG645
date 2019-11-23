BUILD_DIR = build
SRC_DIR = src

CC = g++
CFLAGS = -Wall -fPIC -m64 -O3 -std=c++14

SRC = $(shell find $(SRC_DIR) -name "*.cpp")
OBJ = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRC))

NVCC = nvcc
CUDA_FLAGS = -m64 -O3 -std=c++14 -arch=sm_50 -gencode=arch=compute_50,code=sm_50

CUDA_SRC = $(shell find $(SRC_DIR) -name "*.cu")
CUDA_OBJ = $(patsubst $(SRC_DIR)/%.cu,$(BUILD_DIR)/%.o,$(CUDA_SRC))

EXE = lab4

RM = rm -r
MKDIR_P = mkdir -p

all: $(OBJ) $(CUDA_OBJ)
	$(NVCC) $(CUDA_FLAGS) -o $(EXE) $^

clean:
	$(RM) $(BUILD_DIR) $(EXE)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(MKDIR_P) $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cu
	$(MKDIR_P) $(dir $@)
	$(NVCC) $(CUDA_FLAGS) -c $< -o $@
