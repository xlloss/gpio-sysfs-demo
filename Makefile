SRCS=gpiolib.c gpiodemo.c
#CFLAGS=
#CC=$(CROSS_COMPILE)gcc

all: fan_rpm

fan_rpm: fan_rpm.c
	$(CC) $(CFLAGS) -o fan_rpm gpiolib.c fan_rpm.c -lpthread -lrt

install: all
	install -m 4755 fan_rpm $(INSTALL)

clean:
	-rm -f fan_rpm
