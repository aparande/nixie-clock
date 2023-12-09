FQBN=arduino:mbed_nano:nano33ble

compiledb:
	arduino-cli compile -b $(FQBN) --libraries hv513_bank --libraries mbed_boost_converter --only-compilation-database --build-path .compiledb nixie-clock.ino

compile:
	arduino-cli compile -b $(FQBN) --libraries hv513_bank --libraries mbed_boost_converter nixie-clock.ino

upload: compile
	arduino-cli upload -b $(FQBN) --port /dev/cu.usbmodem14601 nixie-clock.ino
