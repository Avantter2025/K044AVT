# Makefile — exemplos/c/

CC      = gcc
CFLAGS  = -O2 -Wall -Wextra -std=c11
LIB_DIR = ../../driver_display
INCS    = -I$(LIB_DIR)
LIBS    = -L$(LIB_DIR) -lK044AVT -lpthread
RPATH   = '-Wl,-rpath,$$ORIGIN/$(LIB_DIR)'

TARGETS = exemplo_basico exemplo_posicao exemplo_eventos exemplo_keyecho exemplo_aux diagnostico_lcd captura_teclas exemplo_cgram test_display_visual test_display_visual_ctrl test_display_versao test_display_scroll test_display_cursor_blink test_display_cursor test_display_bell test_display_basico test_display_printf test_display_str_long test_display_repeat test_display_keyecho_nav teste_caps_status fingerprint_demo

.PHONY: all clean

all: $(TARGETS)

exemplo_basico: exemplo_basico.c $(LIB_DIR)/libK044AVT.so
	$(CC) $(CFLAGS) -o $@ $< $(INCS) $(LIBS) $(RPATH)

exemplo_posicao: exemplo_posicao.c $(LIB_DIR)/libK044AVT.so
	$(CC) $(CFLAGS) -o $@ $< $(INCS) $(LIBS) $(RPATH)

exemplo_eventos: exemplo_eventos.c $(LIB_DIR)/libK044AVT.so
	$(CC) $(CFLAGS) -o $@ $< $(INCS) $(LIBS) $(RPATH)

exemplo_keyecho: exemplo_keyecho.c $(LIB_DIR)/libK044AVT.so
	$(CC) $(CFLAGS) -o $@ $< $(INCS) $(LIBS) $(RPATH)

exemplo_aux: exemplo_aux.c $(LIB_DIR)/libK044AVT.so
	$(CC) $(CFLAGS) -o $@ $< $(INCS) $(LIBS) $(RPATH)

teste_caps_status: teste_caps_status.c $(LIB_DIR)/libK044AVT.so
	$(CC) $(CFLAGS) -o $@ $< $(INCS) $(LIBS) $(RPATH)

diagnostico_lcd: diagnostico_lcd.c $(LIB_DIR)/libK044AVT.so
	$(CC) $(CFLAGS) -o $@ $< $(INCS) $(LIBS) $(RPATH)

captura_teclas: captura_teclas.c $(LIB_DIR)/libK044AVT.so
	$(CC) $(CFLAGS) -o $@ $< $(INCS) $(LIBS) $(RPATH)

exemplo_cgram: exemplo_cgram.c $(LIB_DIR)/libK044AVT.so
	$(CC) $(CFLAGS) -o $@ $< $(INCS) $(LIBS) $(RPATH)

test_display_visual: test_display_visual.c $(LIB_DIR)/libK044AVT.so
	    $(CC) $(CFLAGS) -o $@ $< $(INCS) $(LIBS) $(RPATH)
	   
test_display_visual_ctrl: test_display_visual_ctrl.c $(LIB_DIR)/libK044AVT.so
	    $(CC) $(CFLAGS) -o $@ $< $(INCS) $(LIBS) $(RPATH)	        

test_display_versao: test_display_versao.c $(LIB_DIR)/libK044AVT.so
	$(CC) $(CFLAGS) -o $@ $< $(INCS) $(LIBS) $(RPATH)
	
test_display_scroll: test_display_scroll.c $(LIB_DIR)/libK044AVT.so
	    $(CC) $(CFLAGS) -o $@ $< $(INCS) $(LIBS) $(RPATH)	
	
test_display_cursor_blink: test_display_cursor_blink.c $(LIB_DIR)/libK044AVT.so
	    $(CC) $(CFLAGS) -o $@ $< $(INCS) $(LIBS) $(RPATH)	
	
test_display_cursor: test_display_cursor.c $(LIB_DIR)/libK044AVT.so
	    $(CC) $(CFLAGS) -o $@ $< $(INCS) $(LIBS) $(RPATH)	
	
test_display_bell: test_display_bell.c $(LIB_DIR)/libK044AVT.so
	    $(CC) $(CFLAGS) -o $@ $< $(INCS) $(LIBS) $(RPATH)	
	
test_display_basico: test_display_basico.c $(LIB_DIR)/libK044AVT.so
	    $(CC) $(CFLAGS) -o $@ $< $(INCS) $(LIBS) $(RPATH)	
	
test_display_printf: test_display_printf.c $(LIB_DIR)/libK044AVT.so
	    $(CC) $(CFLAGS) -o $@ $< $(INCS) $(LIBS) $(RPATH)	
	    
test_display_str_long:test_display_str_long.c $(LIB_DIR)/libK044AVT.so
	    $(CC) $(CFLAGS) -o $@ $< $(INCS) $(LIBS) $(RPATH)	
	     
test_display_repeat:test_display_repeat.c $(LIB_DIR)/libK044AVT.so
	    $(CC) $(CFLAGS) -o $@ $< $(INCS) $(LIBS) $(RPATH)

test_display_keyecho_nav: test_display_keyecho_nav.c $(LIB_DIR)/libK044AVT.so
	    $(CC) $(CFLAGS) -o $@ $< $(INCS) $(LIBS) $(RPATH)

fingerprint_demo: fingerprint_demo.c $(LIB_DIR)/libK044AVT.so
	    $(CC) $(CFLAGS) -o $@ $< $(INCS) $(LIBS) $(RPATH)

$(LIB_DIR)/libK044AVT.so: $(LIB_DIR)/display_driver.c $(LIB_DIR)/display_driver.h $(LIB_DIR)/k044avt.hpp
	$(MAKE) -C $(LIB_DIR)

clean:
	rm -f $(TARGETS)
