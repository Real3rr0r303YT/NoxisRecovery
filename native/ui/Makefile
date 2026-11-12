# Noxis Recovery (native) - UI
# Made by 3rr0r303YT on TikTok - GNU GPL v3

CC      ?= cc
PKGS     = cairo libdrm
CFLAGS  += -O2 -Wall -Wextra -Wno-unused-parameter -std=c11 $(shell pkg-config --cflags $(PKGS))
LDLIBS  += $(shell pkg-config --libs $(PKGS)) -lm

SRC = main.c drm.c draw.c input.c icons.c device.c ops.c screen_home.c
OBJ = $(SRC:.c=.o)
BIN = noxis-ui

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS) $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# One frame to a PNG, for checking the layout against design/mock.html.
# Needs only cairo, so it runs on any desktop without libdrm or a device.
preview:
	$(CC) -O2 -std=c11 -DNX_NO_DRM $(shell pkg-config --cflags cairo) \
	      main.c draw.c icons.c screen_home.c -o noxis-ui-preview \
	      $(shell pkg-config --libs cairo) -lm
	./noxis-ui-preview --png preview.png

# The write paths run against temp files, never a real partition.
test:
	$(CC) -O2 -Wall -std=c11 test_ops.c ops.c device.c -o test_ops
	./test_ops

clean:
	rm -f $(OBJ) $(BIN) noxis-ui-preview preview.png test_ops

.PHONY: all clean preview test
