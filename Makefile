#
# Makefile for asterisk-res_json
# Contributed by Fernando Santos <fernando@nextbilling.com.br>
#
ASTBIN=$(shell which asterisk)
ASTLIBDIR:=$(shell $(ASTBIN) -rx 'core show settings' | grep 'Module directory' | awk '{print $$3}')
ifeq ($(strip $(ASTLIBDIR)),)
	MODULES_DIR:=$(INSTALL_PREFIX)/usr/lib/asterisk/modules
else
	MODULES_DIR:=$(INSTALL_PREFIX)$(ASTLIBDIR)
endif

INSTALL:=install
CC:=gcc
ECHO:=echo
OPTIMIZE:=-O2
DEBUG:=-g

RES_JSON_TARGET = res_json.so
RES_JSON_OBJ = res_json.o cJSON.o
RES_JSON_SRC = res_json.c cJSON.c

MODULES = res_json.so

LIBS+=-g -ggdb -I. 
LIBS+=$(shell pkg-config --cflags --libs asterisk) -I/usr/local/include/asterisk -I/usr/include/asterisk 
CFLAGS += -Wno-missing-field-initializers -Wno-unknown-pragmas -Wextra -Wno-unused-parameter -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Winit-self -Wmissing-format-attribute \
          -Wformat=2 -g -fPIC -D_GNU_SOURCE

LDFLAGS += -shared -Wno-unknown-pragmas

.PHONY: install clean

%.o: %.c $(HEADERS)
	$(CC) -c $(CFLAGS) -D'AST_MODULE="res_json"' -D'AST_MODULE_SELF_SYM=__internal_res_json_self' -o $@ $<

res_json.so: $(RES_JSON_OBJ)
	$(CC) $(LDFLAGS) $(RES_JSON_OBJ) $(LIBS) -o $@

all: $(MODULES)

clean:
	find . -name "*.o" -type f -delete
	find . -name "*.so" -type f -delete

install: $(MODULES)
	$(INSTALL) -m 755 -d $(DESTDIR)$(MODULES_DIR)
	$(INSTALL) -m 755 $(MODULES) $(DESTDIR)$(MODULES_DIR)
	@echo " +------- res_json Installation Complete ------+"
	@echo " +                                             +"
	@echo " + res_json app has successfully installed     +"
	@echo " +---------------------------------------------+"
