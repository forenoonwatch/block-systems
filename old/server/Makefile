TARGET_EXEC := BlockSystemsServer

BUILD_DIR := bin
SRC_DIRS := src

LIB_DIR := C:/cpplibs
ENGINE_DIR := ../nx-engine

CC	:= g++
CXX := g++

rwildcard=$(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))
MKDIR_P := mkdir -p

SRCS := $(call rwildcard, $(SRC_DIRS)/, *.cpp *.c)
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)


UNAME := $(shell uname -s)

LDFLAGS := -Wall

ifeq ($(UNAME), Linux)
	LDLIBS := -L"$(ENGINE_DIR)/bin" -lNXEngine -pthread `pkg-config glfw3 --static --libs` -lGLU -lGL -lGLEW -lglfw3 -lassimp -lyojimbo -lsodium -lmbedtls -lmbedx509 -lmbedcrypto
	CXXFLAGS := -I$(CURDIR)/src -I$(ENGINE_DIR)/include -msse2
else
	LDLIBS := -L"$(ENGINE_DIR)/bin" -lNXEngine -lglu32 -lopengl32 -L"$(LIB_DIR)/lib" -lglew32 -lglfw3 -lassimp.dll -lkernel32 -luser32 -lgdi32 -lwinspool -lshell32 -lole32 -loleaut32 -luuid -lcomdlg32 -ladvapi32
	CXXFLAGS := -I$(CURDIR)/src -I$(ENGINE_DIR)/include -I$(LIB_DIR)/include -msse2
endif

CPPFLAGS := -std=c++17 -g -ggdb

all: game

game: $(BUILD_DIR)/$(TARGET_EXEC)

run:
	@echo "Running $(TARGET_EXEC)..."
	@"./$(BUILD_DIR)/$(TARGET_EXEC)"

$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	@echo "Building $(TARGET_EXEC)..."
	@$(CC) $(OBJS) -o $@ $(LDFLAGS) $(LDLIBS)

$(BUILD_DIR)/%.cpp.o: %.cpp
	@echo "Building $@..."
	@$(MKDIR_P) $(dir $@)
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

.PHONY: all run game 
