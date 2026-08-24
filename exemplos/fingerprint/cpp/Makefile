# Makefile — exemplos/fingerprint/cpp/

CXX      = g++
CXXFLAGS = -O2 -Wall -Wextra -std=c++17
LIB_DIR  = ../../../driver_display
INCS     = -I$(LIB_DIR)
LIBS     = -L$(LIB_DIR) -lK044AVT -lpthread
RPATH    = '-Wl,-rpath,$$ORIGIN/$(LIB_DIR)'

TARGETS = finger2_v2

.PHONY: all clean

all: $(TARGETS)

finger2_v2: finger2_v2.cpp $(LIB_DIR)/libK044AVT.so
	$(CXX) $(CXXFLAGS) -o $@ $< $(INCS) $(LIBS) $(RPATH)

$(LIB_DIR)/libK044AVT.so: $(LIB_DIR)/display_driver.c $(LIB_DIR)/display_driver.h $(LIB_DIR)/k044avt.hpp
	$(MAKE) -C $(LIB_DIR)

clean:
	rm -f $(TARGETS)
