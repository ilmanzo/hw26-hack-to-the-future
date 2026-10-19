CC = /home/andrea/.local/llvm-mos/bin/mos-nes-mmc3-clang
PYTHON = python3

CFLAGS = -O2 -Isrc -Iassets
LDFLAGS = -lneslib -lnesdoug -lfamitone2

TARGET = hacktothefuture.nes
CHR_ROM = assets/graphics.chr
ROOMS_H = assets/rooms.h

SRCS = $(wildcard src/*.c) assets/chr-rom.s assets/audio.s
AUDIO = $(wildcard assets/audio/*.s)
HEADERS = $(wildcard src/*.h)

# Headless tests (Mesen2). libstdc++ preload works around a std::bad_cast abort in MesenCore.so.
MESEN ?= $(HOME)/.local/opt/mesen2/Mesen
MESEN_PRELOAD ?= /usr/lib64/libstdc++.so.6

all: $(TARGET)

$(CHR_ROM): tools/build_chr.py
	$(PYTHON) tools/build_chr.py $(CHR_ROM)

$(ROOMS_H): tools/build_rooms.py tools/check_rooms.py
	$(PYTHON) tools/build_rooms.py $(ROOMS_H)

$(TARGET): $(SRCS) $(HEADERS) $(CHR_ROM) $(ROOMS_H) $(AUDIO)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET) $(LDFLAGS)
	@echo "[SUCCESS] Built $(TARGET) for NES MMC3"

# No local llvm-mos SDK: build inside the Containerfile image.
podman:
	podman build -t llvm-mos -f Containerfile .
	podman run --rm -v "$(CURDIR)":/src:Z llvm-mos make CC=mos-nes-mmc3-clang

clean:
	rm -f $(TARGET) $(CHR_ROM) $(ROOMS_H)

# Needs a built ROM (make, or make podman). Exit code 0 = all passed.
TESTS = tests/smoke.lua tests/room.lua tests/map.lua tests/sprites.lua

test:
	@test -f $(TARGET) || { echo "build $(TARGET) first"; exit 1; }
	mkdir -p tests/out
	@for t in $(TESTS); do \
		echo "== $$t"; \
		ROOT=$(CURDIR) LD_PRELOAD=$(MESEN_PRELOAD) timeout 300 $(MESEN) --testRunner \
			--nes.port1.type=NesController --debug.scriptWindow.allowIoOsAccess=true \
			$(TARGET) $$t || exit 1; \
	done

.PHONY: all podman clean test
