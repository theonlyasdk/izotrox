TARGET      = izotrox
SRCS        = $(shell find src -type f \( -name '*.cpp' -o -name '*.c' \))
SDL_FLAGS   = $(shell pkg-config --libs --cflags sdl2)
CXXFLAGS    = -O3 -std=c++23 $(SDL_FLAGS) -Isrc
LDFLAGS     = -lm $(SDL_FLAGS)
INSTALL_DIR = /data/adb/$(TARGET).dir/

.PHONY: all build cmake clean build_clean install run push

all: build

build: cmake
	cd build && ninja -j$(shell nproc)
	./build/$(TARGET)

cmake:
	mkdir -p build
	cd build && cmake .. -G Ninja \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_CXX_STANDARD=23 \
		-DCMAKE_CXX_STANDARD_REQUIRED=ON
	cp build/compile_commands.json .

install: build
	cd build && ninja install

run:
	./build/$(TARGET)

clean:
	@if [ -d "build" ]; then cd build && ninja clean; fi

build_clean:
	rm -rf build

push:
	adb shell mkdir -p $(INSTALL_DIR)
	adb push build/$(TARGET) $(INSTALL_DIR)
