CC = /home/andrea/.local/llvm-mos/bin/mos-nes-nrom-clang
PYTHON = python3

CFLAGS = -O2 -Isrc -Iassets
LDFLAGS = -lneslib -lnesdoug -lfamitone2

TARGET = hacktothefuture.nes
CHR_ROM = assets/graphics.chr

SRCS = $(wildcard src/*.c) assets/chr-rom.s assets/audio.s
AUDIO = $(wildcard assets/audio/*.s)
HEADERS = $(wildcard src/*.h)

# Headless tests (Mesen2). libstdc++ preload works around a std::bad_cast abort in MesenCore.so.
MESEN ?= $(HOME)/.local/opt/mesen2/Mesen
MESEN_PRELOAD ?= /usr/lib64/libstdc++.so.6

all: $(TARGET)

$(CHR_ROM): tools/build_chr.py
	$(PYTHON) tools/build_chr.py $(CHR_ROM)

$(TARGET): $(SRCS) $(HEADERS) $(CHR_ROM) $(AUDIO)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET) $(LDFLAGS)
	@echo "[SUCCESS] Built $(TARGET) for NES NROM"

# No local llvm-mos SDK: build inside the Containerfile image.
podman:
	podman build -t llvm-mos -f Containerfile .
	podman run --rm -v "$(CURDIR)":/src:Z llvm-mos make CC=mos-nes-nrom-clang

clean:
	rm -f $(TARGET) $(CHR_ROM)

# Needs a built ROM (make, or make podman). Exit code 0 = all passed.
test:
	@test -f $(TARGET) || { echo "build $(TARGET) first"; exit 1; }
	mkdir -p tests/out
	ROOT=$(CURDIR) LD_PRELOAD=$(MESEN_PRELOAD) timeout 180 $(MESEN) --testRunner \
		--nes.port1.type=NesController --debug.scriptWindow.allowIoOsAccess=true \
		$(TARGET) tests/smoke.lua

.PHONY: all podman clean test
