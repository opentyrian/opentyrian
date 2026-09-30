# BUILD SETTINGS ###############################################################

ifneq ($(filter Msys Cygwin, $(shell uname -o)), )
    PLATFORM := WIN32
    TYRIAN_DIR = C:\\TYRIAN
else
    PLATFORM := UNIX
    TYRIAN_DIR = $(gamesdir)/tyrian
endif

# true, false, or auto (true if pkg-config can find SDL_net)
WITH_NETWORK := auto

################################################################################

# see https://www.gnu.org/prep/standards/html_node/Makefile-Conventions.html

SHELL = /bin/sh

CC ?= gcc
INSTALL ?= install
PKG_CONFIG ?= pkg-config
WINDRES ?= windres

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
RES :=
ifeq ($(PLATFORM), WIN32)
    TARGET := opentyrian.exe
    # The icon, from the same resource script the Visual Studio build uses.
    RES := obj/resources.o
endif

SRCS := $(wildcard src/*.c)
OBJS := $(SRCS:src/%.c=obj/%.o)
DEPS := $(SRCS:src/%.c=obj/%.d)

###

ifeq ($(WITH_NETWORK), auto)
    ifeq ($(shell $(PKG_CONFIG) --exists SDL_net 2>/dev/null && echo true), true)
        WITH_NETWORK := true
    else
        WITH_NETWORK := false
        $(info SDL_net not found; building without network support)
    endif
endif

ifeq ($(WITH_NETWORK), true)
    EXTRA_CPPFLAGS += -DWITH_NETWORK
endif

OPENTYRIAN_VERSION := $(shell $(VCS_IDREV) 2>/dev/null && \
                              touch src/opentyrian_version.h)
ifneq ($(OPENTYRIAN_VERSION), )
    EXTRA_CPPFLAGS += -DOPENTYRIAN_VERSION='"$(OPENTYRIAN_VERSION)"'
endif

# -Wno-format-truncation only exists in GCC; Clang rejects it under -Werror
ifeq ($(findstring clang, $(shell $(CC) --version 2>/dev/null)), )
    WNO_FORMAT_TRUNCATION := -Wno-format-truncation
endif

CPPFLAGS ?= -MMD
CPPFLAGS += -DNDEBUG
CFLAGS ?= -pedantic \
          -Wall \
          -Wextra \
          $(WNO_FORMAT_TRUNCATION) \
          -Wno-missing-field-initializers \
          -O2
LDFLAGS ?=
LDLIBS ?=

ifeq ($(WITH_NETWORK), true)
    SDL_CPPFLAGS := $(shell $(PKG_CONFIG) sdl SDL_net --cflags)
    SDL_LDFLAGS := $(shell $(PKG_CONFIG) sdl SDL_net --libs-only-L --libs-only-other)
    SDL_LDLIBS := $(shell $(PKG_CONFIG) sdl SDL_net --libs-only-l)
else
    SDL_CPPFLAGS := $(shell $(PKG_CONFIG) sdl --cflags)
    SDL_LDFLAGS := $(shell $(PKG_CONFIG) sdl --libs-only-L --libs-only-other)
    SDL_LDLIBS := $(shell $(PKG_CONFIG) sdl --libs-only-l)
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
	$(INSTALL_DATA) README.md $(DESTDIR)$(docdir)/
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
	rm -f $(DESTDIR)$(docdir)/README.md
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
	rm -f $(RES)
	rm -f $(TARGET)

$(TARGET) : $(OBJS) $(RES)
	$(CC) $(ALL_CFLAGS) $(ALL_LDFLAGS) -o $@ $^ $(ALL_LDLIBS)

-include $(DEPS)

obj/%.o : src/%.c
	@mkdir -p "$(dir $@)"
	$(CC) $(ALL_CPPFLAGS) $(ALL_CFLAGS) -c -o $@ $<

obj/resources.o : visualc/resources.rc visualc/tyrian.ico
	@mkdir -p "$(dir $@)"
	$(WINDRES) --include-dir visualc -i $< -o $@
