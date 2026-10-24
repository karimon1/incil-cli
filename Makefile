CC      ?= cc
CFLAGS  ?= -Wall -Wextra -O2
LIBS    := $(shell pkg-config --libs ncursesw 2>/dev/null || echo -lncursesw)
PREFIX  ?= $(HOME)/.local
TARGET  = incil-tui
SRC     = incil_tui.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LIBS)

install: $(TARGET)
	install -Dm755 incil $(PREFIX)/bin/incil
	install -Dm755 $(TARGET) $(PREFIX)/bin/$(TARGET)
	install -Dm644 data/ytc.tsv $(PREFIX)/share/incil/ytc.tsv
	@echo "Kuruldu: $(PREFIX)/bin/incil, $(PREFIX)/bin/$(TARGET)"

uninstall:
	rm -f $(PREFIX)/bin/incil $(PREFIX)/bin/$(TARGET) $(PREFIX)/share/incil/ytc.tsv

clean:
	rm -f $(TARGET)

.PHONY: all install uninstall clean
