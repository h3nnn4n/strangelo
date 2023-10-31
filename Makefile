TARGET = $(notdir $(CURDIR))
BUILDDIR = $(abspath $(CURDIR)/build)

OPTIONS = -DIMGUI_IMPL_API="extern \"C\"" \
	  -DIMGUI_IMPL_OPENGL_LOADER_GLAD \
	  -DIMGUI_IMPL_OPENGL_LOADER_GLFW

INCLUDES = -Isrc                         \
	   -Ideps/glad/include/          \
	   -Ideps/glfw/include/          \
	   -Ideps/cglm/include/          \
	   -Ideps/glm/                   \
	   -Ideps/cimgui/                \
	   -Ideps/cimgui/imgui           \
	   -Ideps/cimgui/imgui/examples/ \
	   -Ideps/cimplot/               \
	   -Ideps/imgui/                 \
	   -Ideps/cJSON/                 \
	   -Ideps/stb/                   \
	   -Ideps/pcg-c/include          \
           -Ideps/pcg-c/extras

LINKS = -Ldeps/glfw/build/src/ \
	-Ldeps/glad/src/       \
	-Ldeps/cJSON/build/

CPPFLAGS = -Wall -std=c++11 $(OPTIMIZATION) $(OPTIONS) $(LINKS) $(INCLUDES)
CFLAGS = -Wall -std=c99 $(OPTIMIZATION) $(OPTIONS) $(LINKS) $(INCLUDES)

OPTIMIZATION=-O0 -g

LDFLAGS = $(OPTIMIZATION) -Wl,-Ldeps/glfw/build/src/ -Ldeps/cJSON/build/ -Ldeps/pcg-c/src/

LIBS = -lm -lglfw -lpthread -ldl -lstdc++ -lcjson -lpcg_random

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
	LIBS += -lGL
	ECHOFLAGS = -e
endif
ifeq ($(UNAME_S),Darwin)
	LIBS += -framework OpenGL
	CFLAGS += -Wno-unused-command-line-argument
	CPPFLAGS += -Wno-unused-command-line-argument -Wno-mismatched-tags
endif

LD_LIBRARY_PATH = deps/glfw/build/src/:deps/cJSON/build/

CC = gcc
CXX = g++

C_FILES := $(wildcard src/*.c) \
	   $(wildcard deps/pcg-c/extras/*.c) \
	   deps/glad/src/glad.c
CPP_FILES := $(wildcard src/*.cpp)
STB_FILES := $(wildcard deps/stb/*.c)
IMGUI_FILES := $(wildcard ./deps/cimgui/*.cpp) \
	       $(wildcard ./deps/cimgui/imgui/*.cpp) \
	       $(wildcard ./deps/cimgui/imgui/backends/imgui_impl_glfw.cpp) \
	       $(wildcard ./deps/cimgui/imgui/backends/imgui_impl_opengl3.cpp) \
	       $(wildcard ./deps/cimplot/*.cpp) \
	       $(wildcard ./deps/cimplot/implot/*.cpp) \

SOURCES := $(STB_FILES:.c=.o) \
	   $(CPP_FILES:.cpp=.o) \
	   $(IMGUI_FILES:.cpp=.o) \
	   $(C_FILES:.c=.o)
OBJS := $(foreach src,$(SOURCES), $(BUILDDIR)/$(src))

all: build

build: pcg pcg_full $(TARGET)

rebuild: clean $(TARGET)

debug: debug_prepare build

debug_prepare:
	$(eval OPTIMIZATION=-g -pg -O0)

gperftools: gperftools_prepare build

gperftools_prepare:
	$(eval OPTIMIZATION=-DWITHGPERFTOOLS -lprofiler -ltcmalloc -g -pg -O2 -DNDEBUG -fno-inline-functions -fno-inline-functions-called-once -fno-optimize-sibling-calls)

callgrind: callgrind_prepare build

callgrind_prepare:
	$(eval OPTIMIZATION=-g -O2 -DNDEBUG -fno-inline-functions -fno-inline-functions-called-once -fno-optimize-sibling-calls -fno-default-inline -fno-inline)

run: $(TARGET)
	LD_LIBRARY_PATH=$(LD_LIBRARY_PATH) $(CURDIR)/$(TARGET)

gdb: $(TARGET)
	LD_LIBRARY_PATH=$(LD_LIBRARY_PATH) gdb $(CURDIR)/$(TARGET)

pcg:
	@echo $(ECHOFLAGS) "[CC]\tpcg core"
	@$(MAKE) -s -C deps/pcg-c/src/

pcg_full:
	@echo $(ECHOFLAGS) "[CC]\tpcg full"
	@$(MAKE) -s -C deps/pcg-c/

pcg_clean:
	@$(MAKE) clean -C deps/pcg-c/src/ > /dev/null

$(BUILDDIR)/%.o: %.c
	@echo $(ECHOFLAGS) "[CC]\t$<"
	@mkdir -p "$(dir $@)"
	@$(CC) $(CFLAGS) $(L_INC) $(CUSTOM) -o "$@" -c "$<"

$(BUILDDIR)/%.o: %.cpp
	@echo $(ECHOFLAGS) "[CXX]\t$<"
	@mkdir -p "$(dir $@)"
	@$(CXX) $(CPPFLAGS) $(L_INC) $(CUSTOM) -o "$@" -c "$<"

$(TARGET).o: $(OBJS) $(LDSCRIPT)
	@echo $(ECHOFLAGS) "[LD]\t$@"
	@$(CC) $(LDFLAGS) -o "$@" $(OBJS) $(LIBS) $(CUSTOM)

$(TARGET): $(OBJS) $(LDSCRIPT)
	@echo $(ECHOFLAGS) "[LD]\t$@"
	@$(CC) $(LDFLAGS) -o "$@" $(OBJS) $(LIBS) $(CUSTOM)

-include $(OBJS:.o=.d)

clean:
	@echo Cleaning...
	@rm -rf "$(BUILDDIR)/src/"
	@rm -f "$(TARGET).o"

superclean: pcg_clean
	@echo Activating clean slate protocol
	@rm -rf "$(BUILDDIR)"
	@rm -f "$(TARGET).o"
