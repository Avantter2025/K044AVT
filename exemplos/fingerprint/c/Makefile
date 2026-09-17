# Makefile — exemplos/c/

CC      = gcc
CFLAGS  = -O2 -Wall -Wextra -std=c11
LIB_DIR = /usr/local/lib
INCS    = -I$(LIB_DIR/include)
LIBS    = -L$(LIB_DIR) -lK044AVT -lpthread
RPATH   = '-Wl,-rpath,$$ORIGIN/$(LIB_DIR)'

TARGETS =  test_fingerprint test_fingerprint_led test_fingerprint_lock test_fingerprint_lock_v2

.PHONY: all clean

all: $(TARGETS)

test_fingerprint: test_fingerprint.c $(LIB_DIR)/libK044AVT.so
	$(CC) $(CFLAGS) -o $@ $< $(INCS) $(LIBS) $(RPATH)

test_fingerprint_led: test_fingerprint_led.c $(LIB_DIR)/libK044AVT.so
	$(CC) $(CFLAGS) -o $@ $< $(INCS) $(LIBS) $(RPATH)

test_fingerprint_lock: test_fingerprint_lock.c $(LIB_DIR)/libK044AVT.so
	$(CC) $(CFLAGS) -o $@ $< $(INCS) $(LIBS) $(RPATH)

test_fingerprint_lock_v2: test_fingerprint_lock_v2.c $(LIB_DIR)/libK044AVT.so
	$(CC) $(CFLAGS) -o $@ $< $(INCS) $(LIBS) $(RPATH)

clean:
	rm -f $(TARGETS)
