# BUILD SETTINGS ###########################################

PLATFORM := UNIX

TARGET := opentyrian

############################################################

STRIP := strip
SDL_CONFIG := sdl-config

ifneq ($(PLATFORM), UNIX)
    include crosscompile.mk
endif

SRCS := $(wildcard src/*.c)
OBJS := $(SRCS:src/%.c=obj/%.o)

# FLAGS ####################################################

ifneq ($(MAKECMDGOALS), release)
    CFLAGS += -g3 -O0 -Werror
else
    CFLAGS += -g0 -O2 -DNDEBUG
endif
CFLAGS += -pedantic -Wall -Wextra -Wno-sign-compare -Wno-missing-field-initializers

SDL_CFLAGS := $(shell $(SDL_CONFIG) --cflags)
SDL_LDLIBS := $(shell $(SDL_CONFIG) --libs) -lSDL_net

ALL_CFLAGS = --std=c99 -I./src -DTARGET_$(PLATFORM) $(SDL_CFLAGS) $(CFLAGS)
LDLIBS += $(SDL_LDLIBS)

# RULES ####################################################

.PHONY : all release clean

all : $(TARGET)

release : all
	$(STRIP) $(TARGET)

clean :
	rm -rf obj/*
	rm -f $(TARGET)

ifneq ($(MAKECMDGOALS), clean)
    -include $(OBJS:.o=.d)
endif

$(TARGET) : $(OBJS)
	$(CC) -o $@ $(LDFLAGS) $^ $(LDLIBS)

obj/%.o : src/%.c
	@mkdir -p "$(dir $@)"
	$(CC) -o $@ -c -MMD $(ALL_CFLAGS) $< 
