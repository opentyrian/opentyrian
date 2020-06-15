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

# see http://www.pathname.com/fhs/pub/fhs-2.3.html

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

HG_REV := $(shell hg id -ib 2>/dev/null && \
                  touch src/hg_revision.h)
ifneq ($(HG_REV), )
    EXTRA_CPPFLAGS += -DHG_REV='"$(HG_REV)"'
endif

CPPFLAGS := -DNDEBUG
CFLAGS := -pedantic
CFLAGS += -MMD
CFLAGS += -Wall \
          -Wextra \
          -Wno-missing-field-initializers
CFLAGS += -O2
LDFLAGS := 
LDLIBS := 

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

.PHONY : install
install : $(TARGET) installdirs
	$(INSTALL_PROGRAM) $(TARGET) $(DESTDIR)$(bindir)/
	$(INSTALL_DATA) CREDITS NEWS README $(DESTDIR)$(docdir)/
	$(INSTALL_DATA) linux/man/opentyrian.6 $(DESTDIR)$(man6dir)/opentyrian$(man6ext)

.PHONY : uninstall
uninstall :
	rm -f $(DESTDIR)$(bindir)/$(TARGET)
	rm -f $(DESTDIR)$(docdir)/{CREDITS,NEWS,README}
	rm -f $(DESTDIR)$(man6dir)/opentyrian$(man6ext)

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
