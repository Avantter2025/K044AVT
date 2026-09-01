# =============================================================================
# Makefile — exemplo LCD (lcd.cpp)
# Avanttec Tecnologia — TEC44AVT / libK044AVT.so
#
# USO
#   make
#   sudo ./lcd
#
# PRE-REQUISITOS
#   - libK044AVT.so instalada
#   - headers display_driver.h e k044avt.hpp
#   - g++ com suporte a C++17
#
# O executavel precisa de acesso direto as portas PS/2, entao roda com sudo.
# =============================================================================

CXX      = g++
CXXFLAGS = -O2 -Wall -Wextra -std=c++17

LIB_DIR ?= /usr/local/lib
INC_DIR ?= /usr/local/include

TARGET   = lcd

.PHONY: all clean

all: $(TARGET)

$(TARGET): lcd.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< \
	    -I$(INC_DIR) -L$(LIB_DIR) -lK044AVT -lpthread \
	    -Wl,-rpath,$(LIB_DIR)

clean:
	rm -f $(TARGET)
