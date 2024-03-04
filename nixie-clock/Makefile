FQBN=arduino:mbed_nano:nano33ble
PORT=$(shell arduino-cli board list | awk '/$(FQBN)/ {print $$1}')

compiledb:
	arduino-cli compile -b $(FQBN) --libraries hv513_bank --libraries mbed_boost_converter --only-compilation-database --build-path .compiledb nixie-clock.ino

compile:
	arduino-cli compile -b $(FQBN) --libraries hv513_bank --libraries mbed_boost_converter nixie-clock.ino

upload: compile
	arduino-cli upload -b $(FQBN) --port $(PORT) nixie-clock.ino

