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
INSTALL_DATA ?= ${INSTALL} -m 644

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
    EXTRA_CFLAGS += -DWITH_NETWORK
endif

HG_REV := $(shell hg id -ib && touch src/hg_revision.h)
ifneq ($(HG_REV), )
    EXTRA_CFLAGS += -DHG_REV='"$(HG_REV)"'
endif

CFLAGS := -pedantic
CFLAGS += -MMD
CLFAGS += -Wall \
          -Wextra \
          -Wno-missing-field-initializers
CFLAGS += -O2
CFLAGS += -DNDEBUG

SDL_CFLAGS := $(shell $(PKG_CONFIG) sdl --cflags)
SDL_LDLIBS := $(shell $(PKG_CONFIG) sdl --libs)
ifeq ($(WITH_NETWORK), true)
    SDL_LDLIBS += -lSDL_net
endif

ALL_CFLAGS = -std=iso9899:1999 \
             -DTARGET_$(PLATFORM) \
             -DTYRIAN_DIR='"$(TYRIAN_DIR)"' \
             $(EXTRA_CFLAGS) \
             $(SDL_CFLAGS) \
             $(CFLAGS)
ALL_LDFLAGS = $(LDFLAGS)
LDLIBS += $(SDL_LDLIBS) \
          -lm

###

.PHONY : all
all : $(TARGET)

.PHONY : debug
debug : CFLAGS += -Werror
debug : CFLAGS += -O0
debug : CFLAGS += -g3 \
                  -UNDEBUG
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
	$(CC) $(ALL_CFLAGS) $(ALL_LDFLAGS) -o $@ $^ $(LDLIBS)

-include $(DEPS)

obj/%.o : src/%.c
	@mkdir -p "$(dir $@)"
	$(CC) $(ALL_CFLAGS) -c -o $@ $<
