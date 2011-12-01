
include Makefile.inc

all: $(TARGET)

arm: arm/$(SPUTARGET)

arm/$(SPUTARGET):
	make -C arm $(SPUTARGET)

arm-install: arm
	cp arm/$(SPUTARGET) romdisk/$(SPUTARGET)

arm-clean:
	make -C arm clean

sh4: sh4/$(TARGET)

sh4/$(TARGET): arm-install
	make -C sh4 $(TARGET)

sh4-clean:
	make -C sh4 clean

$(TARGET): sh4
	cp sh4/$(TARGET) $(TARGET)

clean: arm-clean sh4-clean
	-rm -f $(TARGET)

