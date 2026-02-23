# Build helper for Izotrox

TARGET      = izotrox
BUILD_DIR   = build
INSTALL_DIR = /data/adb/$(TARGET).install.dir/
MAKE		= make
.PHONY: all configure build run clean rebuild push

all: build run

configure:
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake .. \
		-DCMAKE_CXX_STANDARD=23 \
		-DCMAKE_CXX_STANDARD_REQUIRED=ON \
		-DCMAKE_CXX_COMPILER=clang++

build: configure
	@cd $(BUILD_DIR) && $(MAKE) -j6

run:
	@./$(BUILD_DIR)/$(TARGET)

clean:
	@if [ -d "$(BUILD_DIR)" ]; then cd $(BUILD_DIR) && $(MAKE) clean; fi

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
