CC=avr-gcc
OBJCOPY=avr-objcopy
OBJDUMP=avr-objdump
MCU=atmega1284p
F_CPU=20000000
TARGET=nes
OBJS=main.o playback.o songs.o controller.o lcd.o

CFLAGS=-Os -std=gnu99 -mmcu=$(MCU) -DF_CPU=$(F_CPU) -D__DELAY_BACKWARD_COMPATIBLE__

all: hex lst

$(TARGET).elf: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

hex: $(TARGET).hex

%.hex: %.elf
	$(OBJCOPY) -j .text -j .data -O ihex $< $@

lst: $(TARGET).lst

%.lst: %.elf
	$(OBJDUMP) -h -d $< > $@

clean:
	rm -rf *.o *.elf *.hex *.lst

program: hex
	avrdude -c stk500v2 -p m1284p -v -U $(TARGET).hex
