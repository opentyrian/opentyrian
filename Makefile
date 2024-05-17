# BUILD SETTINGS ###############################################################

ifneq ($(filter Msys Cygwin, $(shell uname -o)), )
    PLATFORM := WIN32
    OS := win
    TYRIAN_DIR = C:\\TYRIAN
else ifeq (Darwin, $(shell uname)) # macOS
    PLATFORM := UNIX
    OS := osx
    TYRIAN_DIR = $(gamesdir)/opentyrian2000
else
    PLATFORM := UNIX
    OS := linux
    TYRIAN_DIR = $(gamesdir)/tyrian
endif

# detect the architecture
HOST_TRIPLET := $(shell $(CC) -dumpmachine)
_ARCH := $(word 1, $(subst -, ,$(HOST_TRIPLET)))

ifeq ($(_ARCH), aarch64)
    ARCH := arm64
else
    ARCH := $(_ARCH)
endif

# check if VCPKG_TRIPLET, VCPKG_TARGET_TRIPLET, or VCPKG_DEFAULT_TRIPLET is set
# if not, set it to the OS and ARCH
ifneq ($(VCPKG_TRIPLET), )
    $(info VCPKG_TRIPLET: $(VCPKG_TRIPLET))
else ifneq ($(VCPKG_TARGET_TRIPLET), )
    VCPKG_TRIPLET := $(VCPKG_TARGET_TRIPLET)
else ifneq ($(VCPKG_DEFAULT_TRIPLET), )
    VCPKG_TRIPLET := $(VCPKG_DEFAULT_TRIPLET)
else
    _VCPKG_TRIPLET := $(ARCH)-$(OS)
    # if using windows, use the static-md triplet by default
    ifeq ($(OS), win)
        VCPKG_TRIPLET := $(_VCPKG_TRIPLET)-static-md
    else
        VCPKG_TRIPLET := $(_VCPKG_TRIPLET)
    endif
endif

WITH_NETWORK := true
WITH_MIDI := true

################################################################################

# see https://www.gnu.org/prep/standards/html_node/Makefile-Conventions.html

SHELL = /bin/sh

CC ?= gcc
INSTALL ?= install
PKG_CONFIG ?= pkg-config

VCS_IDREV ?= (git describe --tags || git rev-parse --short HEAD)

INSTALL_PROGRAM ?= $(INSTALL)
INSTALL_DATA ?= $(INSTALL) -m 644

prefix ?= /usr/local
exec_prefix ?= $(prefix)

bindir ?= $(exec_prefix)/bin
datarootdir ?= $(prefix)/share
datadir ?= $(datarootdir)
docdir ?= $(datarootdir)/doc/opentyrian
mandir ?= $(datarootdir)/man
man6dir ?= $(mandir)/man6
man6ext ?= .6
desktopdir ?= $(datarootdir)/applications
icondir ?= $(datarootdir)/icons

# see https://www.pathname.com/fhs/pub/fhs-2.3.html

gamesdir ?= $(datadir)/games

###

TARGET := opentyrian

SRCS := $(wildcard src/*.c)
OBJS := $(SRCS:src/%.c=obj/%.o)
DEPS := $(SRCS:src/%.c=obj/%.d)

###

ifeq ($(WITH_NETWORK), true)
    EXTRA_CPPFLAGS += -DWITH_NETWORK
endif

ifeq ($(WITH_MIDI), true)
    EXTRA_CPPFLAGS += -DWITH_MIDI
endif

ifeq ($(OS), linux)
    EXTRA_CPPFLAGS += -DNO_NATIVE_MIDI
endif

OPENTYRIAN_VERSION := $(shell $(VCS_IDREV) 2>/dev/null && \
                              touch src/opentyrian_version.h)
ifneq ($(OPENTYRIAN_VERSION), )
    EXTRA_CPPFLAGS += -DOPENTYRIAN_VERSION='"$(OPENTYRIAN_VERSION)"'
endif

CPPFLAGS ?= -MMD
CPPFLAGS += -DNDEBUG
CFLAGS ?= -pedantic \
          -Wall \
          -Wextra \
          -Wno-strict-prototypes \
          -Wno-missing-field-initializers \
          -O3

LDFLAGS ?=
LDLIBS ?=

libmidiconv_PC_PATH :=

# check if the "vcpkg_installed" directory exists in the directory that this makefile is in
# if it does, add it to the pkg-config search path=
VCPKG_CHLDDIR := $(VCPKG_TRIPLET)/lib/pkgconfig
ifeq ($(findstring debug, $(MAKECMDGOALS)), debug)
    VCPKG_CHLDDIR := $(VCPKG_TRIPLET)/debug/lib/pkgconfig
endif

# local vcpkg
ifneq ($(wildcard vcpkg_installed), )
    VCPKG_PC_PATH := $(CURDIR)/vcpkg_installed/$(VCPKG_CHLDDIR)
# global vcpkg
else ifneq ($(VCPKG_ROOT), )
    VCPKG_PC_PATH := $(VCPKG_ROOT)/installed/$(VCPKG_CHLDDIR)
endif

ifeq ($(PKG_CONFIG_PATH), )
    PKG_CONFIG_PATH := $(VCPKG_PC_PATH):$(libmidiconv_PC_PATH):$(shell $(PKG_CONFIG) --variable pc_path pkg-config)
else
    PKG_CONFIG_PATH := $(VCPKG_PC_PATH):$(libmidiconv_PC_PATH):$(PKG_CONFIG_PATH)
endif


SDL_PACKAGES := sdl2
MIDIPROC_LIBS :=
PC_CMD := PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) $(PKG_CONFIG)

ifeq ($(WITH_NETWORK), true)
    SDL_PACKAGES += SDL2_net
endif

ifeq ($(WITH_MIDI), true)
    SDL_PACKAGES += SDL2_mixer_ext
    MIDIPROC_LIBS := midiproc fluidsynth
endif

SDL_CPPFLAGS := $(shell $(PC_CMD) $(SDL_PACKAGES) $(MIDIPROC_LIBS) --cflags)
SDL_LDFLAGS := $(shell $(PC_CMD) $(SDL_PACKAGES) $(MIDIPROC_LIBS) --libs-only-L --libs-only-other)
SDL_LDLIBS := $(shell $(PC_CMD) $(SDL_PACKAGES) $(MIDIPROC_LIBS) --libs-only-l)

# add stdc++ to the ldlibs if using midiproc
ifneq ($(MIDIPROC_LIBS), )
    SDL_LDLIBS += -lstdc++
endif

ALL_CPPFLAGS = -DTARGET_$(PLATFORM) \
               -DTYRIAN_DIR='"$(TYRIAN_DIR)"' \
               $(EXTRA_CPPFLAGS) \
               $(SDL_CPPFLAGS) \
               $(CPPFLAGS)
ALL_CFLAGS = -std=iso9899:1999 \
             $(CFLAGS)
ALL_LDFLAGS = $(SDL_LDFLAGS) \
              $(LDFLAGS)
ALL_LDLIBS = -lm \
             $(SDL_LDLIBS) \
             $(LDLIBS)

###

.PHONY : all
all : $(TARGET)

.PHONY : debug
debug : CPPFLAGS += -UNDEBUG
debug : CFLAGS += -Werror
debug : CFLAGS += -O0
debug : CFLAGS += -g3
debug : all

.PHONY : installdirs
installdirs :
	mkdir -p $(DESTDIR)$(bindir)
	mkdir -p $(DESTDIR)$(docdir)
	mkdir -p $(DESTDIR)$(man6dir)
	mkdir -p $(DESTDIR)$(desktopdir)
	mkdir -p $(DESTDIR)$(icondir)/hicolor/22x22/apps
	mkdir -p $(DESTDIR)$(icondir)/hicolor/24x24/apps
	mkdir -p $(DESTDIR)$(icondir)/hicolor/32x32/apps
	mkdir -p $(DESTDIR)$(icondir)/hicolor/48x48/apps
	mkdir -p $(DESTDIR)$(icondir)/hicolor/128x128/apps

.PHONY : install
install : $(TARGET) installdirs
	$(INSTALL_PROGRAM) $(TARGET) $(DESTDIR)$(bindir)/
	$(INSTALL_DATA) NEWS README $(DESTDIR)$(docdir)/
	$(INSTALL_DATA) linux/man/opentyrian.6 $(DESTDIR)$(man6dir)/opentyrian$(man6ext)
	$(INSTALL_DATA) linux/opentyrian.desktop $(DESTDIR)$(desktopdir)/
	$(INSTALL_DATA) linux/icons/tyrian-22.png $(DESTDIR)$(icondir)/hicolor/22x22/apps/opentyrian.png
	$(INSTALL_DATA) linux/icons/tyrian-24.png $(DESTDIR)$(icondir)/hicolor/24x24/apps/opentyrian.png
	$(INSTALL_DATA) linux/icons/tyrian-32.png $(DESTDIR)$(icondir)/hicolor/32x32/apps/opentyrian.png
	$(INSTALL_DATA) linux/icons/tyrian-48.png $(DESTDIR)$(icondir)/hicolor/48x48/apps/opentyrian.png
	$(INSTALL_DATA) linux/icons/tyrian-128.png $(DESTDIR)$(icondir)/hicolor/128x128/apps/opentyrian.png

.PHONY : uninstall
uninstall :
	rm -f $(DESTDIR)$(bindir)/$(TARGET)
	rm -f $(DESTDIR)$(docdir)/NEWS $(DESTDIR)$(docdir)/README
	rm -f $(DESTDIR)$(man6dir)/opentyrian$(man6ext)
	rm -f $(DESTDIR)$(desktopdir)/opentyrian.desktop
	rm -f $(DESTDIR)$(icondir)/hicolor/22x22/apps/opentyrian.png
	rm -f $(DESTDIR)$(icondir)/hicolor/24x24/apps/opentyrian.png
	rm -f $(DESTDIR)$(icondir)/hicolor/32x32/apps/opentyrian.png
	rm -f $(DESTDIR)$(icondir)/hicolor/48x48/apps/opentyrian.png
	rm -f $(DESTDIR)$(icondir)/hicolor/128x128/apps/opentyrian.png

.PHONY : clean
clean :
	rm -f $(OBJS)
	rm -f $(DEPS)
	rm -f $(TARGET)

$(TARGET) : $(OBJS)
	$(CC) $(ALL_CFLAGS) $(ALL_LDFLAGS) -o $@ $^ $(ALL_LDLIBS)

-include $(DEPS)

obj/%.o : src/%.c
	@mkdir -p "$(dir $@)"
	$(CC) $(ALL_CPPFLAGS) $(ALL_CFLAGS) -c -o $@ $<
