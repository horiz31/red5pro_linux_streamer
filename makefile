.PHONY : clean

CFLAGS= -fPIC -g
CXXFLAGS += -fpermissive

SOURCES = $(shell find ./ -name '*.c')
HEADERS = $(shell find ./ -name '*.h')
OBJECTS=$(SOURCES:.c=.o)

LDFLAGS+= -ldl

TARGET=testbed.bin

all: $(TARGET)

clean:
	rm -f $(OBJECTS) $(TARGET)

$(TARGET) : $(OBJECTS)
	g++ $(CFLAGS) $(OBJECTS) -o $@ $(LDFLAGS)
