# Makefile — exemplos/displaylcd/cpp/
#
# Funciona nos dois cenarios:
#
#   ARVORE DE DESENVOLVIMENTO — driver_display/ tem os fontes. Se
#   display_driver.c estiver mais novo que a biblioteca, o .so e'
#   reconstruido automaticamente antes de compilar o exemplo (evita linkar
#   contra uma libK044AVT.so desatualizada sem perceber).
#
#   PACOTE DO CLIENTE — so' chegam lcd.cpp, a libK044AVT.so ja' compilada e
#   os headers. Sem display_driver.c, a regra de reconstrucao e' desligada;
#   antes ela fazia o make abortar com "Sem regra para processar o alvo
#   .../display_driver.c", sem sequer compilar o exemplo.
#
# Biblioteca instalada no sistema (make install), em vez da pasta do projeto:
#
#   make LIB_DIR=/usr/local/lib INC_DIR=/usr/local/include

LIB_DIR ?= ../../../driver_display
INC_DIR ?= $(LIB_DIR)

CXX      = g++
CXXFLAGS = -O2 -Wall -Wextra -std=c++17
INCS     = -I$(INC_DIR)
LIBS     = -L$(LIB_DIR) -lK044AVT -lpthread

# rpath: com LIB_DIR relativo, grava um caminho relativo ao proprio binario
# ($ORIGIN), para o exemplo rodar sem instalar nada. Com LIB_DIR absoluto
# (biblioteca instalada), $ORIGIN nao se aplica — grava o caminho direto.
ifeq ($(patsubst /%,,$(LIB_DIR)),)
RPATH = -Wl,-rpath,$(LIB_DIR)
else
RPATH = '-Wl,-rpath,$$ORIGIN/$(LIB_DIR)'
endif

# A presenca do fonte do driver e' o que distingue os dois cenarios acima.
DRIVER_SRC = $(wildcard $(LIB_DIR)/display_driver.c)

ifeq ($(DRIVER_SRC),)
LIB_DEP =
else
LIB_DEP = $(LIB_DIR)/libK044AVT.so
endif

TARGETS = lcd

.PHONY: all clean

all: $(TARGETS)

lcd: lcd.cpp $(LIB_DEP)
	$(CXX) $(CXXFLAGS) -o $@ $< $(INCS) $(LIBS) $(RPATH)

ifneq ($(DRIVER_SRC),)
$(LIB_DIR)/libK044AVT.so: $(LIB_DIR)/display_driver.c $(LIB_DIR)/display_driver.h $(LIB_DIR)/k044avt.hpp
	$(MAKE) -C $(LIB_DIR)
endif

clean:
	rm -f $(TARGETS)
