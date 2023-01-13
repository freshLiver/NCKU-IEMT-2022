DISK = 3
MODE = read
ADDR = 66
SIZE = 10
DATA = aa

TARGET = main

all: $(TARGET)

$(TARGET): main.c force
	gcc -o $@ $<

run: $(TARGET)
ifeq ($(MODE), read)
	./$(TARGET) --disk $(DISK) --$(MODE) --lba $(ADDR) --sector_cnt $(SIZE)
else ifeq ($(MODE), write)
	./$(TARGET) --disk $(DISK) --$(MODE) --lba $(ADDR) --sector_cnt $(SIZE) --data $(DATA)
else ifeq ($(MODE), inquiry)
	./$(TARGET) --disk $(DISK) --inquiry
endif

clean:
	rm -rf *.o $(TARGET)

force:
