TARGET_EXEC := BlockSystems

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
	GLFW3 := $(shell pkg-config glfw3 --static --libs)
	ASSIMP := $(shell pkg-config assimp --static --libs)
	GLEW := $(shell pkg-config glew --static --libs)

	FREETYPE := $(shell pkg-config freetype2 --static --libs)
	FREETYPE_I := $(shell pkg-config freetype2 --static --cflags)

	YOJIMBO := -lyojimbo
	SODIUM := $(shell pkg-config libsodium --static --libs)
	MBEDTLS := -lmbedtls -lmbedx509 -lmbedcrypto

	BULLET := $(shell pkg-config bullet --static --libs)
	BULLET_I := $(shell pkg-config bullet --static --cflags)

	LDLIBS := -L"$(ENGINE_DIR)/bin" -lNXEngine -pthread $(GLFW3) $(ASSIMP) $(GLEW) $(FREETYPE) $(YOJIMBO) $(SODIUM) $(MBEDTLS) $(BULLET)
	CXXFLAGS := -I$(CURDIR)/src -I$(ENGINE_DIR)/include -msse2 $(FREETYPE_I) $(BULLET_I)
else
	GLFW3 := -lglfw3
	ASSIMP := -lassimp.dll
	GLEW := -lglew32

	WIN := -lkernel32 -luser32 -lgdi32 -lwinspool -lshell32 -lole32 -loleaut32 -luuid -lcomdlg32 -ladvapi32

	BULLET := -lBulletDynamics -lBulletInverseDynamics -lBulletSoftBody -lBulletCollision -lBullet3Geometry -lBullet3Common -lLinearMath
	BULLET_I := -I$(LIB_DIR)/include/bullet

	FREETYPE := -lfreetype
	FREETYPE_I := -I$(LIB_DIR)/include/freetype2

	LDLIBS := -L"$(ENGINE_DIR)/bin" -lNXEngine -lglu32 -lopengl32 -L"$(LIB_DIR)/lib" $(GLEW) $(GLFW3) $(ASSIMP) $(WIN) $(FREETYPE) $(BULLET)
	CXXFLAGS := -I$(CURDIR)/src -I$(ENGINE_DIR)/include -I$(LIB_DIR)/include $(BULLET_I) $(FREETYPE_I) -msse2
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
