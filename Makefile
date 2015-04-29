#******************************************************************************
#	Copyright (C) 2015 Francis Bergin
#
#
#	This file is part of internet-thermostat.
#
#	internet-thermostat is free software: you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation, either version 3 of the License, or
#	(at your option) any later version.
#
#	internet-thermostat is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with internet-thermostat.  If not, see <http://www.gnu.org/licenses/>.
#
#******************************************************************************

PRG = internet-thermostat

CC = avr-gcc
MMCU = atmega32
FREQ = 16000000

CC_HEADERS = ./inc
CC_OPTIMIZE = -O1
CC_DEBUG = -g3 -gdwarf-2
CC_ERRORS = -Wall
CC_ARGS = -mmcu=$(MMCU) -DF_CPU=$(FREQ)UL -I $(CC_HEADERS) $(CC_OPTIMIZE) $(CC_DEBUG) $(CC_ERRORS)

# install variables
AVRDUDE = avrdude
AVRDUDE_PROG = dragon_jtag
AVRDUDE_PORT = usb
AVRDUDE_FLAGS = -p $(MMCU) -c $(AVRDUDE_PROG) -P $(AVRDUDE_PORT) -e
AVRDUDE_WRITE_FLASH = -U flash:w:$(PRG).hex:i
AVRDUDE_WRITE_EEPROM = -U eeprom:w:$(PRG).eep:i

# object files
OBJS = \
./bin/obj/adc.o \
./bin/obj/arp.o \
./bin/obj/display.o \
./bin/obj/enc28j60.o \
./bin/obj/ethernet.o \
./bin/obj/http.o \
./bin/obj/icmp.o \
./bin/obj/ip.o \
./bin/obj/main.o \
./bin/obj/relays.o \
./bin/obj/rotaryencoder.o \
./bin/obj/tcp.o

# all targets
all : ./bin/$(PRG).elf ./bin/$(PRG).hex ./bin/$(PRG).eep printsize

# compiler
./bin/obj/%.o : ./src/%.c
	@mkdir -p $(@D)
	$(CC) -c $(CC_ARGS) -o $@ $<

# linker and other
./bin/$(PRG).elf : $(OBJS)
	$(CC) -o ./bin/$(PRG).elf $(OBJS) -Wl,-Map=./bin/$(PRG).map -Wl,-lm -mmcu=$(MMCU)

./bin/$(PRG).hex : ./bin/$(PRG).elf
	avr-objcopy -O ihex -R .eeprom -R .fuse -R .lock -R .signature ./bin/$(PRG).elf ./bin/$(PRG).hex

./bin/$(PRG).eep : ./bin/$(PRG).elf
	avr-objcopy -j .eeprom  --set-section-flags=.eeprom=alloc,load --change-section-lma .eeprom=0 --no-change-warnings -O ihex ./bin/$(PRG).elf ./bin/$(PRG).eep

printsize :
	avr-size -C --mcu=$(MMCU) ./bin/$(PRG).elf

# program the device
install : $(PRG).hex $(PRG).eep
	$(AVRDUDE) $(AVRDUDE_FLAGS) $(AVRDUDE_WRITE_FLASH) $(AVRDUDE_WRITE_EEPROM)

# erase the device
uninstall :
	$(AVRDUDE) $(AVRDUDE_FLAGS)

# clean generated files
clean:
	rm -r ./bin
