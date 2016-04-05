CC=gcc

dwebmake: radioApi.c dwebsvr.c
	$(CC) -o radioApi radioApi.c dwebsvr.c -pthread -std=gnu99

vocore: radioApi.c dwebsvr.c
	export STAGING_DIR=$$PWD; \
`find /home -name mipsel-openwrt-linux-gcc -print -quit -print 2>/dev/null` -o radioApi -Os -s radioApi.c dwebsvr.c -pthread

wr740n: radioApi.c dwebsvr.c
	export STAGING_DIR=$$PWD; \
`find /home -maxdepth 6 -name mips-openwrt-linux-uclibc-gcc -print -quit -print 2>/dev/null` -o radioApi -Os -s radioApi.c dwebsvr.c -pthread

.PHONY: clean

clean:
	rm -f *.o *.exe radioApi
