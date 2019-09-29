TARGET_EXEC := BlockSystems

BUILD_DIR := bin
SRC_DIRS := src

LIB_DIR := C:/cpplibs
ENGINE_DIR := nx-engine

CC	:= g++
CXX := g++

rwildcard=$(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))
MKDIR_P := mkdir -p

SRCS := $(call rwildcard, $(SRC_DIRS)/, *.cpp *.c)
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)

LDFLAGS := -Wall
LDLIBS := -L"$(ENGINE_DIR)/bin" -lNXEngine -lglu32 -lopengl32 -L"$(LIB_DIR)/lib" -lglew32 -lglfw3 -lassimp.dll -lkernel32 -luser32 -lgdi32 -lwinspool -lshell32 -lole32 -loleaut32 -luuid -lcomdlg32 -ladvapi32

CPPFLAGS := -std=c++17 -g -ggdb
CXXFLAGS := -I$(CURDIR)/src -I$(ENGINE_DIR)/include -I$(LIB_DIR)/include -msse2

all: engine game

game: $(BUILD_DIR)/$(TARGET_EXEC) 

engine:
	@$(MAKE) -C $(ENGINE_DIR) -f Makefile

run:
	@echo "Running $(TARGET_EXEC)..."
	@"./$(BUILD_DIR)/$(TARGET_EXEC).exe"

clean:
	@echo "Cleaning binaries..."
	@$(RM) -r $(BUILD_DIR)/$(SRC_DIRS)
	@$(RM) -r $(ENGINE_DIR)/$(BUILD_DIR)

$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	@echo "Building $(TARGET_EXEC)..."
	@$(CC) $(OBJS) -o $@ $(LDFLAGS) $(LDLIBS)

$(BUILD_DIR)/%.cpp.o: %.cpp
	@echo "Building $@..."
	@$(MKDIR_P) $(dir $@)
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

.PHONY: all clean run engine game
