CC = /home/andrea/.local/llvm-mos/bin/mos-nes-nrom-clang
PYTHON = python3

CFLAGS = -O2 -Isrc -Iassets
LDFLAGS = -lneslib -lnesdoug

TARGET = hacktothefuture.nes
CHR_ROM = assets/graphics.chr

SRCS = src/main.c assets/chr-rom.s
HEADERS = src/palettes.h src/sprites.h

all: $(TARGET)

$(CHR_ROM): tools/build_chr.py
	$(PYTHON) tools/build_chr.py $(CHR_ROM)

$(TARGET): $(SRCS) $(HEADERS) $(CHR_ROM)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET) $(LDFLAGS)
	@echo "[SUCCESS] Built $(TARGET) for NES NROM"

clean:
	rm -f $(TARGET) $(CHR_ROM)

.PHONY: all clean
