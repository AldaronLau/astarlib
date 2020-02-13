PORT=/dev/ttyACM0

MCU=atmega32u4
CFLAGS=-g -Wall -mcall-prologues -mmcu=$(MCU) -Os
LDFLAGS=-Wl,-gc-sections -Wl,-relax
CC=avr-gcc
TARGET=out/main.hex
OBJECT_FILES=src/buttons.o src/leds.o src/main.o src/timer.o src/gpio.o

all: $(TARGET)

clean:
	rm -f *.o *.hex *.obj *.hex

%.hex: %.obj
	avr-objcopy -R .eeprom -O ihex $< $@

%.obj: $(OBJECT_FILES)
	$(CC) $(CFLAGS) $(OBJECT_FILES) $(LDFLAGS) -o $@

program: $(TARGET)
	avrdude -p $(MCU) -c avr109 -P $(PORT) -U flash:w:$(TARGET)
