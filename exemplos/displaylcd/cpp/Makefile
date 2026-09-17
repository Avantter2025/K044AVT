# =============================================================================
# Makefile — exemplo LCD (lcd.cpp)
# Avanttec Tecnologia — TEC44AVT / libK044AVT.so
#
# Compila e linka a libK044AVT.so ja' instalada.
#
# USO
#   make
#   sudo ./lcd
#
# BIBLIOTECA EM OUTRO LUGAR
#   Por padrao procura em /usr/local
#   Para apontar outro caminho:
#
# PRE-REQUISITOS
#   - libK044AVT.so instalada
#   - headers display_driver.h e k044avt.hpp
#   - g++ com suporte a C++17
#
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
