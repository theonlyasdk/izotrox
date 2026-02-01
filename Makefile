# Helper makefile for building Izotrox
# To run the project, run 'make'
# To push files to remote Android device, run 'make push'
TARGET      = izotrox
SRCS        = $(shell find src -type f \( -name '*.cpp' -o -name '*.c' \))
SDL_FLAGS   = $(shell pkg-config --libs --cflags sdl2)
LDFLAGS     = -lm $(SDL_FLAGS)
INSTALL_DIR = /data/adb/$(TARGET).install.dir/

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

run:
	./build/$(TARGET)

clean:
	@if [ -d "build" ]; then cd build && ninja clean; fi

build_clean:
	rm -rf build

push:
	@echo "Pushing source to the connected Android device..."
	adb shell mkdir -p $(INSTALL_DIR)
	adb push build/$(TARGET) $(INSTALL_DIR)
	adb shell mkdir -p /data/adb/izotrox
	adb push res /data/adb/izotrox/
	adb push src /data/adb/izotrox/
	adb push CMakeLists.txt /data/adb/izotrox/
	adb push Makefile /data/adb/izotrox/
	adb push Makefile /data/adb/izotrox/
