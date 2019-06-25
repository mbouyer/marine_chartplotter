OBJ=_output/
OBJECTS= $(OBJ)lcd.o $(OBJ)spi.o $(OBJ)usb.o $(OBJ)main.o 
HEASERS= lcd.h spi.h usb.h

CC= sdcc --use-non-free --Werror -mpic16 -p18f27j53
CFLAGS= --fomit-frame-pointer --denable-peeps --optimize-cmp --optimize-df

all: $(OBJ)prog.bin

$(OBJ)prog.hex:$(OBJECTS)
	${CC} -V -Wl,-spic18f27j53.lkr --no-crt --ivt-loc=0x400 --obanksel=2 -Wl,libc18f.lib $(OBJECTS) -o $@

${OBJ}prog.bin: ${OBJ}prog.hex
	hex2bin1k ${OBJ}prog.hex ${OBJ}prog.bin
	@ls -lh ${OBJ}prog.bin

$(OBJ)main.o: main.c version.h ${HEASERS}
	${CC} -V -Wl,-spic18f27j53.lkr --no-crt --ivt-loc=0x400 --obanksel=2 -c main.c -o $@

$(OBJ)spi.o: spi.c ${HEASERS}
	${CC} -V -Wl,-spic18f27j53.lkr --no-crt --ivt-loc=0x400 --obanksel=2 -c spi.c -o $@

$(OBJ)lcd.o: lcd.c ${HEASERS}
	${CC} -V -Wl,-spic18f27j53.lkr --no-crt --ivt-loc=0x400 --obanksel=2 -c lcd.c -o $@

$(OBJ)usb.o: usb.c ${HEASERS}
	${CC} -V -Wl,-spic18f27j53.lkr --no-crt --ivt-loc=0x400 --obanksel=2 -c usb.c -o $@

.PHONY: version.h
version.h:
	sh ./version.sh

clean: 
	rm -rf $(OBJ)*.o $(OBJ)*~ $(OBJ)*.asm $(OBJ)*.lst  *~ 2>/dev/null

