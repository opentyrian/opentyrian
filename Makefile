# BUILD SETTINGS ###############################################################

ifneq ($(filter Msys Cygwin, $(shell uname -o)), )
    PLATFORM := WIN32
    TYRIAN_DIR = C:\\TYRIAN
else
    PLATFORM := UNIX
    TYRIAN_DIR = $(gamesdir)/tyrian
endif

WITH_NETWORK := true

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
          -Wno-format-truncation \
          -Wno-missing-field-initializers \
          -O2
LDFLAGS ?=
LDLIBS ?=

ifeq ($(WITH_NETWORK), true)
    SDL_CPPFLAGS := $(shell $(PKG_CONFIG) sdl2 SDL2_net --cflags)
    SDL_LDFLAGS := $(shell $(PKG_CONFIG) sdl2 SDL2_net --libs-only-L --libs-only-other)
    SDL_LDLIBS := $(shell $(PKG_CONFIG) sdl2 SDL2_net --libs-only-l)
else
    SDL_CPPFLAGS := $(shell $(PKG_CONFIG) sdl2 --cflags)
    SDL_LDFLAGS := $(shell $(PKG_CONFIG) sdl2 --libs-only-L --libs-only-other)
    SDL_LDLIBS := $(shell $(PKG_CONFIG) sdl2 --libs-only-l)
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
