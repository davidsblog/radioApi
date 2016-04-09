CC=gcc
CFLAGS=-pthread -std=gnu99
CFILES=radioApi.c dwebsvr.c
TARGET=radioApi

$(TARGET): $(CFILES)
	$(CC) $(CFILES) $(CFLAGS) -o $@

vocore: $(CFILES) $(RESOURCES)
	export STAGING_DIR=$$PWD; \
    `find /home -name mipsel-openwrt-linux-gcc -print -quit -print 2>/dev/null` -o $(TARGET) \
    -Os -s $(CFILES) $(CFLAGS) -D __OPENWRT__

wr740n: $(CFILES) $(RESOURCES)
	export STAGING_DIR=$$PWD; \
    `find /home -maxdepth 6 -name mips-openwrt-linux-uclibc-gcc -print -quit -print 2>/dev/null` -o $(TARGET) \
    -Os -s $(CFILES) $(CFLAGS) -D __OPENWRT__

.PHONY: clean

install: $(TARGET)
	cp -n $(TARGET) /usr/local/bin
	cp -n init_script /etc/init.d/radioApi
	chmod +x /etc/init.d/radioApi
	update-rc.d radioApi defaults
	/etc/init.d/radioApi start

uninstall:
	-/etc/init.d/radioApi stop
	-update-rc.d -f radioApi remove
	-rm -f /usr/local/bin/radioApi
	-rm -f /etc/init.d/radioApi

clean:
	rm -f *.o $(TARGET)
