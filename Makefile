CC      = gcc
CFLAGS  = -Wall -Wextra -O2
LIBS    = -lncurses
TARGET  = incil-tui
SRC     = incil_tui.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LIBS)

install: $(TARGET)
	cp $(TARGET) $(HOME)/.local/bin/$(TARGET)
	chmod +x $(HOME)/.local/bin/$(TARGET)
	@echo "Kuruldu: $(HOME)/.local/bin/$(TARGET)"

clean:
	rm -f $(TARGET)

.PHONY: all install clean
