# Build helper for Izotrox

TARGET      = izotrox
BUILD_DIR   = build
INSTALL_DIR = /data/adb/$(TARGET).install.dir/
.PHONY: all configure build run clean rebuild push

all: build run

configure:
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake .. -G Ninja \
		-DCMAKE_BUILD_TYPE=Debug \
		-DCMAKE_CXX_STANDARD=23 \
		-DCMAKE_CXX_STANDARD_REQUIRED=ON \
		-DCMAKE_C_COMPILER=clang \
		-DCMAKE_CXX_COMPILER=clang++

build: configure
	@cd $(BUILD_DIR) && ninja -j $(shell nproc)

run:
	@./$(BUILD_DIR)/$(TARGET)

clean:
	@if [ -d "$(BUILD_DIR)" ]; then cd $(BUILD_DIR) && ninja clean; fi

rebuild:
	@rm -rf $(BUILD_DIR)
	@$(MAKE) build

push:
	@echo "Pushing build to Android device..."
	adb shell mkdir -p $(INSTALL_DIR)
	adb push $(BUILD_DIR)/$(TARGET) $(INSTALL_DIR)
	adb shell mkdir -p /data/adb/izotrox
	adb push res /data/adb/izotrox/
	adb push src /data/adb/izotrox/
	adb push CMakeLists.txt /data/adb/izotrox/
	adb push Makefile /data/adb/izotrox/
