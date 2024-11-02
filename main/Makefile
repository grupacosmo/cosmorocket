all:
	pio -f -c vim run

upload_beam:
	pio -f -c vim run --target upload --environment ttgo-t-beam

upload_lolin:
	pio -f -c vim run --target upload --environment lolin32_lite
	
check:
	pio -f -c vim check
clean:
	pio -f -c vim run --target clean

commands:
	pio -f -c vim run --target compiledb

program:
	pio -f -c vim run --target program
	

uploadfs:
	pio -f -c vim run --target uploadfs

monitor:
	pio device monitor -b 115200

update:
	pio -f -c vim update

